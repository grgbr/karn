#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

typedef void (sort_fn)(void    *base,
                       size_t   nmemb,
                       size_t   size,
                       int    (*compar)(const void *, const void *));

static struct timespec
tspec_sub(const struct timespec *restrict a, const struct timespec *restrict b)
{
	struct timespec res = {
		.tv_sec  = a->tv_sec - b->tv_sec,
		.tv_nsec = a->tv_nsec - b->tv_nsec
	};

	if (res.tv_nsec < 0) {
		res.tv_sec--;
		res.tv_nsec += 1000000000;
	}

	return res;
}

static int
compare(const void *a, const void *b)
{
	return *(unsigned int *)a - *(unsigned int *)b;
}

static unsigned long long
account_sort(FILE *file, unsigned int *keys, sort_fn *sort, unsigned int key_nr)
{
	struct timespec start, elapse;
	unsigned int    k;

	rewind(file);

	k = 0;
	while (fread(&keys[k], sizeof(keys[0]), 1, file) == 1)
		k++;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	sort(keys, key_nr, sizeof(keys[0]), compare);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = tspec_sub(&elapse, &start);

	return ((long long)elapse.tv_sec * 1000000000LL) +
	       (long long)elapse.tv_nsec;
}

static void
usage(const char *me)
{
	fprintf(stderr,
	        "Usage: %s [-h|--help] [-p|--prio PRIORITY] FILE ALGORITHM LOOPS\n",
	        me);
}

int main(int argc, char *argv[])
{
	FILE               *file;
	int                 nr;
	char               *errstr;
	unsigned int        loops = 0;
	unsigned int       *keys;
	unsigned int        k;
	struct sched_param  parm = { 0, };
	sort_fn            *sort;
	long long           nsec;

	while (true) {
		int                        opt;
		static const struct option lopts[] = {
			{"help",    0, NULL, 'h'},
			{"prio",    1, NULL, 'p'},
			{0,         0, 0,    0}
		};

		opt = getopt_long(argc, argv, "hp:", lopts, NULL);
		if (opt < 0)
			/* No more options: go parsing positional arguments. */
			break;

		switch (opt) {
		case 'p': /* priority */
			parm.sched_priority = (int)strtoul(optarg, &errstr, 0);
			if (*errstr) {
				fprintf(stderr, "Invalid priority\n");
				usage(argv[0]);
				return EXIT_FAILURE;
			}

			break;

		case 'h': /* Help message. */
			usage(argv[0]);
			return EXIT_SUCCESS;

		case '?': /* Unknown option. */
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/*
	 * Check positional arguments are properly specified on command
	 * line.
	 */
	argc -= optind;

	switch (argc) {
	case 3:
		break;

	default:
		fprintf(stderr, "Missing argument\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (!strcmp(argv[optind + 1], "qsort"))
		sort = qsort;
	else {
		fprintf(stderr, "Invalid sort algorithm \"%s\"\n",
		        argv[optind + 1]);
		return EXIT_FAILURE;
	}

	loops = strtoul(argv[optind + 2], &errstr, 0);
	if (*errstr)
		loops = 0;
	if (!loops) {
		fprintf(stderr, "Invalid number of loops\n");
		return EXIT_FAILURE;
	}

	file = fopen(argv[optind], "r");
	if (!file) {
		fprintf(stderr, "Failed to open file %s: %s\n", argv[1],
		        strerror(errno));
		return EXIT_FAILURE;
	}

	if (fseek(file, 0, SEEK_END)) {
		perror("Failed to probe file end");
		return EXIT_FAILURE;
	}

	nr = ftell(file);
	if (nr < 0) {
		perror("Failed to probe file size");
		return EXIT_FAILURE;
	}

	nr /= (int)sizeof(uint32_t);
	if (nr <= 0) {
		fprintf(stderr, "Invalid file content\n");
		return EXIT_FAILURE;
	}

	keys = malloc(nr * sizeof(keys[0]));
	if (!keys)
		return EXIT_FAILURE;

	account_sort(file, keys, sort, nr);

	for (k = 0; k < (unsigned int)(nr - 1); k++) {
		if (compare(&keys[k], &keys[k + 1]) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			return EXIT_FAILURE;
		}
	}

	if (parm.sched_priority) {
		if (sched_setscheduler(getpid(), SCHED_FIFO, &parm)) {
			perror("Failed set scheduling policy");
			return EXIT_FAILURE;
		}
	}

	while (loops--) {
		nsec = account_sort(file, keys, sort, nr);

		printf("nsec: %llu\n", nsec);
	}

	return EXIT_SUCCESS;
}
