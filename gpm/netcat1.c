// netcat1 is a read-only TCP client
// See chap 8 in The Go Programming Language

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <libdill.h>


#define LD_500_MS \
	(now() + 500)

#define get_errno() \
	(errno == 0 ? "none" : strerror(errno))

#define log_fatal(msg, ...) \
	do {\
		fprintf(stderr, "[ERROR] %s:%d (errno: %s): " msg "\n",\
			__FILE__, __LINE__, get_errno(), ##__VA_ARGS__);\
		fflush(stderr);\
		abort();\
	} while(0)


void must_copy(FILE *dst, int src)
{
	assert(dst);
	assert(src >= 0);

	int rc;
	while(1) {
		unsigned char c;
		rc = brecv(src, &c, 1, -1);
		if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
		fprintf(dst, "%c", c);
	}
}

int main()
{
	const char* host = "localhost";
	const int port = 8000;

	int conn = happyeyeballs_connect(host, port, LD_500_MS);
	if (conn < 0) {
		log_fatal("could not connect to %s:%d", host, port);
	}

	must_copy(stdout, conn);

	int rc = tcp_close(conn, LD_500_MS);
	if (rc != 0) {
		log_fatal("Could not close connection");
	}

	return 0;
}
