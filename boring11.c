/* Little boring program to learn how libdill supports concurrency.
   This uses a "Daisy Chain" to demo channel communication */

#include <libdill.h>
#include <stdio.h>
#include <assert.h>

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
    int cr = 0;
    
    int leftmost = channel(sizeof(int), 0);
    assert(leftmost >= 0);
    
    int right = leftmost;
    int left = leftmost;
    for (int i = 0; i < n; ++i)
    {
        right = channel(sizeof(int), 0);
        assert(right >= 0);

        cr = go(pass_it_on(left, right));
        assert(cr >= 0);
        
        left = right;
    }

    cr = go(start(right));
    assert(cr >= 0);

    int counter = 0;
    int rc = chrecv(leftmost, &counter, sizeof(counter), -1);
    assert(rc == 0);

    printf("Counter: %d\n", counter);
    return 0;
}
