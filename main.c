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

/* Handler for client connection */
void handle_conn(int );

/* Ten-second deadline */
#define DEADLINE (now() + 10000)


int main(int argc, char const *argv[]) {
    int port = 5555;
    if(argc > 1) port = atoi(argv[1]);
    /* Create an IP address to bind to all interfaces */
    ipaddr addr;
    int rc = ipaddr_local(&addr, NULL, port, 0);
    assert(rc == 0);
    /* Create a socket to listen at addr for incoming connections */
    int ls = tcp_listen(&addr, 10);
    if(ls < 0) {
        perror("Can't open listening socket.");
        return 1;
    }
    /* Listen for incoming connection */
    while(1) {
        int s = tcp_accept(ls, NULL, DEADLINE);
        if(s < 0) {
            if(errno == ETIMEDOUT)
                continue;
            else
                break;
        }
        int cr = go(handle_conn(s));
        assert(cr >= 0);
    }
    return 0;
}

coroutine void handle_conn(int s) {
    printf("New connection. Socket id: %d\n", s);
    s = crlf_start(s);
    assert(s >= 0);
    /* Send a message to the client */
    int rc = msend(s, "Hey! What's your name?", 22, DEADLINE);
    if(rc != 0) goto cleanup;
    printf("msend1: rc=%d, errno=%d\n", rc, errno);
    char name[256];
    /* Receive the client's name */
    size_t sz = mrecv(s, name, sizeof(name), DEADLINE);
    printf("mrecv: sz=%zu, errno=%d\n", sz, errno);
    /* It's possible for sz >= 0 and for errno to != 0 */
    if(sz < 0 || errno) goto cleanup;
    name[sz] = 0;
    /* Send the client a polite greeting */
    char greeting[256];
    rc = snprintf(greeting, sizeof(greeting), "Hola, %s!", name);
    rc = msend(s, greeting, rc, DEADLINE);
    printf("msend2: rc=%d, errno=%d\n", rc, errno);

cleanup:
    rc = hclose(s);
    assert(rc == 0);
}
