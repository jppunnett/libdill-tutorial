/* Compute disk usage of files in a dir. Adapted from The Go Programming
   Language book.
*/
#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <libdill.h>

/* Channel for file sizes */
int filesz = -1;

static int
recordsz(const char *fpath, const struct stat *sb,
         int tflag, struct FTW *ftwbuf)
{
    int fsize = 0, rc = 0;
    if(tflag == FTW_F) {
        fsize = sb->st_size;
        rc = chsend(filesz, &fsize, sizeof(fsize), -1);
        if(rc != 0) return 1;
    }
    return 0;
}

coroutine void
walkdirs(char *dirs[], int ndirs)
{
    int rc = 0;
    for(int i = 0; i < ndirs; ++i) {
        rc = nftw(dirs[i], recordsz, 20, FTW_PHYS);
        if(rc != 0) {
            perror("nftw");
            exit(EXIT_FAILURE);
        }
    }
    rc = chdone(filesz);
    if(rc != 0) {
        perror("chdone");
        exit(EXIT_FAILURE);
    }
}

char *curdir[] = {"."};

int
main(int argc, char *argv[])
{
    /* Determine initial directories */
    char **roots = curdir;
    int ndirs = 1;
    if(argc > 1) {
        roots = &argv[1];
        ndirs = argc - 1;   
    }
    /* Traverse directories */
    filesz = channel(sizeof(int), 1);
    if(filesz < 0) {
        perror("channel");
        exit(EXIT_FAILURE);
    }
    int rc = go(walkdirs(roots, ndirs));
    if(rc < 0) {
        perror("go");
        exit(EXIT_FAILURE);
    }
    /* Collect ad print results */
    int nfiles = 0, nbytes = 0;
    int size = 0;
    while(1) {
        rc = chrecv(filesz, &size, sizeof(size), -1);
        if(rc != 0) {
            if(errno == EPIPE) {
                /* Sender done with channel */
                break;
            } else {
                perror("chrecv");
                exit(EXIT_FAILURE);
            }
        }
        nfiles++;
        nbytes += size;
    }
    printf("%d files\t%.1f GB\n", nfiles, nbytes / 1e9);
    exit(EXIT_SUCCESS);
}