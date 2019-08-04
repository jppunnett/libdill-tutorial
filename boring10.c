/* Little boring program to learn how libdill supports concurrency.
   This one uses select/choose and uses "quit" channel to signal when main and
   coroutine are done */

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

coroutine void boring(int start_from, int ch, int quit_ch[2])
{
    printf("Entering boring()\n");

    int rc = 0;
    int done = 0;

    struct chclause clauses[] = {
        {CHSEND, ch, &start_from, sizeof(start_from)},
        {CHRECV, quit_ch[0], &done, sizeof(done)}
    };

    while(!done) {
        rc = choose(clauses, 2, -1);
        assert(rc >= 0 && errno == 0);
        ++start_from;
    }

    printf("Leaving boring()\n");
    rc = chsend(quit_ch[1], &done, sizeof(done), -1);
    assert(rc == 0);
}

int boring_gen(int start_from, int quit_ch[2])
{
    int ch[2];
    int rc = chmake(ch);
    assert(rc == 0);
    int bundle = go(boring(start_from, ch[1], quit_ch));
    assert(bundle >= 0);
    return ch[0];
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    printf("Starting.\n");

    int rc;
    
    int quit_ch[2];
    rc = chmake(quit_ch);
    assert(rc == 0);

    int boring_ch = boring_gen(1, quit_ch);
    assert(boring_ch >= 0);

    int boring_count = 0;
    int max =  rand() % 10;
    printf("max: %d\n", max);
    
    int i;
    for (i = 0; i < max; ++i)
    {
        rc = chrecv(boring_ch, &boring_count, sizeof(boring_count), -1);
        assert(rc == 0);
        printf("Boring count: %d\n", boring_count);
    }

    /* Tell boring coroutine to finish up */
    int done = 1;
    rc = chsend(quit_ch[1], &done, sizeof(done), -1);
    assert(rc == 0);

    /* Wait on boring coroutine to confirm they cleaned up */
    rc = chrecv(quit_ch[0], &done, sizeof(done), -1);
    assert(rc == 0);

    printf("Done.\n");
    return 0;
}
