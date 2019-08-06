/* Little boring program to learn how libdill supports concurrency.
   This uses a "Daisy Chain" to demo channel communication */

#include <libdill.h>
#include <stdio.h>
#include <assert.h>

#define CH_COPY(ch_from, ch_to) do { ch_to[0] = ch_from[0]; ch_to[1] = ch_from[1];; } while(0)

coroutine void pass_it_on(int left_ch, int right_ch)
{
    int rc = 0;
    int counter = 0;
    /* Receive counter value from the right */
    rc = chrecv(right_ch, &counter, sizeof(counter), -1);
    assert(rc == 0);
    /* Increment and pass to the left */
    counter++;
    rc = chsend(left_ch, &counter, sizeof(counter), -1);
    assert(rc == 0);
}

coroutine void start(int right)
{
    int i = 1;
    int rc = chsend(right, &i, sizeof(i), -1);
    assert(rc == 0);
}

int main(int argc, char const *argv[])
{
    const int n = 10000;
    int counter = 0;
    
    int bundle = 0;
    int rc = 0;
    
    int leftmost[2];
    rc = chmake(leftmost);
    assert(rc == 0);
    
    int right[2];
    CH_COPY(leftmost, right);
    
    int left[2];
    CH_COPY(leftmost, left);
    
    int i;
    for(i = 0; i < n; ++i) {
        rc = chmake(right);
        assert(rc == 0);

        bundle = go(pass_it_on(left[1], right[0]));
        assert(bundle >= 0);
        
        CH_COPY(right, left);
    }

    bundle = go(start(right[1]));
    assert(bundle >= 0);

    
    rc = chrecv(leftmost[0], &counter, sizeof(counter), -1);
    assert(rc == 0);

    printf("Counter: %d\n", counter);
    return 0;
}
