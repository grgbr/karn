#include "karn_pt.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

typedef void (sort_fn)(void    *base,
                       size_t   nmemb,
                       size_t   size,
                       int    (*compar)(const void *, const void *));

static int
compare(const void *a, const void *b)
{
	return *(unsigned int *)a - *(unsigned int *)b;
}

static unsigned long long
account_sort(const struct pt_entries *entries,
             unsigned int            *keys,
             sort_fn                 *sort)
{
	struct timespec start, elapse;
	unsigned int    k;

	pt_init_entry_iter(entries);

	k = 0;
	while (!pt_iter_entry(entries, &keys[k]))
		k++;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	sort(keys, entries->pt_nr, sizeof(keys[0]), compare);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);

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
	unsigned int       loops = 0;
	struct pt_entries  entries;
	unsigned int      *keys;
	unsigned int       k;
	int                prio = 0;
	sort_fn           *sort;
	long long          nsec;

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
			if (pt_parse_sched_prio(optarg, &prio)) {
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

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (pt_open_entries(argv[optind], &entries))
		return EXIT_FAILURE;

	keys = malloc(entries.pt_nr * sizeof(keys[0]));
	if (!keys)
		return EXIT_FAILURE;

	account_sort(&entries, keys, sort);

	for (k = 0; k < (unsigned int)(entries.pt_nr - 1); k++) {
		if (compare(&keys[k], &keys[k + 1]) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			return EXIT_FAILURE;
		}
	}

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	while (loops--) {
		nsec = account_sort(&entries, keys, sort);

		printf("nsec: %llu\n", nsec);
	}

	return EXIT_SUCCESS;
}
