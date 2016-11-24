/* Little boring program to learn how libdill supports concurrency.
   This one uses select/choose */

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

coroutine void boring(int start_from, int ch, int quitch)
{
    printf("Entering boring()\n");

    int done = 0;
    struct chclause clauses[] = {
        {CHSEND, ch, &start_from, sizeof(start_from)},
        {CHRECV, quitch, &done, sizeof(done)}
    };
    
    while(!done) {
        int rc = choose(clauses, 2, -1);
        assert(rc >= 0);
        ++start_from;
    }

    printf("Leaving boring()\n");
}

int boring_gen(int start_from, int quitch)
{
    int ch = channel(sizeof(int), 0);
    assert(ch >= 0);
    int cr = go(boring(start_from, ch, quitch));
    assert(cr >= 0);
    return ch;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    printf("Starting.\n");

    int quitch = channel(sizeof(int), 0);
    assert(quitch >= 0);

    int joech = boring_gen(10, quitch);
    assert(joech >= 0);

    int rc;
    int joe_count = 0;
    int max =  rand() % 10;
    printf("max: %d\n", max);

    for (int i = 0; i < max; ++i)
    {
        rc = chrecv(joech, &joe_count, sizeof(joe_count), -1);
        assert(rc == 0);
        printf("Joe's count: %d\n", joe_count);
    }

    /* Any non-zero value will end the loop in boring() */
    int done = 1;
    rc = chsend(quitch, &done, sizeof(done), -1);
    assert(rc == 0);

    /* This pause gives boring() enough time to exit */
    msleep(now() + 200);
    
    printf("Done.\n");
    return 0;
}
