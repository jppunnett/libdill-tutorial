// Calculates the 45th Fibonacci number with a visual indicator.
// Note: using yield() in fib() may allow the spinner to actually spin, but
// fib() takes a lot longer to complete. E.g. Over 2 minutes with yield()
// vs. 10 seconds without it.
#include <stdio.h>
#include <assert.h>
#include <libdill.h>

// Calculates Fibonacci of x.
static int fib(int x)
{
	// Need to yield or spinner will not have any time to spin.
	int rc = yield();
	assert(rc == 0);

	if (x < 2) return x;
	return fib(x - 1) + fib(x - 2);
}

coroutine void spinner(int delay)
{
	const char spinChars[] = {'-', '\\', '|', '/'};
	while (1) {
		for (int i = 0; i < sizeof(spinChars); i++) {
			printf("\r%c", spinChars[i]);
			msleep(now() + delay);
		}
	}
}

int main()
{
	// Turn off buffering on stdout otherwise we won't see the spinner.
	setbuf(stdout, NULL);

	int rc = go(spinner(500));
	assert(rc != -1);

	const int n = 45;
	int fibN = fib(n);
	printf("\rFibonacci(%d) = %d\n", n, fibN);
}

