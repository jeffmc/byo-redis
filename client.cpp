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

int main() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		die("socket()");
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = ntohs(1234);
	addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
	int rv = connect(fd, (const sockaddr*)&addr, sizeof(addr));
	if (rv) {
		die("connect()");
	}

	char wbuf[] = "hello";
	write(fd, wbuf, strlen(wbuf));
	
	char rbuf[64] = {};
	ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
	if (n < 0) {
		die("read()");
	}
	printf("server says: %s\n", rbuf);
	
	return 0;
}
