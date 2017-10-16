#include "karn_pt.h"
#include "array.h"
#include "fbnr_heap.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

struct fapt_iface {
	char  *fapt_name;
	int  (*fapt_validate)(void);
	int  (*fapt_sort)(unsigned long long *nsecs);
};

static struct pt_entries  fapt_entries;
static unsigned int      *fapt_keys;

static int
fapt_compare_min(const char *a, const char *b)
{
	return *(unsigned int *)a - *(unsigned int *)b;
}

static int
fapt_qsort_compare_min(const void *a, const void *b)
{
	return *(unsigned int *)a - *(unsigned int *)b;
}

static int
fapt_compare_max(const char *a, const char *b)
{
	return 0 - fapt_compare_min(a, b);
}

static void
copy(char *restrict dst, const char *restrict src)
{
	*(unsigned int *)dst = *(unsigned int *)src;
}

/******************************************************************************
 * Quick sorting
 ******************************************************************************/

static int fapt_quick_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	qsort(keys, fapt_entries.pt_nr, sizeof(unsigned int),
	      fapt_qsort_compare_min);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (fapt_compare_min((char *)&keys[n - 1],
		                     (char *)&keys[n]) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_quick_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	qsort(keys, fapt_entries.pt_nr, sizeof(unsigned int),
	      fapt_qsort_compare_min);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

/******************************************************************************
 * Fixed array based binary heap
 ******************************************************************************/

static int fapt_fbnr_heap_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	fbnr_heap_sort((char *)keys, sizeof(unsigned int), fapt_entries.pt_nr,
	               fapt_compare_max, copy);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (fapt_compare_min((char *)&keys[n - 1],
		                     (char *)&keys[n]) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_fbnr_heap_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	fbnr_heap_sort((char *)keys, sizeof(unsigned int), fapt_entries.pt_nr,
	               fapt_compare_max, copy);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

/******************************************************************************
 * Main measurment task handling
 ******************************************************************************/

static const struct fapt_iface fapt_algos[] = {
	{
		.fapt_name     = "quick",
		.fapt_validate = fapt_quick_validate,
		.fapt_sort     = fapt_quick_sort
	},
#if defined(CONFIG_FBNR_HEAP_SORT)
	{
		.fapt_name     = "fbnrh",
		.fapt_validate = fapt_fbnr_heap_validate,
		.fapt_sort     = fapt_fbnr_heap_sort
	},
#endif
};

static int fapt_load(const char *pathname)
{
	unsigned int *k;

	if (pt_open_entries(pathname, &fapt_entries))
		return EXIT_FAILURE;

	fapt_keys = malloc(sizeof(*k) * fapt_entries.pt_nr);
	if (!fapt_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&fapt_entries);

	k = fapt_keys;
	while (!pt_iter_entry(&fapt_entries, k))
		k++;

	return EXIT_SUCCESS;
}

static const struct fapt_iface *
fapt_setup_algo(const char *algo_name)
{
	unsigned int a;

	for (a = 0; a < array_nr(fapt_algos); a++)
		if (!strcmp(algo_name, fapt_algos[a].fapt_name))
			return &fapt_algos[a];

	fprintf(stderr, "Invalid \"%s\" sort algorithm\n", algo_name);

	return NULL;
}

static void
usage(const char *me)
{
	fprintf(stderr,
	        "Usage: %s [OPTIONS] FILE ALGORITHM LOOPS\n"
	        "where OPTIONS:\n"
	        "    -p|--prio  PRIORITY\n"
	        "    -h|--help\n",
	        me);
}

int main(int argc, char *argv[])
{
	const struct fapt_iface *algo;
	unsigned int             l, loops = 0;
	int                      prio = 0;
	unsigned long long       nsecs;




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
	if (argc != 3) {
		fprintf(stderr, "Invalid number of arguments\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	algo = fapt_setup_algo(argv[optind + 1]);
	if (!algo)
		return EXIT_FAILURE;

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (fapt_load(argv[optind]))
		return EXIT_FAILURE;

	if (algo->fapt_validate())
		return EXIT_FAILURE;

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	for (l = 0; l < loops; l++) {
		if (algo->fapt_sort(&nsecs))
			return EXIT_FAILURE;
		printf("nsec=%llu\n", nsecs);
	}

	return EXIT_SUCCESS;
}
