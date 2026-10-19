#include "http.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ENV_PREFIX "L_KAFO_DEV_"
#define ENV_ADDR ENV_PREFIX "ADDR"
#define ENV_PORT ENV_PREFIX "PORT"

enum { BUFFER_SIZE = 1024 * 16 };

int main(void) {
	char *server_addr = getenv(ENV_ADDR);
	if (server_addr == nullptr || *server_addr == 0) {
		fprintf(stderr, "%s is unset or empty\n", ENV_ADDR);
		return EXIT_FAILURE;
	}

	char *server_port = getenv(ENV_PORT);
	if (server_port == nullptr || *server_port == 0) {
		fprintf(stderr, "%s is unset or empty\n", ENV_PORT);
		return EXIT_FAILURE;
	}

	struct addrinfo addr_hints = {0};
	addr_hints.ai_family = AF_UNSPEC;
	addr_hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *addr_result;
	int err =
	    getaddrinfo(server_addr, server_port, &addr_hints, &addr_result);
	if (err != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		return EXIT_FAILURE;
	}

	int server_sockfd = -1;
	for (struct addrinfo *info = addr_result; info != nullptr;
	     info = info->ai_next) {
		server_sockfd = socket(info->ai_family, info->ai_socktype,
		                       info->ai_protocol);
		if (server_sockfd == -1) {
			server_sockfd = -1;
			continue;
		}

		int server_socket_reuse = 1;
		if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR,
		               &server_socket_reuse,
		               sizeof(server_socket_reuse)) == -1) {
			close(server_sockfd);
			server_sockfd = -1;
			continue;
		}

		if (bind(server_sockfd, info->ai_addr, info->ai_addrlen) ==
		    -1) {
			close(server_sockfd);
			server_sockfd = -1;
			continue;
		}

		break;
	}

	freeaddrinfo(addr_result);

	if (server_sockfd == -1) {
		fprintf(stderr, "could not bind to %s:%s\n", server_addr,
		        server_port);
		return EXIT_FAILURE;
	}

	if (listen(server_sockfd, SOMAXCONN) == -1) {
		perror("listen");
		return EXIT_FAILURE;
	}

	struct sockaddr_storage client_addr;
	socklen_t client_addr_len;
	for (;;) {
		client_addr_len = sizeof(client_addr);

		int client_sockfd =
		    accept(server_sockfd, (struct sockaddr *)(&client_addr),
		           &client_addr_len);
		if (client_sockfd == -1) {
			perror("accept");
			continue;
		}

		char buffer[BUFFER_SIZE];
		size_t request_len =
		    recv(client_sockfd, buffer, BUFFER_SIZE, 0);
		if (request_len == (size_t)(-1)) {
			perror("recv");
			goto inform_error;
		}

		struct http_request request;
		enum http_error http_err =
		    http_parse_request(buffer, request_len, &request);
		if (http_err != HTTP_ERROR_NONE) {
			fprintf(stderr,
			        "error(%d): Unknown error parsing request.\n",
			        http_err);
			goto inform_error;
		}

		const char CONTENT_PREFIX[] = "Pong: ";
		size_t content_len =
		    sizeof(CONTENT_PREFIX) - 1 + strlen(request.path) + 1;

		int response_len =
		    snprintf(buffer, BUFFER_SIZE,
		             "HTTP/1.1 200 OK\r\n"
		             "Content-Length: %zu\r\n"
		             "Connection: close\r\n"
		             "\r\n"
		             "%s%s\n",
		             content_len, CONTENT_PREFIX, request.path);
		if (response_len > BUFFER_SIZE - 1) {
			fprintf(stderr,
			        "error: Response too large ('%d' bytes).\n",
			        response_len);
			goto inform_error;
		}
		if (send(client_sockfd, buffer, response_len, 0) == -1) {
			perror("send");
		}
		goto cleanup;

	inform_error:
		char error_data[] = "HTTP/1.1 500\r\n"
		                    "Content-Length: 0\r\n"
		                    "Connection: close\r\n"
		                    "\r\n";
		if (send(client_sockfd, error_data, sizeof(error_data) - 1,
		         0) == -1) {
			perror("send");
		}

	cleanup:
		close(client_sockfd);
	}

	close(server_sockfd);

	return EXIT_SUCCESS;
}
