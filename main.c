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
void handle_conn(int, int);
/* Statistics coroutine */
void stats(int);

/* Ten-second deadline */
#define DEADLINE (now() + 10000)

/* Events to track */
#define CONN_ESTABLISHED    1
#define CONN_SUCCEEDED      2
#define CONN_FAILED         3

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
    /* Channel for stats and handler coroutines to communicate. */
    int ch = channel(sizeof(int), 0);
    assert(ch >= 0);
    /* Launch the stats coroutine */
    int cr = go(stats(ch));
    assert(cr >= 0);

    /* Forever listen for incoming connection */
    while(1) {
        int s = tcp_accept(ls, NULL, DEADLINE);
        if(s < 0 || errno != ETIMEDOUT) {
            perror("tcp_accept failed.");
            break;
        }
        int cr = go(handle_conn(s, ch));
        assert(cr >= 0);
    }
    return 0;
}

coroutine void handle_conn(int s, int ch) {
    s = crlf_start(s);
    assert(s >= 0);
    /* Record established connection */
    int op = CONN_ESTABLISHED;
    int rc = chsend(ch, &op, sizeof(op), -1);
    assert(rc >= 0);
    /* Send a message to the client */
    rc = msend(s, "Hey! What's your name?", 22, DEADLINE);
    if(rc != 0) goto cleanup;
    printf("msend1: rc=%d, errno=%d\n", rc, errno);
    char name[256];
    /* Receive the client's name */
    ssize_t sz = mrecv(s, name, sizeof(name), DEADLINE);
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
    /* Record connection status */
    op = errno == 0 ? CONN_SUCCEEDED : CONN_FAILED;
    rc = chsend(ch, &op, sizeof(op), -1);
    assert(rc == 0);
    rc = hclose(s);
    assert(rc == 0);
}

coroutine void stats(int ch) {
    int active = 0, succeeded = 0, failed = 0;
    while(1) {
        int op;
        int rc = chrecv(ch, &op, sizeof(op), -1);
        assert(rc == 0);

        switch(op) {
        case CONN_ESTABLISHED:
            ++active;
            break;
        case CONN_SUCCEEDED:
            --active;
            ++succeeded;
            break;
        case CONN_FAILED:
            --active;
            ++failed;
            break;
        }

        printf("active: %-5d  succeeded: %-5d  failed: %-5d\n",
                    active, succeeded, failed);
    }
}