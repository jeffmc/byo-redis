#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <vector>
#include <string>

void lnmsg(int ln, const char* msg) {
	printf("%i: %s\n", ln, msg);
}
#define die(msg) lnmsg(__LINE__, msg); exit(1)
#define msg(msg) lnmsg(__LINE__, msg)

int32_t read_full(int fd, char *buf, size_t n) {
	while (n > 0) {
		ssize_t rv = read(fd, buf, n);
		if (rv <= 0) {
			return -1; // error, or unexpected EOF
		}
		assert((size_t)rv <= n);
		n -= (size_t)rv;
		buf += rv;
	}
	return 0;
}

int32_t write_all(int fd, const char* buf, size_t n) {
	while (n > 0) {
		ssize_t rv = write(fd, buf, n);
		if (rv <= 0) {
			return -1;
		}
		assert((size_t)rv <= n);
		n -= (size_t)rv;
		buf += rv;
	}
	return 0;
}


const size_t k_max_msg = 4096;
const size_t k_max_args = 5;

void fd_set_nb(int fd) {
	errno = 0;
	int flags = fcntl(fd, F_GETFL, 0);
	if (errno) {
		die("fcntl error");
		return;
	}

	flags |= O_NONBLOCK;

	errno = 0;
	(void)fcntl(fd, F_SETFL, flags);
	if (errno) {
		die("fcntl error");
	}
}

enum {
	SER_NIL = 0, // Like null
	SER_ERR = 1, // An error code and a message
	SER_STR = 2, // A string
	SER_INT = 3, // An int64
	SER_ARR = 4, // Array
};

enum {
	STATE_REQ = 0,
	STATE_RES = 1,
	STATE_END = 2,
};

struct Conn {
	int fd = -1;
	uint32_t state = 0;
	// buffer for reading
	size_t rbuf_size = 0;
	uint8_t rbuf[4 + k_max_msg];
	// buffer for writing
	size_t wbuf_size = 0;
	size_t wbuf_sent = 0;
	uint8_t wbuf[4 + k_max_msg];
};

enum {
	RES_OK = 0,
	RES_ERR = 1,
	RES_NX = 2,
};
