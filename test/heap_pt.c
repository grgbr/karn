#include "bheap.h"
#include "sbnm_heap.h"
#include "dbnm_heap.h"
#include "array.h"
#include "karn_pt.h"
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

struct hppt_iface {
	char  *hppt_name;
	int  (*hppt_load)(const char *pathname);
	void (*hppt_insert)(unsigned long long *nsecs);
	void (*hppt_extract)(unsigned long long *nsecs);
};

static struct pt_entries hppt_entries;

/******************************************************************************
 * Fixed array based binomial heap
 ******************************************************************************/

#if defined(CONFIG_BHEAP_FIXED)

static unsigned int       *hppt_bnr_keys;
static struct bheap_fixed *hppt_bnr_heap;

static void hppt_bnr_copy(char       *restrict dest,
                          const char *restrict src)
{
	*((unsigned int *)dest) = *((unsigned int *)src);
}

static int hppt_bnr_compare_min(const char *restrict first,
                                const char *restrict second)
{
	return *((unsigned int *)first) - *((unsigned int *)second);
}

static void
hppt_bnr_insert_bulk(void)
{
	unsigned int *k;
	int           n;

	bheap_clear_fixed(hppt_bnr_heap);

	for (n = 0, k = hppt_bnr_keys; n < hppt_entries.pt_nr; n++, k++)
		bheap_insert_fixed(hppt_bnr_heap, (char *)k, hppt_bnr_copy,
		                   hppt_bnr_compare_min);
}

static int
hppt_bnr_validate(void)
{
	unsigned int cur, old;
	int          n;

	hppt_bnr_heap = bheap_create_fixed(sizeof(cur), hppt_entries.pt_nr);
	if (!hppt_bnr_heap)
		return EXIT_FAILURE;

	hppt_bnr_insert_bulk();

	bheap_extract_fixed(hppt_bnr_heap, (char *)&old, hppt_bnr_copy,
	                    hppt_bnr_compare_min);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		bheap_extract_fixed(hppt_bnr_heap, (char *)&cur, hppt_bnr_copy,
		                    hppt_bnr_compare_min);

		if (old > cur) {
			fprintf(stderr, "Bogus heap scheme\n");
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_bnr_load(const char *pathname)
{
	unsigned int *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	hppt_bnr_keys = malloc(sizeof(*k) * hppt_entries.pt_nr);
	if (!hppt_bnr_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = hppt_bnr_keys;
	while (!pt_iter_entry(&hppt_entries, k))
		k++;

	return hppt_bnr_validate();
}

static void
hppt_bnr_insert(unsigned long long *nsecs)
{
	struct timespec start, elapse;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	hppt_bnr_insert_bulk();
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

static void
hppt_bnr_extract(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	unsigned int    cur;
	int             n;

	hppt_bnr_insert_bulk();

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		bheap_extract_fixed(hppt_bnr_heap, (char *)&cur, hppt_bnr_copy,
		                    hppt_bnr_compare_min);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

#endif /* defined(CONFIG_BHEAP_FIXED) */

/******************************************************************************
 * Singly linked list based binomial heap
 ******************************************************************************/

#if defined(CONFIG_SBNM_HEAP)

struct hppt_sbnm_key {
	struct sbnm_heap_node node;
	unsigned int          value;
};

static struct hppt_sbnm_key *sbnm_heap_keys;

static int
hppt_sbnm_compare_min(const struct sbnm_heap_node *first,
                      const struct sbnm_heap_node *second)
{
	return ((struct hppt_sbnm_key *)first)->value -
	       ((struct hppt_sbnm_key *)second)->value;
}

static void
hppt_sbnm_insert_bulk(struct sbnm_heap *heap)
{
	int                   n;
	struct hppt_sbnm_key *k;

	sbnm_heap_init(heap);

	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		sbnm_heap_insert(heap, &k->node, hppt_sbnm_compare_min);
}

static int
hppt_sbnm_validate(void)
{
	struct sbnm_heap      heap;
	struct hppt_sbnm_key *cur, *old;
	int                   n;

	hppt_sbnm_insert_bulk(&heap);

	old = sbnm_heap_entry(sbnm_heap_extract(&heap, hppt_sbnm_compare_min),
	                      struct hppt_sbnm_key, node);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = sbnm_heap_entry(sbnm_heap_extract(&heap,
		                                        hppt_sbnm_compare_min),
		                      struct hppt_sbnm_key, node);

		if (old->value > cur->value) {
			fprintf(stderr, "Bogus heap scheme\n");
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_sbnm_load(const char *pathname)
{
	struct hppt_sbnm_key *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	sbnm_heap_keys = malloc(hppt_entries.pt_nr * sizeof(*sbnm_heap_keys));
	if (!sbnm_heap_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = sbnm_heap_keys;
	while (!pt_iter_entry(&hppt_entries, &k->value))
		k++;

	return hppt_sbnm_validate();
}

static void
hppt_sbnm_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct sbnm_heap heap;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	hppt_sbnm_insert_bulk(&heap);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

static void
hppt_sbnm_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct sbnm_heap heap;
	int              n;

	hppt_sbnm_insert_bulk(&heap);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		sbnm_heap_extract(&heap, hppt_sbnm_compare_min);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

#endif /* defined(CONFIG_SBNM_HEAP) */

/******************************************************************************
 * Doubly linked list based binomial heap
 ******************************************************************************/

#if defined(CONFIG_DBNM_HEAP)

struct hppt_dbnm_key {
	struct dbnm_heap_node node;
	unsigned int          value;
};

static struct hppt_dbnm_key *dbnm_heap_keys;

static int
hppt_dbnm_compare_min(const struct dbnm_heap_node *first,
                      const struct dbnm_heap_node *second)
{
	return ((struct hppt_dbnm_key *)first)->value -
	       ((struct hppt_dbnm_key *)second)->value;
}

static void
hppt_dbnm_insert_bulk(struct dbnm_heap *heap)
{
	int                   n;
	struct hppt_dbnm_key *k;

	dbnm_heap_init(heap);

	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		dbnm_heap_insert(heap, &k->node, hppt_dbnm_compare_min);
}

static int
hppt_dbnm_validate(void)
{
	struct dbnm_heap      heap;
	struct hppt_dbnm_key *cur, *old;
	int                   n;

	hppt_dbnm_insert_bulk(&heap);

	old = dbnm_heap_entry(dbnm_heap_extract(&heap, hppt_dbnm_compare_min),
	                      struct hppt_dbnm_key, node);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = dbnm_heap_entry(dbnm_heap_extract(&heap,
		                                        hppt_dbnm_compare_min),
		                      struct hppt_dbnm_key, node);

		if (old->value > cur->value) {
			fprintf(stderr, "Bogus heap scheme\n");
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_dbnm_load(const char *pathname)
{
	struct hppt_dbnm_key *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	dbnm_heap_keys = malloc(hppt_entries.pt_nr * sizeof(*dbnm_heap_keys));
	if (!dbnm_heap_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = dbnm_heap_keys;
	while (!pt_iter_entry(&hppt_entries, &k->value))
		k++;

	return hppt_dbnm_validate();
}

static void
hppt_dbnm_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct dbnm_heap heap;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	hppt_dbnm_insert_bulk(&heap);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

static void
hppt_dbnm_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct dbnm_heap heap;
	int              n;

	hppt_dbnm_insert_bulk(&heap);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		dbnm_heap_extract(&heap, hppt_dbnm_compare_min);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

#endif /* defined(CONFIG_DBNM_HEAP) */

/******************************************************************************
 * Main measurment task handling
 ******************************************************************************/

static const struct hppt_iface hppt_algos[] = {
#if defined(CONFIG_BHEAP_FIXED)
	{
		.hppt_name    = "bnr",
		.hppt_load    = hppt_bnr_load,
		.hppt_insert  = hppt_bnr_insert,
		.hppt_extract = hppt_bnr_extract
	},
#endif
#if defined(CONFIG_SBNM_HEAP)
	{
		.hppt_name    = "sbnm",
		.hppt_load    = hppt_sbnm_load,
		.hppt_insert  = hppt_sbnm_insert,
		.hppt_extract = hppt_sbnm_extract
	},
#endif
#if defined(CONFIG_DBNM_HEAP)
	{
		.hppt_name    = "dbnm",
		.hppt_load    = hppt_dbnm_load,
		.hppt_insert  = hppt_dbnm_insert,
		.hppt_extract = hppt_dbnm_extract
	},
#endif
};

static const struct hppt_iface *
hppt_setup_algo(const char *algo_name)
{
	unsigned int a;

	for (a = 0; a < array_nr(hppt_algos); a++)
		if (!strcmp(algo_name, hppt_algos[a].hppt_name))
			return &hppt_algos[a];

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
	const struct hppt_iface *algo;
	unsigned int                 l, loops = 0;
	int                          prio = 0;
	unsigned long long           nsecs;

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

	algo = hppt_setup_algo(argv[optind + 1]);
	if (!algo)
		return EXIT_FAILURE;

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (algo->hppt_load(argv[optind]))
		return EXIT_FAILURE;

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	for (l = 0; l < loops; l++) {
		algo->hppt_insert(&nsecs);
		printf("insert: nsec=%llu\n", nsecs);
	}

	for (l = 0; l < loops; l++) {
		algo->hppt_extract(&nsecs);
		printf("extract: nsec=%llu\n", nsecs);
	}

	return EXIT_SUCCESS;
}
