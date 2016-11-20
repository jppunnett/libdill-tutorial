/* libdill tutorial at http://libdill.org/tutorial.html
   Build with:
   $ cc -Wall -ldill -ldsock -o ldhello.out main.c
*/
#include <libdill.h>
#include <dsock.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    int port = 5555;
    if(argc > 1) port = atoi(argv[1]);
    /* Create an IP address to bind to all interfaces */
    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    assert(rc == 0);
    /* Create a socket to aisten at addr for incoming connections */
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket.");
        return 1;
    }
    /* Listen for incoming connection */
    while(1) {
        int s = tcp_accept(ls, NULL, -1);
        assert(s >= 0);
        printf("New connect. Socket id: %d\n", s);
        hclose(s);
    }

    return 0;
}
