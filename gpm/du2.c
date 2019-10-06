/* Compute disk usage of files in a dir. Adapted from The Go Programming
   Language book.
*/
#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <libdill.h>

/* Channel to communicate file sizes */
int filesz_ch[2];

static int
recordsz(const char *fpath, const struct stat *sb,
         int tflag, struct FTW *ftwbuf)
{
    int64_t fsize = 0;
    int rc = 0;
    if(tflag == FTW_F) {
        fsize = sb->st_size;
        rc = chsend(filesz_ch[1], &fsize, sizeof(fsize), -1);
        if(rc != 0) return 1;
    }
    return 0;
}

coroutine void
walkdirs(char *dirs[], int ndirs)
{
    int rc = 0;
    for(int i = 0; i < ndirs; ++i) {
        // printf("dirs[i] = %s\n", dirs[i]);
        rc = nftw(dirs[i], recordsz, 20, FTW_PHYS);
        if(rc != 0) {
            perror("nftw");
            exit(EXIT_FAILURE);
        }
    }
    rc = chdone(filesz_ch[1]);
    if(rc != 0) {
        perror("chdone");
        exit(EXIT_FAILURE);
    }
}

coroutine void
time_startTick(int ch, int64_t interval)
{
    int rc;
    int64_t t;
    while(1) {
        rc = msleep(now() + interval);
        if(rc == -1) return;
        t = now();
        rc = chsend(ch, &t, sizeof(t), -1);
        if(rc == -1) return; 
    }
}

/* time_Tick returns a channel through which the caller will receive the current
   time every interval (ms). */
int
time_Tick(int64_t interval)
{
    int ch[2];
    int rc = chmake(ch);
    if(rc != 0) {
	    perror("Could not make timer channel.");
	    return -1;
    }
    /* An interval of zero means never tick, but we return the channel so the
       caller can still receive on the channel. In this case it will never 
       receive any tick. */
    if(interval == 0) return ch[0];

    rc = go(time_startTick(ch[1], interval));
    if(rc < 0) {
	    perror("Could not start ticker coroutine.");
	    return -1;
    }

    return ch[0];
}

void
printDirUsage(int64_t nfiles, int64_t nbytes)
{
    printf("%ld files\t%.1f GB\n", nfiles, nbytes / 1e9);
}


char *curdir[] = {"."};

int
main(int argc, char *argv[])
{
    int verbose = 0;
    int c;
    while((c = getopt(argc, argv, "v")) != -1) {
        switch(c) {
        case 'v':
            verbose = 1;
            break;
        }
	// TODO: Add help option
    }
    /* Default to current directory if no directories provided */
    char **roots = curdir;
    int ndirs = 1;
    if(optind < argc) {
        roots = &argv[optind];
        ndirs = argc - optind;   
    }

    int rc;

    /* Start coroutine to traverse directories */
    rc = chmake(filesz_ch);
    if(rc != 0) {
        perror("Cannot make file size channel.");
        exit(EXIT_FAILURE);
    }
    
    int bundle = go(walkdirs(roots, ndirs));
    if(bundle < 0) {
        perror("go");
        exit(EXIT_FAILURE);
    }

    /* Start ticker to periodically display results */
    int ticker_ch = time_Tick(verbose ? 500 : 0);
    if(ticker_ch < 0) {
        perror("time_Tick");
        exit(EXIT_FAILURE);
    }
    int64_t size = 0;
    int64_t tick = 0;
    struct chclause clauses[] = {
        {CHRECV, filesz_ch[0], &size, sizeof(size)},
        {CHRECV, ticker_ch, &tick, sizeof(tick)}
    };
    
    int64_t nfiles = 0, nbytes = 0;
    int done = 0;
    while(!done) {
        rc = choose(clauses, 2, -1);
        switch(rc) {
        case 0:
            if(errno == EPIPE) {
                /* Channel closed by sending coroutine */
                done = 1;
                break;
            } else {
                if(errno != 0) {
                    perror("choose");
                    exit(EXIT_FAILURE);
                }
            }
            nfiles++;
            nbytes += size;
            break;
        case 1:
            printDirUsage(nfiles, nbytes);
            break;
        case -1:
            perror("choose");
            exit(EXIT_FAILURE);
        }
    }
    printDirUsage(nfiles, nbytes);
    exit(EXIT_SUCCESS);
}
