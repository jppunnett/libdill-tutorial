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
        int s = tcp_accept(ls, NULL, -1);
        assert(s >= 0);
        handle_conn(s);
    }

    return 0;
}

void handle_conn(int s) {
    printf("New connection. Socket id: %d\n", s);
    s = crlf_start(s);
    assert(s >= 0);
    /* Send a message to the client */
    int rc = msend(s, "Hey! What's your name?", 22, -1);
    if(rc != 0) goto cleanup;
    /* Receive the client's name */
    char name[256];
    size_t sz = mrecv(s, name, sizeof(name), -1);
    if(sz < 0) goto cleanup;
    name[sz] = 0;
    /* Send the client a polite greeting */
    char greeting[256];
    rc = snprintf(greeting, sizeof(greeting), "Hola, %s!", name);
    msend(s, greeting, rc, -1);

cleanup:
    rc = hclose(s);
    assert(rc == 0);
}