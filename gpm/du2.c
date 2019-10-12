/* Compute disk usage of files in a dir. Adapted from The Go Programming
   Language book.
*/
#include <stdio.h>
#include <libdill.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include "ticker.h"

static void printDirUsage(int64_t nfiles, int64_t nbytes)
{
	printf("%ld files\t%.1f GB\n", nfiles, nbytes / 1e9);
}

// walkDir: recursivly walks directory reporting files sizes through ch
void walkDir(char *dirName, int ch)
{
	DIR *dir = opendir(dirName);
	if (dir == NULL) {
		perror("could not open directory");
		fprintf(stderr, "%s\n", dirName);
		return;
	}

	char entryName[256] = "";
	int rc;
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		// Skip current and parent dir entries
		if (strcmp(dp->d_name, ".") == 0
		    || strcmp(dp->d_name, "..") == 0)
			continue;

		rc = snprintf(entryName,
			      sizeof(entryName), "%s/%s", dirName,
			      dp->d_name);
		if (rc < 0) {
			fprintf(stderr,
				"Error building dir entry name.\n");
			break;
		}
		// Use lstat, not stat, so we don't follow symbolic links. 
		struct stat stats;
		rc = lstat(entryName, &stats);
		if (rc != 0) {
			perror("lstat");
			fprintf(stderr, "lstat(%s) failed (%d)\n",
				entryName, rc);
			break;
		}
		// Check if dir entry is a directory
		if (S_ISDIR(stats.st_mode)) {
			walkDir(entryName, ch);
		} else {
			rc = chsend(ch, &stats.st_size,
				    sizeof(stats.st_size), -1);
			if (rc < 0) {
				perror
				    ("Could not send size down channel.");
				break;
			}
		}
	}

	rc = closedir(dir);
	if (rc != 0) {
		perror("could not close directory");
	}
}

// walkDirStart: coroutine to walk each directory in dirs.
coroutine void walkDirStart(char *dirs[], int ndirs, int ch)
{
	for (int i = 0; i < ndirs; ++i) {
		walkDir(dirs[i], ch);
	}

	int rc = chdone(ch);
	if (rc != 0) {
		perror("chdone on size channel failed");
	}
}

char *curdir[] = { "." };

int main(int argc, char *argv[])
{
	int rc;
	int verbose = 0;
	int c;
	while ((c = getopt(argc, argv, "v")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		}
	}

	/* Default to current directory if no directories provided */
	char **roots = curdir;
	int ndirs = 1;
	if (optind < argc) {
		roots = &argv[optind];
		ndirs = argc - optind;
	}

	/* Channel to report files sizes */
	int filesz_ch[2];
	rc = chmake(filesz_ch);
	if (rc != 0) {
		perror("could not make size channel");
		exit(EXIT_FAILURE);
	}

	rc = go(walkDirStart(roots, ndirs, filesz_ch[1]));
	if (rc < 0) {
		perror("could not start walkDirStart");
		exit(EXIT_FAILURE);
	}

	/* Start ticker to periodically display results */
	int ticker_ch = TickEvery(verbose ? 500 : 0);
	if (ticker_ch < 0) {
		perror("time_Tick");
		exit(EXIT_FAILURE);
	}

	int64_t nfiles = 0, nbytes = 0;

	int64_t size = 0;
	int64_t tick = 0;
	struct chclause clauses[] = {
		{CHRECV, filesz_ch[0], &size, sizeof(size)}
		,
		{CHRECV, ticker_ch, &tick, sizeof(tick)}
	};

	int done = 0;
	while (!done) {
		rc = choose(clauses, 2, -1);
		switch (rc) {
		case 0:
			if (errno == EPIPE) {
				/* Channel closed by sending coroutine */
				done = 1;
				break;
			} else {
				if (errno != 0) {
					perror("choose");
					exit(EXIT_FAILURE);
				}
			}
			nfiles++;
			nbytes += size;
			break;
		case 1:
			if (verbose) {
				printDirUsage(nfiles, nbytes);
			}
			break;
		case -1:
			perror("choose");
			exit(EXIT_FAILURE);
		}
	}

	printDirUsage(nfiles, nbytes);
	exit(EXIT_SUCCESS);
}
