#include "karn_pt.h"
#include <utils/cdefs.h>
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

/******************************************************************************
 * Glibc's quick sorting
 ******************************************************************************/

static int fapt_qsort_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	qsort(keys, fapt_entries.pt_nr, sizeof(unsigned int),
	      pt_qsort_compare);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_qsort_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	qsort(keys, fapt_entries.pt_nr, sizeof(unsigned int),
	      pt_qsort_compare);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

/******************************************************************************
 * Bubble sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FARR_BUBBLE_SORT)

#include "farr.h"

static int fapt_bubble_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	farr_bubble_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_bubble_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	farr_bubble_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FARR_BUBBLE_SORT) */

/******************************************************************************
 * Selection sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FARR_SELECTION_SORT)

#include "farr.h"

static int fapt_selection_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	farr_selection_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_selection_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	farr_selection_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FARR_SELECTION_SORT) */

/******************************************************************************
 * Insertion sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FARR_INSERTION_SORT)

#include "farr.h"

static int fapt_insertion_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	farr_insertion_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_insertion_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	farr_insertion_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FARR_INSERTION_SORT) */

/******************************************************************************
 * Quick sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FARR_QUICK_SORT)

#include "farr.h"

static int fapt_quick_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	farr_quick_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
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
	farr_quick_sort((char *)keys,  sizeof(*keys),
	                 fapt_entries.pt_nr, pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FARR_QUICK_SORT) */

/******************************************************************************
 * Fixed array based binary heap sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FBNR_HEAP_SORT)

#include "fbnr_heap.h"

static int fapt_fbnr_heap_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	fbnr_heap_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	               pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
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
	fbnr_heap_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	               pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FBNR_HEAP_SORT) */

/******************************************************************************
 * Fixed array based weak heap sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FWK_HEAP_SORT)

#include "fwk_heap.h"

static int fapt_fwk_heap_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	if (fwk_heap_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	                  pt_compare_min, pt_copy_key))
		goto free;

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_fwk_heap_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;
	int              ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	if (fwk_heap_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	                  pt_compare_min, pt_copy_key))
		goto free;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

#endif /* defined(CONFIG_KARN_FWK_HEAP_SORT) */

/******************************************************************************
 * Fixed array based introspective sorting
 ******************************************************************************/

#if defined(CONFIG_KARN_FARR_INTRO_SORT)

#include "farr.h"

static int fapt_intro_validate(void)
{
	int           n;
	unsigned int *keys;
	int           ret = EXIT_FAILURE;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	farr_intro_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	                pt_compare_min, pt_copy_key);

	for (n = 1; n < fapt_entries.pt_nr; n++) {
		if (keys[n - 1] > keys[n]) {
			fprintf(stderr, "Bogus sorting scheme\n");
			goto free;
		}
	}

	ret = EXIT_SUCCESS;

free:
	free(keys);

	return ret;
}

static int fapt_intro_sort(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *keys;

	keys = malloc(sizeof(*keys) * fapt_entries.pt_nr);
	if (!keys)
		return EXIT_FAILURE;

	memcpy(keys, fapt_keys, sizeof(*keys) * fapt_entries.pt_nr);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	farr_intro_sort((char *)keys, sizeof(*keys), fapt_entries.pt_nr,
	                pt_compare_min, pt_copy_key);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	free(keys);

	return EXIT_SUCCESS;
}

#endif /* defined(CONFIG_KARN_FARR_INTRO_SORT) */

/******************************************************************************
 * Main measurment task handling
 ******************************************************************************/

static const struct fapt_iface fapt_algos[] = {
	{
		.fapt_name     = "qsort",
		.fapt_validate = fapt_qsort_validate,
		.fapt_sort     = fapt_qsort_sort
	},
#if defined(CONFIG_KARN_FBNR_HEAP_SORT)
	{
		.fapt_name     = "fbnrh",
		.fapt_validate = fapt_fbnr_heap_validate,
		.fapt_sort     = fapt_fbnr_heap_sort
	},
#endif
#if defined(CONFIG_KARN_FWK_HEAP_SORT)
	{
		.fapt_name     = "fwkh",
		.fapt_validate = fapt_fwk_heap_validate,
		.fapt_sort     = fapt_fwk_heap_sort
	},
#endif
#if defined(CONFIG_KARN_FARR_BUBBLE_SORT)
	{
		.fapt_name     = "bubble",
		.fapt_validate = fapt_bubble_validate,
		.fapt_sort     = fapt_bubble_sort
	},
#endif
#if defined(CONFIG_KARN_FARR_SELECTION_SORT)
	{
		.fapt_name     = "selection",
		.fapt_validate = fapt_selection_validate,
		.fapt_sort     = fapt_selection_sort
	},
#endif
#if defined(CONFIG_KARN_FARR_INSERTION_SORT)
	{
		.fapt_name     = "insertion",
		.fapt_validate = fapt_insertion_validate,
		.fapt_sort     = fapt_insertion_sort
	},
#endif
#if defined(CONFIG_KARN_FARR_QUICK_SORT)
	{
		.fapt_name     = "quick",
		.fapt_validate = fapt_quick_validate,
		.fapt_sort     = fapt_quick_sort
	},
#endif
#if defined(CONFIG_KARN_FARR_INTRO_SORT)
	{
		.fapt_name     = "intro",
		.fapt_validate = fapt_intro_validate,
		.fapt_sort     = fapt_intro_sort
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
