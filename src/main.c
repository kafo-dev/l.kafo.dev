#include "http.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum { BUFFER_SIZE = 1024 * 16 };

int main(void) {
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	int server_socket_reuse = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR,
	               &server_socket_reuse,
	               sizeof(server_socket_reuse)) == -1) {
		perror("setsockopt(SO_REUSEADDR)");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr = {
	    .sin_family = AF_INET,
	    .sin_port = htons(9090),
	    .sin_addr = {.s_addr = htonl(INADDR_ANY)}};
	if (bind(server_sockfd, (struct sockaddr *)(&server_addr),
	         sizeof(server_addr)) == -1) {
		perror("bind");
		return EXIT_FAILURE;
	}

	if (listen(server_sockfd, SOMAXCONN) == -1) {
		perror("listen");
		return EXIT_FAILURE;
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	for (;;) {
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
			        "Error(%d): Unknown error parsing request.\n",
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
			        "Error: Response too large ('%d' bytes).\n",
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

	return EXIT_SUCCESS;
}
