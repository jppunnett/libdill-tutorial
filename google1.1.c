/* Kind of Google search using libdill.
   Based on example from Rob Pike's talk at Google I/O 2012.
   Work-in-progress...
*/

#include <libdill.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int ch;

struct Result {
    const char *kind;
    const char *query;
    char *found;
};

coroutine void fakeSearch(const char *kind, const char *query)
{
    char *tmp = malloc(128);
    assert(tmp);
    msleep(now() + rand() % 500);
    int n = snprintf(tmp, 128, "Nothing found for %s search.", kind);
    tmp[n] = 0;
    
    struct Result *r = malloc(sizeof(struct Result));
    assert(r);
    r->kind = kind;
    r->query = query;
    r->found = tmp;

    int rc = chsend(ch, &r, sizeof(r), -1);
    assert(rc == 0);
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    ch = channel(sizeof(struct Result*), 1);
    assert(ch >= 0);

    int start = now();
    go(fakeSearch("web", "golang"));
    go(fakeSearch("image", "golang"));
    go(fakeSearch("video", "golang"));
    
    int rc = 0;
    struct Result *r = NULL;
    for(int i = 0; i < 3; ++i) {
        rc = chrecv(ch, &r, sizeof(r), -1);
        assert(rc == 0);
        printf("%s:\n\t%s\n\t\t%s\n", r->kind, r->query, r->found);
        free(r->found);
        free(r);
    }
    int elapsed = now() - start;
    printf("%d ms\n", elapsed);

    return 0;
}
