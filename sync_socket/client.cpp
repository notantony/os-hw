#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>


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
	ssize_t len;
	ssize_t clen, msg_len;
    while (scanf("%s", buffer) != EOF) {
		msg_len = strlen(buffer);
		len = 0;
		while (len < msg_len) {
			clen = sendto(st, buffer + len, msg_len - len, 0, &server, sizeof(server.sa_family) + strlen(server.sa_data));
			if (clen == -1) {
				fprintf(stderr, "Error while sending message, error code: %d\n", errno);
				break;
			}
			len += clen;
		}
    }
    return 0;
}