#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>


const size_t BUF_SIZE = 4096;
char buffer[BUF_SIZE];
char *socket_name = "game_socket";

int main(int argc, char **argv) {
	if (argc != 1) {
		printf("No arguments expected\n");
		return 0;
	}
	sockaddr server;

	int st;
	if ((st = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Cannot create socket, error code: %d\n", errno);
		return EXIT_FAILURE;
	}

	server.sa_family = AF_UNIX;
	strcpy(server.sa_data, socket_name);
	
	if (bind(st, &server, sizeof(server.sa_family) + strlen(server.sa_data)) == -1) {
		fprintf(stderr, "Error while binding, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	printf("Server started\n");

	int i_pipe[2];
	int o_pipe[2];
	if (pipe(i_pipe) == -1 || pipe(o_pipe) == -1) {
		fprintf(stderr, "Cannot create pipe");
		return EXIT_FAILURE;
	}

	ssize_t len;
	while (1) {
		if ((len = recvfrom(st, buffer, BUF_SIZE, 0, NULL, NULL)) < 0) {
			fprintf(stderr, "Error while receiving message, error code: %d\n", errno);
			continue;
		}
		buffer[len] = 0;
		if (!strcmp(buffer, "exit")) {
			printf("Shutting down server\n");
			break;
		}
		printf("%s connected\n", buffer);
		
	}
	if (close(st) == -1) {
		fprintf(stderr, "Cannot close socket, error code, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	if (unlink(socket_name) == -1) {
		fprintf(stderr, "Cannot delete socket file, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	return 0;
}
