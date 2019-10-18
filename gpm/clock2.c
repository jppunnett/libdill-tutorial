// clock2: TCP server that periodically writes the time.
// This variation handles connections concurrently.
// See chapter 8 in The Go Programming Language.

#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <libdill.h>

#include "log.h"

coroutine void handleConn(int s)
{
	assert(s >= 0);
	printf("Connection on socket %d\n", s);

	int rc;
	time_t dt;
	char buf[128];
	int written;
	
	while (1) {
		time(&dt);
		written = snprintf(buf, sizeof(buf), "%s",
				asctime(localtime(&dt)));
		assert(written > 0 && written < sizeof(buf));

		rc = bsend(s, buf, written, -1);
		if (rc != 0) break;

		rc = msleep(now() + 1000);
		assert(rc == 0);
	}

	printf("Disonnecting from socket: %d\n", s);

	rc = tcp_close(s, -1);
	if (rc != 0 && errno != ECONNRESET)
		perror("tcp_close");
}

int main(int argc, char *argv[])
{
	int port = 8000;

	struct ipaddr addr;
	int rc = ipaddr_local(&addr, NULL, port, 0);
	if (rc < 0)
		log_fatal("Could build localhost address on port %d", port);

	int ls = tcp_listen(&addr, 10);
	if (ls < 0)
		log_fatal("Could not listen on port %d", port);

	printf("Listening on port %d\n", port);

	while (1) {
		int s = tcp_accept(ls, NULL, -1);
		if (s < 0) {
			perror("tcp_accept");
			goto cleanup;
		}

		rc = go(handleConn(s));
		if (rc < 0) {
			perror("Could not launch handleConn");
			goto cleanup;
		}
	}

cleanup:
	rc = tcp_close(ls, -1);
	if (rc != 0)
		log_fatal("Could not close connection.");

	return 0;
}

