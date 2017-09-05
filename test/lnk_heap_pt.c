#include "bnm_heap.h"
#include "array.h"
#include "karn_pt.h"
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

struct lnk_heap_iface {
	char  *name;
	int  (*load)(const char *pathname);
	void (*insert)(unsigned long long *nsecs);
	void (*extract)(unsigned long long *nsecs);
};

struct lnk_bnm_heap_key {
	struct bnm_heap_node node;
	unsigned int         value;
};

static struct pt_entries        lnk_bnm_heap_entries;
static struct lnk_bnm_heap_key *lnk_bnm_heap_keys;

static int
lnk_bnm_heap_compare_min(const struct bnm_heap_node *first,
                         const struct bnm_heap_node *second)
{
	return ((struct lnk_bnm_heap_key *)first)->value -
	       ((struct lnk_bnm_heap_key *)second)->value;
}

static void
lnk_bnm_heap_insert_bulk(struct bnm_heap *heap)
{
	int                      n;
	struct lnk_bnm_heap_key *k;

	bnm_heap_init(heap);

	for (n = 0, k = lnk_bnm_heap_keys;
	     n < lnk_bnm_heap_entries.pt_nr;
	     n++, k++)
		bnm_heap_insert(heap, &k->node, lnk_bnm_heap_compare_min);
}

static int
lnk_bnm_heap_validate(void)
{
	struct bnm_heap          heap;
	struct lnk_bnm_heap_key *cur, *old;
	int                      n;

	lnk_bnm_heap_insert_bulk(&heap);

	old = bnm_heap_entry(bnm_heap_extract(&heap, lnk_bnm_heap_compare_min),
	                     struct lnk_bnm_heap_key, node);

	for (n = 1; n < lnk_bnm_heap_entries.pt_nr; n++) {
		cur = bnm_heap_entry(bnm_heap_extract(&heap,
		                                      lnk_bnm_heap_compare_min),
		                     struct lnk_bnm_heap_key, node);

		if (old->value > cur->value) {
			fprintf(stderr, "Bogus heap scheme\n");
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
lnk_bnm_heap_load(const char *pathname)
{
	struct lnk_bnm_heap_key *k;

	if (pt_open_entries(pathname, &lnk_bnm_heap_entries))
		return EXIT_FAILURE;

	lnk_bnm_heap_keys = malloc(lnk_bnm_heap_entries.pt_nr *
	                           sizeof(*lnk_bnm_heap_keys));
	if (!lnk_bnm_heap_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&lnk_bnm_heap_entries);

	k = lnk_bnm_heap_keys;
	while (!pt_iter_entry(&lnk_bnm_heap_entries, &k->value))
		k++;

	return lnk_bnm_heap_validate();
}

static void
lnk_bnm_heap_insert(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	struct bnm_heap heap;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	lnk_bnm_heap_insert_bulk(&heap);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

static void
lnk_bnm_heap_extract(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	struct bnm_heap heap;
	int             n;

	lnk_bnm_heap_insert_bulk(&heap);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	for (n = 0; n < lnk_bnm_heap_entries.pt_nr; n++)
		bnm_heap_extract(&heap, lnk_bnm_heap_compare_min);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);

	return;
}

static const struct lnk_heap_iface lnk_heap_algos[] = {
	{
		.name    = "dbnm",
		.load    = lnk_bnm_heap_load,
		.insert  = lnk_bnm_heap_insert,
		.extract = lnk_bnm_heap_extract,
	},
};

static const struct lnk_heap_iface *
lnk_setup_heap_algo(const char *algo_name)
{
	unsigned int a;

	for (a = 0; a < array_nr(lnk_heap_algos); a++)
		if (!strcmp(algo_name, lnk_heap_algos[a].name))
			return &lnk_heap_algos[a];

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
	const struct lnk_heap_iface *algo;
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

	algo = lnk_setup_heap_algo(argv[optind + 1]);
	if (!algo)
		return EXIT_FAILURE;

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (algo->load(argv[optind]))
		return EXIT_FAILURE;

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	for (l = 0; l < loops; l++) {
		algo->insert(&nsecs);
		printf("insert: nsec=%llu\n", nsecs);
	}

	for (l = 0; l < loops; l++) {
		algo->extract(&nsecs);
		printf("extract: nsec=%llu\n", nsecs);
	}

	return EXIT_SUCCESS;
}
