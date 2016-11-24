/* Little boring program to learn how libdill supports concurrency */

#include <libdill.h>

#include <stdio.h>

coroutine void boring(const char* msg)
{
    int i = 0;
    for(; 1; ++i) {
        printf("%s %d\n", msg, i);
        msleep(now() + 1000);
    }

}

int main(int argc, char const *argv[])
{
    go(boring("Boring!"));
    printf("I'm listening.\n");
    msleep(now() + 5000);
    printf("You're boring, I'm leaving.\n");
    return 0;
}

