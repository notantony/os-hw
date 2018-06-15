#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>


const size_t BUF_SIZE = 4096;
char buffer[BUF_SIZE];

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Wrong format, usage: server <address>\n");
		return 0;
	}
	sockaddr server;
	if (strlen(argv[1]) + 1 > sizeof(server.sa_data)) {
		printf("Address is too long\n");
		return 0;
	}

	int st;
	if ((st = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Cannot create socket, error code: %d\n", errno);
		return EXIT_FAILURE;
	}

	server.sa_family = AF_UNIX;
	strcpy(server.sa_data, argv[1]);
	
	if (bind(st, &server, sizeof(server.sa_family) + strlen(server.sa_data)) == -1) {
		fprintf(stderr, "Error while binding, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	printf("Server started\n");


	ssize_t clen;
	while (1) {
		ssize_t len = 0;
		int flag = 0;
		while ((clen = recvfrom(st, buffer + len, BUF_SIZE - len - 1, MSG_DONTWAIT * flag, NULL, NULL)) > 0) {
			flag = 1;
			fprintf(stderr, "Received %ld bytes\n", clen);	
			len += clen;	
		}
		if (clen < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			fprintf(stderr, "Error while receiving message, error code: %d\n", errno);
			continue;
		}
		buffer[len] = 0;
		if (!strcmp(buffer, "exit")) {
			printf("Shutting down server\n");
			break;
		}
		printf("Hello, %s!\n", buffer);
	}
	if (close(st) == -1) {
		fprintf(stderr, "Cannot close socket, error code, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	if (unlink(argv[1]) == -1) {
		fprintf(stderr, "Cannot delete socket file, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	return 0;
}
