/* Little boring program to learn how libdill supports concurrency.
   This one uses the Fan-in "pattern" */

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

coroutine void boring(const char* msg, int ch)
{
    int i = 0;
    while(1) {
        char *buf = malloc(128);
        assert(buf);
        int n = snprintf(buf, 128, "%s %d", msg, i);
        assert(n >= 0);
        buf[n] = 0;
        int rc = chsend(ch, &buf, sizeof(buf), -1);
        assert(rc == 0);
        msleep(now() + (500 + (rand() % 1000)));
        ++i;
    }
}

coroutine void fanIn(int child_ch, int parent_ch)
{
    while(1) {
        /* receive msg from child */
        char *msg = NULL;
        int rc = chrecv(child_ch, &msg, sizeof(msg), -1);
        assert(rc == 0);
        assert(msg);
        /* Send to parent */
        rc = chsend(parent_ch, &msg, sizeof(msg), -1);
        assert(rc == 0);
    }
}

int fanIn_gen(int ch1, int ch2)
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);

    int hbundle;
    hbundle = go(fanIn(ch1, ch[1]));
    assert(hbundle >= 0);
    
    hbundle = go(fanIn(ch2, ch[1]));
    assert(hbundle >= 0);
    
    return ch[0];
}

int boring_gen(const char* msg)
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    int hbundle = go(boring(msg, ch[1]));
    assert(hbundle >= 0);
    return ch[0];
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));

    printf("I'm listening.\n");
    int ch = fanIn_gen(boring_gen("Joe"), boring_gen("Ann"));
    assert(ch >= 0);
    
    int i;
    for(i = 0; i < 10; ++i) {
        char *msg = NULL;
        int rc = chrecv(ch, &msg, sizeof(msg), -1);
        assert(rc == 0);
        assert(msg);
        printf("%s\n", msg);
        free(msg);
    }
    printf("You're both boring, I'm leaving.\n");
    return 0;
}
