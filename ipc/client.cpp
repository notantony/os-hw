#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>

const size_t BUF_SIZE = 4096;
char buffer[BUF_SIZE];
const char *address = "test.sock";


int main(int argc, char **argv) {
    if (argc != 1) {
		printf("Wrong format: no arguments expected\n");
		return 0;
	}
	sockaddr server;

    int st;
    if ((st = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Cannot create socket, error code: %d\n", errno);
		return EXIT_FAILURE;
	}

	server.sa_family = AF_UNIX;
	strcpy(server.sa_data, address);
	socklen_t server_len  = sizeof(server.sa_family) + strlen(server.sa_data);

	if (connect(st, &server, server_len) == -1) {
		fprintf(stderr, "Cannot connect to server, error code: %d\n", errno);
		return EXIT_FAILURE;
	}

	printf("Connection established\n");
	
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;


	ssize_t msg_len;
    if (msg_len = recvmsg(st, &msg, 0) == -1) {
		fprintf(stderr, "Error while receiving pipe file descriptor\n");
		return -1;
	}

    int pipe_fd;
    cmsg = CMSG_FIRSTHDR(&msg); 
    memmove(&pipe_fd, CMSG_DATA(cmsg), sizeof(int));

	const char *hello = "Hello, this is pipe";
	if (write(pipe_fd, hello, strlen(hello)) == -1) {
		fprintf(stderr, "Error while sending message, error code: %d\n", errno);
	}
    while (scanf("%s", buffer) != EOF) {
		if (write(pipe_fd, buffer, strlen(buffer)) == -1) {
			fprintf(stderr, "Error while sending message, error code: %d\n", errno);
		}
		if (!strcmp(buffer, "Bye")) {
			return 0;
		}
	}
	const char *bye = "Bye";
	if (write(pipe_fd, bye, strlen(bye)) == -1) {
		fprintf(stderr, "Error while sending message, error code: %d\n", errno);
	}
    return 0;
}