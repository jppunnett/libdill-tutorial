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
        int64_t sleep_for = now() + (500 + (rand() % 1000));
        msleep(sleep_for);
        ++start_from;
    }
}

int boring_gen(int start_from)
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    int bundle = go(boring(start_from, ch[1]));
    assert(bundle >= 0);
    return ch[0];
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
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    int bundle = go(timeout(ms, ch[1]));
    assert(bundle >= 0);
    return ch[0];
}

int main(int argc, char const *argv[])
{
    const int wait_for_ms = 1000;

    srand(time(NULL));
    printf("Starting.\n");

    int boring_count = 0;
    int done = 0;
    struct chclause clauses[] = {
        {CHRECV, boring_gen(10), &boring_count, sizeof(boring_count)},
        {CHRECV, timeout_gen(wait_for_ms), &done, sizeof(done)}
    };
    
    while(!done) {
        int rc = choose(clauses, 2, -1);
        assert(rc >= 0 && errno ==0);
        switch(rc) {
        case 0:
            printf("Boring count: %d\n", boring_count);
            /* Reset timer */
            clauses[1].ch = timeout_gen(wait_for_ms);
            break;
        case 1:
            /* Boring took too long to answer. */
            printf("Too long, Boring! Leaving.\n");
            break;
        default:
            printf("No one ready...\n");
        }
    }
    printf("Done.\n");
    return 0;
}
