/* libdill tutorial at http://libdill.org/tutorial.html
   Build with:
   $ cc -Wall -ldill -ldsock -o ldhello main.c
*/
#include <libdill.h>
#include <dsock.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int port = 5555;
	if(argc > 1) port = atoi(argv[1]);

	ipaddr addr;
	int rc = ipaddr_local(&addr, NULL, port, 0);
	assert(rc == 0);

	return 0;
}
