#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

static void lnmsg(int ln, const char* msg) {
	printf("%i: %s\n", ln, msg);
}
#define die(msg) lnmsg(__LINE__, msg); exit(1)
#define msg(msg) lnmsg(__LINE__, msg)

static void do_something(int);

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		die("socket()");
	}
		
	int val = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
	int rv = bind(fd, (const sockaddr*)&addr, sizeof(addr));
	if (rv) {
		die("bind()");
	}

	// listen
	rv = listen(fd, SOMAXCONN);
	if (rv) {
		die("listen()");
	}

	while (true) {
		// accept
		struct sockaddr_in client_addr = {};
		socklen_t socklen = sizeof(client_addr);
		int connfd = accept(fd, (struct sockaddr*)&client_addr, &socklen);
		if (connfd < 0) {
			continue; // error
		}

		do_something(fd);	
		close(connfd);
	}	
	return 0;
}

static void do_something(int connfd) {
	char rbuf[64] = {};
	ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		msg("read() error");
		return;
	}
	printf("client says: %s\n", rbuf);

	char wbuf[] = "world";
	write(connfd, wbuf, strlen(wbuf));
}