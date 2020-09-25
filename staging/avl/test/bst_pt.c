#include "perf.h"
#include <karn/avl.h>
#include <karn/pavl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

struct bstpt_iface {
	char  *bstpt_name;
	int  (*bstpt_load)(const char *pathname);
	void (*bstpt_append)(unsigned long long *nsecs);
	void (*bstpt_delete)(unsigned long long *nsecs);
	void (*bstpt_find)(unsigned long long *nsecs);
	void (*bstpt_forward)(unsigned long long *nsecs);
	void (*bstpt_backward)(unsigned long long *nsecs);
	void (*bstpt_clear)(unsigned long long *nsecs);
	void (*bstpt_bulk)(unsigned long long *nsecs);
};

static struct pt_entries bstpt_entries;

/******************************************************************************
 * Standard AVL tree
 ******************************************************************************/

#if defined(CONFIG_KARN_AVL)

struct bstpt_avl_key {
	struct avl_node node;
	unsigned int    value;
};

static struct bstpt_avl_key *avl_keys;
static int                   avl_count;

static int
bstpt_avl_compare_node_key(const struct avl_node *node,
                           const void            *key,
                           const void            *data __unused)
{
	return pt_compare_min((char *)&((struct bstpt_avl_key *)node)->value,
	                      (char *)key);
}

static int
bstpt_avl_compare_nodes(const struct avl_node *first,
                        const struct avl_node *second)
{
	return pt_compare_min((char *)&((struct bstpt_avl_key *)first)->value,
	                      (char *)&((struct bstpt_avl_key *)second)->value);
}

static void
bstpt_avl_release_key(struct avl_node *node __unused, void *data)
{
	int *cnt = (int *)data;

	*cnt = *cnt + 1;
}

static void
bstpt_avl_print_key(const struct avl_node *node)
{
	const struct bstpt_avl_key *key = (struct bstpt_avl_key *)node;

	printf("%#x(%d)", key->value, (int)node->balance);
}

static void
bstpt_avl_print_tree(const struct avl_tree *tree)
{
	avl_print_tree(tree, 16U, bstpt_avl_print_key);
}

static int
bstpt_avl_append_all(struct avl_tree *tree)
{
	int                   n;
	struct bstpt_avl_key *k;

	avl_count = 0;
	avl_init_tree(tree,
	              bstpt_avl_compare_node_key,
	              bstpt_avl_release_key,
	              &avl_count);

	for (n = 0, k = avl_keys; n < bstpt_entries.pt_nr; n++, k++)
		if (avl_append_node(tree, &k->node, &k->value))
			return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int
bstpt_avl_validate(void)
{
	struct avl_tree       tree;
	struct avl_iter       iter;
	int                   n;
	struct bstpt_avl_key *k;
	struct avl_node      *node;

	if (bstpt_avl_append_all(&tree)) {
		fprintf(stderr, "bogus AVL append scheme: insertion failed\n");
		return EXIT_FAILURE;
	}

	if (!avl_check_tree(&tree,
	                    bstpt_entries.pt_nr,
	                    bstpt_avl_compare_nodes)) {
		fprintf(stderr,
		        "bogus AVL append scheme: property violation\n");
		goto fail;
	}

	k = NULL;
	avl_walk_forward(&iter, &tree, node) {
		if (k && (((struct bstpt_avl_key *)node)->value <= k->value)) {
			fprintf(stderr, "bogus AVL forward iterator\n");
			goto fail;
		}

		k = (struct bstpt_avl_key *)node;
	}

	k = NULL;
	avl_walk_backward(&iter, &tree, node) {
		if (k && (((struct bstpt_avl_key *)node)->value >= k->value)) {
			fprintf(stderr, "bogus AVL backward iterator\n");
			goto fail;
		}

		k = (struct bstpt_avl_key *)node;
	}

	for (n = 0, k = avl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		if (avl_find_node(&tree, &k->value) != (struct avl_node *)k) {
			fprintf(stderr, "bogus AVL node finder\n");
			goto fail;
		}
	}

	for (n = 0, k = avl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		if (avl_delete_node(&tree, &k->value) != &k->node) {
			fprintf(stderr,
			        "bogus AVL delete scheme: deletion failed\n");
			goto fail;
		}

		if (!avl_check_tree(&tree,
		                    bstpt_entries.pt_nr - n - 1,
		                    bstpt_avl_compare_nodes)) {
			fprintf(stderr,
			        "bogus AVL delete scheme: "
			        "property violation\n");
			goto fail;
		}
	}

	bstpt_avl_append_all(&tree);
	avl_clear_tree(&tree);
	if (avl_count != bstpt_entries.pt_nr) {
		fprintf(stderr, "bogus AVL clear operation: skipped nodes\n");
		goto fail;
	}
	if (avl_tree_count(&tree) != 0) {
		fprintf(stderr,
		        "bogus AVL clear operation: invalid node count\n");
		goto fail;
	}

	return EXIT_SUCCESS;

fail:
	bstpt_avl_print_tree(&tree);
	return EXIT_FAILURE;
}

static int
bstpt_avl_load(const char *pathname)
{
	struct bstpt_avl_key *k;

	if (pt_open_entries(pathname, &bstpt_entries))
		return EXIT_FAILURE;

	avl_keys = malloc(bstpt_entries.pt_nr * sizeof(*avl_keys));
	if (!avl_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&bstpt_entries);

	k = avl_keys;
	while (!pt_iter_entry(&bstpt_entries, &k->value))
		k++;

	return bstpt_avl_validate();
}

static void
bstpt_avl_append(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	struct avl_tree tree;

	*nsecs = 0;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	bstpt_avl_append_all(&tree);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_avl_delete(unsigned long long *nsecs)
{
	int                   n;
	struct bstpt_avl_key *k;
	struct avl_tree       tree;
	struct timespec       start, elapse;

	*nsecs = 0;

	bstpt_avl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = avl_keys; n < bstpt_entries.pt_nr; n++, k++)
		avl_delete_node(&tree, &k->value);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_avl_find(unsigned long long *nsecs)
{
	int                   n;
	struct bstpt_avl_key *k;
	struct avl_tree       tree;
	struct timespec       start, elapse;

	*nsecs = 0;

	bstpt_avl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = avl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		avl_find_node(&tree, &k->value);
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_avl_forward(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;

	*nsecs = 0;

	bstpt_avl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	avl_walk_forward(&iter, &tree, node)
		;
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_avl_backward(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct avl_tree  tree;
	struct avl_iter  iter;
	struct avl_node *node;

	*nsecs = 0;

	bstpt_avl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	avl_walk_backward(&iter, &tree, node)
		;
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_avl_clear(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct avl_tree  tree;

	*nsecs = 0;

	bstpt_avl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	avl_clear_tree(&tree);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

#endif /* defined(CONFIG_KARN_AVL) */

/******************************************************************************
 * "Parented" AVL tree
 ******************************************************************************/

#if defined(CONFIG_KARN_PAVL)

struct bstpt_pavl_key {
	struct pavl_node node;
	unsigned int     value;
};

static struct bstpt_pavl_key *pavl_keys;
static struct bstpt_pavl_key *pavl_sorted_keys;
static int                    pavl_count;

static int
bstpt_pavl_compare_node_key(const struct pavl_node *node,
                            const void             *key,
                            const void             *data __unused)
{
	return pt_compare_min((char *)&((struct bstpt_pavl_key *)node)->value,
	                      (char *)key);
}

static int
bstpt_pavl_compare_nodes(const struct pavl_node *first,
                         const struct pavl_node *second)
{
	return pt_compare_min((char *)&((struct bstpt_pavl_key *)first)->value,
	                      (char *)&((struct bstpt_pavl_key *)second)->value);
}

static void
bstpt_pavl_release_key(struct pavl_node *node __unused, void *data)
{
	int *cnt = (int *)data;

	*cnt = *cnt + 1;
}

static void
bstpt_pavl_print_key(const struct pavl_node *node)
{
	const struct bstpt_pavl_key *key = (struct bstpt_pavl_key *)node;

	printf("%#x(%d)", key->value, (int)node->balance);
}

static void
bstpt_pavl_print_tree(const struct pavl_tree *tree)
{
	pavl_print_tree(tree, 16U, bstpt_pavl_print_key);
}

static int
bstpt_pavl_append_all(struct pavl_tree *tree)
{
	int                    n;
	struct bstpt_pavl_key *k;

	pavl_count = 0;
	pavl_init_tree(tree,
	               bstpt_pavl_compare_node_key,
	               bstpt_pavl_release_key,
	               &pavl_count);

	for (n = 0, k = pavl_keys; n < bstpt_entries.pt_nr; n++, k++)
		if (pavl_append_node(tree, &k->node, &k->value))
			return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static struct pavl_node *
bstpt_pavl_get_node_byid(unsigned int index, const void *keys)
{
	struct bstpt_pavl_key *k = NULL;

	assert(index < bstpt_entries.pt_nr);

	k = &((struct bstpt_pavl_key *)keys)[index];

	return &k->node;
}

static void
bstpt_pavl_bulk_load(struct pavl_tree *tree)
{
	pavl_init_tree(tree,
	               bstpt_pavl_compare_node_key,
	               NULL,
	               NULL);

	pavl_load_tree_from_sorted(tree,
	                           pavl_sorted_keys,
	                           bstpt_entries.pt_nr,
	                           bstpt_pavl_get_node_byid);
}

static int
bstpt_pavl_validate(void)
{
	struct pavl_tree       tree;
	int                    n;
	struct bstpt_pavl_key *k;
	struct pavl_node      *node;

	if (bstpt_pavl_append_all(&tree)) {
		fprintf(stderr, "bogus PAVL append scheme: insertion failed\n");
		return EXIT_FAILURE;
	}

	if (!pavl_check_tree(&tree,
	                     bstpt_entries.pt_nr,
	                     bstpt_pavl_compare_nodes)) {
		fprintf(stderr,
		        "bogus PAVL append scheme: property violation\n");
		goto fail;
	}

	k = NULL;
	pavl_walk_forward_inorder(&tree, node) {
		if (k && (((struct bstpt_pavl_key *)node)->value <= k->value)) {
			fprintf(stderr, "bogus PAVL forward iterator\n");
			goto fail;
		}

		k = (struct bstpt_pavl_key *)node;
	}

	k = NULL;
	pavl_walk_backward_inorder(&tree, node) {
		if (k && (((struct bstpt_pavl_key *)node)->value >= k->value)) {
			fprintf(stderr, "bogus PAVL backward iterator\n");
			goto fail;
		}

		k = (struct bstpt_pavl_key *)node;
	}

	for (n = 0, k = pavl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		if (pavl_find_node(&tree, &k->value) != (struct pavl_node *)k) {
			fprintf(stderr, "bogus PAVL node finder\n");
			goto fail;
		}
	}

	for (n = 0, k = pavl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		pavl_delete_node(&tree, &k->node);

		if (!pavl_check_tree(&tree,
		                     bstpt_entries.pt_nr - n - 1,
		                     bstpt_pavl_compare_nodes)) {
			fprintf(stderr,
			        "bogus PAVL delete scheme: "
			        "property violation\n");
			goto fail;
		}
	}

	bstpt_pavl_append_all(&tree);
	pavl_clear_tree(&tree);
	if (pavl_count != bstpt_entries.pt_nr) {
		fprintf(stderr, "bogus PAVL clear operation: skipped nodes\n");
		goto fail;
	}
	if (pavl_tree_count(&tree) != 0) {
		fprintf(stderr,
		        "bogus PAVL clear operation: invalid node count\n");
		goto fail;
	}

	bstpt_pavl_bulk_load(&tree);
	if (!pavl_check_tree(&tree,
	                     bstpt_entries.pt_nr,
	                     bstpt_pavl_compare_nodes)) {
		fprintf(stderr,
			"bogus PAVL bulk load scheme: "
			"property violation\n");
		goto fail;
	}

	return EXIT_SUCCESS;

fail:
	bstpt_pavl_print_tree(&tree);
	return EXIT_FAILURE;
}

static inline int
bstpt_pavl_qsort_compare(const void *a, const void *b)
{
	return pt_compare_min((char *)&((struct bstpt_pavl_key *)a)->value,
	                      (char *)&((struct bstpt_pavl_key *)b)->value);
}

static int
bstpt_pavl_load(const char *pathname)
{
	struct bstpt_pavl_key *k;
	size_t                 sz;

	if (pt_open_entries(pathname, &bstpt_entries))
		return EXIT_FAILURE;

	sz = bstpt_entries.pt_nr * sizeof(*pavl_keys);

	pavl_keys = malloc(sz);
	if (!pavl_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&bstpt_entries);

	k = pavl_keys;
	while (!pt_iter_entry(&bstpt_entries, &k->value))
		k++;

	pavl_sorted_keys = malloc(sz);
	if (!pavl_sorted_keys)
		return EXIT_FAILURE;

	memcpy(pavl_sorted_keys, pavl_keys, sz);

	qsort(pavl_sorted_keys,
	      bstpt_entries.pt_nr,
	      sizeof(*pavl_sorted_keys),
	      bstpt_pavl_qsort_compare);

	return bstpt_pavl_validate();
}

static void
bstpt_pavl_append(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct pavl_tree tree;

	*nsecs = 0;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	bstpt_pavl_append_all(&tree);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_delete(unsigned long long *nsecs)
{
	int                    n;
	struct bstpt_pavl_key *k;
	struct pavl_tree       tree;
	struct timespec        start, elapse;

	*nsecs = 0;

	bstpt_pavl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = pavl_keys; n < bstpt_entries.pt_nr; n++, k++)
		pavl_delete_node(&tree, &k->node);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_find(unsigned long long *nsecs)
{
	int                    n;
	struct bstpt_pavl_key *k;
	struct pavl_tree       tree;
	struct timespec        start, elapse;

	*nsecs = 0;

	bstpt_pavl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = pavl_keys; n < bstpt_entries.pt_nr; n++, k++) {
		pavl_find_node(&tree, &k->value);
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_forward(unsigned long long *nsecs)
{
	struct timespec   start, elapse;
	struct pavl_tree  tree;
	struct pavl_node *node;

	*nsecs = 0;

	bstpt_pavl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	pavl_walk_forward_inorder(&tree, node)
		;
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_backward(unsigned long long *nsecs)
{
	struct timespec   start, elapse;
	struct pavl_tree  tree;
	struct pavl_node *node;

	*nsecs = 0;

	bstpt_pavl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	pavl_walk_backward_inorder(&tree, node)
		;
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_clear(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct pavl_tree tree;

	*nsecs = 0;

	bstpt_pavl_append_all(&tree);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	pavl_clear_tree(&tree);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
bstpt_pavl_bulk(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct pavl_tree tree;

	*nsecs = 0;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	bstpt_pavl_bulk_load(&tree);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

#endif /* defined(CONFIG_KARN_PAVL) */

/******************************************************************************
 * Main measurment task handling
 ******************************************************************************/

static const struct bstpt_iface bstpt_algos[] = {
#if defined(CONFIG_KARN_AVL)
	{
		.bstpt_name     = "avl",
		.bstpt_load     = bstpt_avl_load,
		.bstpt_append   = bstpt_avl_append,
		.bstpt_delete   = bstpt_avl_delete,
		.bstpt_find     = bstpt_avl_find,
		.bstpt_forward  = bstpt_avl_forward,
		.bstpt_backward = bstpt_avl_backward,
		.bstpt_clear    = bstpt_avl_clear
	},
#endif
#if defined(CONFIG_KARN_PAVL)
	{
		.bstpt_name     = "pavl",
		.bstpt_load     = bstpt_pavl_load,
		.bstpt_append   = bstpt_pavl_append,
		.bstpt_delete   = bstpt_pavl_delete,
		.bstpt_find     = bstpt_pavl_find,
		.bstpt_forward  = bstpt_pavl_forward,
		.bstpt_backward = bstpt_pavl_backward,
		.bstpt_clear    = bstpt_pavl_clear,
		.bstpt_bulk     = bstpt_pavl_bulk,
	},
#endif
};

static const struct bstpt_iface *
bstpt_setup_algo(const char *algo_name)
{
	unsigned int a;

	for (a = 0; a < array_nr(bstpt_algos); a++)
		if (!strcmp(algo_name, bstpt_algos[a].bstpt_name))
			return &bstpt_algos[a];

	fprintf(stderr,
		"Invalid \"%s\" binary search tree algorithm\n",
	        algo_name);

	return NULL;
}

static int
pt_parse_scheme(const char               *arg,
                const char              **scheme,
                const struct bstpt_iface  *algo)
{
	if (!strcmp(arg, "append")) {
		if (!algo->bstpt_append)
			goto inval;
	}
	else if (!strcmp(arg, "delete")) {
		if (!algo->bstpt_delete)
			goto inval;
	}
	else if (!strcmp(arg, "find")) {
		if (!algo->bstpt_find)
			goto inval;
	}
	else if (!strcmp(arg, "forward")) {
		if (!algo->bstpt_forward)
			goto inval;
	}
	else if (!strcmp(arg, "backward")) {
		if (!algo->bstpt_backward)
			goto inval;
	}
	else if (!strcmp(arg, "clear")) {
		if (!algo->bstpt_clear)
			goto inval;
	}
	else if (!strcmp(arg, "bulk")) {
		if (!algo->bstpt_bulk)
			goto inval;
	}
	else {
		fprintf(stderr,
		        "Unknown \"%s\" binary search tree scheme\n",
		        arg);
		return EXIT_FAILURE;
	}

	*scheme = arg;

	return EXIT_SUCCESS;

inval:
	fprintf(stderr, "Invalid \"%s\" binary search tree scheme\n", arg);
	return EXIT_FAILURE;
}

static void
usage(const char *me)
{
	fprintf(stderr,
	        "Usage: %s [OPTIONS] FILE ALGORITHM LOOPS [SCHEME]\n"
	        "where OPTIONS:\n"
	        "    -p|--prio  PRIORITY\n"
	        "    -h|--help\n",
	        me);
}

int
main(int argc, char *argv[])
{
	const struct bstpt_iface *algo;
	unsigned int              l, loops = 0;
	int                       prio = 0;
	const char               *scheme = "";
	unsigned long long        nsecs;

	while (true) {
		int                        opt;
		static const struct option lopts[] = {
			{"help",    0, NULL, 'h'},
			{"prio",    1, NULL, 'p'},
			{0,         0, 0,    0}
		};

		opt = getopt_long(argc, argv, "hp:", lopts, NULL);
		if (opt < 0)
			/* No more options: go parsing positional arguments. */
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
	if (argc < 3) {
		fprintf(stderr, "Invalid number of arguments\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	algo = bstpt_setup_algo(argv[optind + 1]);
	if (!algo)
		return EXIT_FAILURE;

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (argc == 4) {
		if (pt_parse_scheme(argv[optind + 3], &scheme, algo))
			return EXIT_FAILURE;
	}

	if (algo->bstpt_load(argv[optind]))
		return EXIT_FAILURE;

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	if ((!*scheme && algo->bstpt_append) || !strcmp(scheme, "append")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_append(&nsecs);
			printf("append: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_delete) || !strcmp(scheme, "delete")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_delete(&nsecs);
			printf("delete: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_find) || !strcmp(scheme, "find")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_find(&nsecs);
			printf("find: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_forward) || !strcmp(scheme, "forward")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_forward(&nsecs);
			printf("forward: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_backward) || !strcmp(scheme, "backward")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_backward(&nsecs);
			printf("backward: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_clear) || !strcmp(scheme, "clear")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_clear(&nsecs);
			printf("clear: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->bstpt_bulk) || !strcmp(scheme, "bulk")) {
		for (l = 0; l < loops; l++) {
			algo->bstpt_bulk(&nsecs);
			printf("bulk load: nsec=%llu\n", nsecs);
		}
	}

	return EXIT_SUCCESS;
}
