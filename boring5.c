/* Little boring program to learn how libdill supports concurrency.
   This one uses choose (like Go lang select clause) */

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
        msleep(now() + rand() % 1000);
        ++i;
    }
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

    int joe = boring_gen("Joe");
    assert(joe >= 0);
    int ann = boring_gen("Ann");
    assert(ann >= 0);
    
    char *joe_says = NULL;
    char *ann_says = NULL;

    struct chclause clauses[] = {
        {CHRECV, joe, &joe_says, sizeof(&joe_says)},
        {CHRECV, ann, &ann_says, sizeof(&ann_says)}
    };
    
    int i;
    for(i = 0; i < 10; ++i) {
        int rc = choose(clauses, 2, -1);
        assert(rc >= 0 && errno == 0);
        char **msg = clauses[rc].val;
        assert(msg);
        printf("%s\n", *msg);
        free(*msg);
    }
    printf("You're both boring, I'm leaving.\n");
    return 0;
}
