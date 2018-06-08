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
	
	if (bind(st, &server, server_len) == -1) {
		fprintf(stderr, "Error while binding, error code: %d\n", errno);
		return EXIT_FAILURE;
	}
	printf("Server started\n");

	int in_pipe[2];
	if (pipe(in_pipe) == -1) {
		fprintf(stderr, "Cannot create pipe, error code: %d", errno);
		return EXIT_FAILURE;
	}

	listen(st, 10);
	while (1) {
		int connected;
		if ((connected = accept(st, &server, &server_len)) == -1) {
			fprintf(stderr, "Cannot establish connection, error code: %d\n", errno);
			continue;
		}
		printf("Someone connected\n");
		fflush(stdout);

    	struct msghdr msg;
		memset(&msg, 0, sizeof(msghdr));
		struct cmsghdr *cmsg;
		int *fdptr;
		
		char iobuf[1];
		struct iovec io;
		io.iov_base = iobuf;
		io.iov_len = sizeof(iobuf);
		
		union {
			char buf[CMSG_SPACE(sizeof(int *))];
			struct cmsghdr align;
		} u;

		msg.msg_iov = &io;
		msg.msg_iovlen = 1;
		msg.msg_control = u.buf;
		msg.msg_controllen = sizeof(u.buf);

		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		fdptr = (int *) CMSG_DATA(cmsg);
		memcpy(fdptr, &in_pipe[1], sizeof(int));

    	if (sendmsg(connected, &msg, 0) == -1) {
			fprintf(stderr, "Error while sending file descriptor\n");		
			printf("Disconnected\n");
			fflush(stdout);
			continue;
		}
		
		do {
			ssize_t msg_len = read(in_pipe[0], buffer, BUF_SIZE);
			buffer[msg_len] = 0;
			fprintf(stderr, "%s\n", buffer);
		} while (strcmp(buffer, "Bye"));

		printf("Disconnected\n");
		fflush(stdout);

		if (close(connected) == -1) {
			fprintf(stderr, "Cannot close file descriptor for input connection, error code: %d\n", errno);
		}
	}
	int exit_code = 0;
	if (close(in_pipe[0] == -1)) {
		fprintf(stderr, "Cannot close socket, error code, error code: %d\n", errno);
		exit_code = EXIT_FAILURE;
	}
	if (close(st) == -1) {
		fprintf(stderr, "Cannot close socket, error code, error code: %d\n", errno);
		exit_code = EXIT_FAILURE;
	}
	if (unlink(address) == -1) {
		fprintf(stderr, "Cannot delete socket file, error code: %d\n", errno);
		exit_code = EXIT_FAILURE;
	}
	return 0;
}
