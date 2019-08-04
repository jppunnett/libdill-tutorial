/* Little boring program to learn how libdill supports concurrency.
   This one uses choose */

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

coroutine void boring(int start_from, int ch)
{
    while(1) {
        int rc = chsend(ch, &start_from, sizeof(start_from), -1);
        assert(rc == 0);
        msleep(now() + rand() % 1000);
        ++start_from;
    }
}

int boring_gen(int start_from)
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    int hbundle = go(boring(start_from, ch[1]));
    assert(hbundle >= 0);
    return ch[0];
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    printf("Starting.\n");

    int joe = boring_gen(10);
    assert(joe >= 0);
    int ann = boring_gen(20);
    assert(ann >= 0);
    
    int joe_count = 0;
    int ann_count = 0;

    struct chclause clauses[] = {
        {CHRECV, joe, &joe_count, sizeof(joe_count)},
        {CHRECV, ann, &ann_count, sizeof(ann_count)}
    };
    
    int i;
    for(i = 0; i < 10; ++i) {
        int rc = choose(clauses, 2, -1);
        assert(rc >= 0 && errno == 0);
        int *count = clauses[rc].val;
        assert(count);
        printf("Count %d\n", *count);
    }
    printf("Done.\n");
    return 0;
}
