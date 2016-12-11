/* Kind of Google search using libdill.
   Based on example from Rob Pike's talk at Google I/O 2012.
   Work-in-progress...
*/

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

struct Result {
    const char *kind;
    const char *query;
    const char *found;
};

struct Result fakeSearch(const char *kind, const char *query)
{
    char *tmp = malloc(128);
    assert(tmp);
    msleep(now() + rand() % 500);
    int n = snprintf(tmp, 128, "Nothing found.");
    tmp[n] = 0;
    struct Result r = {kind, query, tmp};
    return r;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int start = now();
    struct Result results[3];
    results[0] = fakeSearch("web", "golang");
    results[1] = fakeSearch("image", "golang");
    results[2] = fakeSearch("video", "golang");
    for(int i = 0; i < 3; ++i)
        printf("%s:\n\t%s\n\t\t%s\n", results[i].kind, results[i].query,
               results[i].found);
    int elapsed = now() - start;
    printf("%d ms\n", elapsed);

    return 0;
}
