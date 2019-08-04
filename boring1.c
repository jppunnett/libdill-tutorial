/* Little boring program to learn how libdill supports concurrency */

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void printerr(int err)
{
    switch(err) {
    case EBADF:
        printf("EBADF: Invalid handle\n");
        break;
    case ECANCELED:
        printf("ECANCELED: Current coroutine is being shut down.\n");
        break;
    case EINVAL:
        printf("EINVAL: Invalid parameter.\n");
        break;
    case ENOTSUP:
        printf("ENOTSUP: Operation not supported.\n");
        break;
    case EPIPE:
        printf("EPIPE: The channel was closed using chdone function.\n");
        break;
    case ETIMEDOUT:
        printf("ETIMEDOUT: The deadline was reached while waiting for a message.\n");
        break;
    default:
        printf("err: %d\n", err);
        break;
    }
}

coroutine void boring(const char* msg, int ch)
{
    int i;
    for(i = 0; i < 5; ++i) {
        char *buf = malloc(128);
        assert(buf);
        int n = snprintf(buf, 128, "%s %d", msg, i);
        assert(n >= 0);
        buf[n] = 0;
        int rc = chsend(ch, &buf, sizeof(&buf), -1);
        if(rc != 0) printerr(errno);
        assert(rc == 0);
    }
}

int main(int argc, char const *argv[])
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    printf("I'm listening.\n");

    go(boring("Boring!", ch[1]));

    int i;
    for(i = 0; i < 5; ++i) {
        char *msg = NULL;
        int rc = chrecv(ch[0], &msg, sizeof(msg), -1);
        assert(rc == 0);
        assert(msg);
        printf("You say: %s\n", msg);
        free(msg);
        msleep(now() + 1000);
    }

    printf("You're boring, I'm leaving.\n");
    return 0;
}
