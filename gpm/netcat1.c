// netcat1 is a read-only TCP client
// See chap 8 in The Go Programming Language

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#define log_fatal(msg, ...) \
	do {\
		fprintf(stderr, "Fatal %s:%d: " msg "\n",\
			__FILE__, __LINE__, ##__VA_ARGS__);\
		fflush(stderr);\
		abort();\
	} while(0)


int tcp_connect(const char* addr)
{
	return -1;
}

void must_copy(int dst, int src)
{
	assert(dst >= 0);
	assert(src >= 0);
}

int main()
{
	const char *addr = "localhost:8000";
	int conn = tcp_connect(addr);
	if (conn < 0) {
		log_fatal("could not connect to %s", addr);
	}

	//must_copy(stdout, conn);

	return 0;
}
