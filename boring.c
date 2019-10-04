/* Little boring program to learn how libdill supports concurrency */

#include <stdio.h>
#include <libdill.h>

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
    printf("I'm listening.\n");
    go(boring("Boring!"));
    msleep(now() + 5000);
    printf("You're boring, I'm leaving.\n");
    return 0;
}

