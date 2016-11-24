/* Little boring program to learn how libdill supports concurrency.
   This one uses select/choose */

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
        msleep(now() + (500 + (rand() % 1000)));
        ++start_from;
    }
}

int boring_gen(int start_from)
{
    int ch = channel(sizeof(int), 0);
    assert(ch >= 0);
    int cr = go(boring(start_from, ch));
    assert(cr >= 0);
    return ch;
}

coroutine void timeout(int ms, int ch)
{
    msleep(now() + ms);
    int done = 1;
    int rc = chsend(ch, &done, sizeof(done), -1);
    assert(rc == 0);
}

int timeout_gen(int ms)
{
    int ch = channel(sizeof(int), 0);
    assert(ch >= 0);
    int cr = go(timeout(ms, ch));
    assert(cr >= 0);
    return ch;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    printf("Starting.\n");

    int joe_count = 0;
    int done = 0;
    struct chclause clauses[] = {
        {CHRECV, boring_gen(10), &joe_count, sizeof(joe_count)},
        {CHRECV, timeout_gen(5000), &done, sizeof(done)}
    };
    
    while(!done) {
        int rc = choose(clauses, 2, -1);
        assert(rc >= 0);
        switch(rc) {
        case 0:
            printf("Joe's count: %d\n", joe_count);
            break;
        case 1:
            /* Joe took too long to answer. */
            printf("Boring! Leaving.\n");
            break;
        default:
            printf("No one ready...\n");
        }
    }
    printf("Done.\n");
    return 0;
}
