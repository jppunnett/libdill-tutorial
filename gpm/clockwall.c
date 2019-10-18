// clockwall displays time of several clocks once.
// See exercise 8.1 in The Go Programming Language


#include <stdio.h>
#include <assert.h>

#include <libdill.h>

#include "dill_helper.h"
#include "log.h"

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

int main(int argc, char *argv[])
{
	int rc;
	int b = bundle();
	if (b < 0)
		log_fatal("Could not create bundle");
	
	rc = bundle_go(b, showClock("localhost", 8000));
	if (rc < 0)
		log_fatal("Could not launch showClock");
	
	rc = bundle_go(b, showClock("localhost", 8001));
	if (rc < 0)
		log_fatal("Could not launch showClock");
	
	rc = bundle_wait(b, -1);
	if (rc != 0)
		log_fatal("bundle_wait");

	hclose(b);

	return 0;
}

