// clockwall displays time of several clocks once.
// See exercise 8.1 in The Go Programming Language


#include <stdio.h>
#include <assert.h>

#include <libdill.h>

#include "dill_helper.h"
#include "log.h"

void must_copy(FILE * dst, int src)
{
	assert(dst);
	assert(src >= 0);

	int rc;
	while (1) {
		unsigned char c;
		rc = brecv(src, &c, 1, -1);
		if (rc == -1 && ((errno == EPIPE) || (ECONNRESET)))
			break;
		fprintf(dst, "%c", c);
	}
}

coroutine void showClock(const char *host, int port)
{
	int conn = happyeyeballs_connect(host, port, LD_500_MS);
	if (conn < 0) {
		log_error("could not connect to %s:%d", host, port);
		return;
	}

	must_copy(stdout, conn);

	int rc = tcp_close(conn, LD_500_MS);
	if (rc != 0)
		log_error("Could not close connection");
}

static int getHostAndPort(const char *clock_spec, char *host, int *port)
{
	if (clock_spec == NULL || host == NULL || port == NULL) {
		errno = EINVAL;
		return -1;
	}

	char *pos = strchr(clock_spec, ':');
	if (pos == NULL) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("Usage: %s <host:port> [<host:port> ...]\n",
		       argv[0]);
		return 1;
	}

	int rc;
	int b = bundle();
	if (b < 0)
		log_fatal("Could not create bundle");

	while (--argc > 0) {
		printf("argv[argc] = %s\n", argv[argc]);
		char host[255];
		int port = 0;

		rc = getHostAndPort(argv[argc], host, &port);
		if (rc != 0) {
			log_error("getHostAndPort %s\n", argv[argc]);
			continue;
		}

		rc = bundle_go(b, showClock(host, port));
		if (rc < 0)
			log_fatal("Could not launch showClock");
	}

	rc = bundle_wait(b, -1);
	if (rc != 0)
		log_fatal("bundle_wait");

	hclose(b);

	return 0;
}
