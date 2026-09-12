#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd == -1) {
		perror("Error while trying to create IP v4 socket");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr = {
	    .sin_family = AF_INET,
	    .sin_port = htons(9090),
	    .sin_addr = {.s_addr = htonl(INADDR_ANY)}};
	if (bind(server_sockfd, (struct sockaddr *)(&server_addr),
	         sizeof(server_addr)) == -1) {
		perror("Error while trying to bind the socket");
		return EXIT_FAILURE;
	}

	if (listen(server_sockfd, SOMAXCONN) == -1) {
		perror("Error while trying to listen on socket");
		return EXIT_FAILURE;
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	while (true) {
		int client_sockfd =
		    accept(server_sockfd, (struct sockaddr *)(&client_addr),
		           &client_addr_len);
		if (client_sockfd == -1) {
			perror(
			    "Error while trying to accept socket connection");
			continue;
		}

		char data[] = "HTTP/1.1 404\r\n"
		              "Content-Length: 0\r\n"
		              "\r\n";

		if (send(client_sockfd, data, sizeof(data) - 1, 0) == -1) {
			perror("Error while trying to send response");
			close(client_sockfd);
			continue;
		}

		close(client_sockfd);
	}

	return EXIT_SUCCESS;
}
