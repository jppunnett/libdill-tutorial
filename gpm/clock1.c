// clock1: TCP server that periodically writes the time.
// See chapter 8 in The Go Programming Language.
#include <stdio.h>
#include <libdill.h>
#include <assert.h>
#include <time.h>

void handleConn(int s)
{
	assert(s >= 0);

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

	rc = tcp_close(s, -1);
	if (rc != 0 && errno != ECONNRESET)
		perror("tcp_close");
}

int main()
{
	struct ipaddr addr;
	int rc = ipaddr_local(&addr, NULL, 8000, 0);
	assert(rc >= 0);

	int ls = tcp_listen(&addr, 10);
	assert(ls >= 0);

	while (1) {
		int s = tcp_accept(ls, NULL, -1);
		assert(s >= 0);
		handleConn(s);
	}

	rc = tcp_close(ls, -1);
	assert(rc == 0);

	return 0;
}

