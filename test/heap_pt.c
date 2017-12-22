#include "fbnr_heap.h"
#include "fwk_heap.h"
#include "sbnm_heap.h"
#include "dbnm_heap.h"
#include "falloc.h"
#include "pbnm_heap.h"
#include "spair_heap.h"
#include "karn_pt.h"
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

struct hppt_iface {
	char  *hppt_name;
	int  (*hppt_load)(const char *pathname);
	//void (*hppt_peek)(unsigned long long *nsecs);
	void (*hppt_insert)(unsigned long long *nsecs);
	void (*hppt_extract)(unsigned long long *nsecs);
	void (*hppt_remove)(unsigned long long *nsecs);
	void (*hppt_promote)(unsigned long long *nsecs);
	void (*hppt_demote)(unsigned long long *nsecs);
	//void (*hppt_merge)(unsigned long long *nsecs);
	void (*hppt_build)(unsigned long long *nsecs);
};

static struct pt_entries hppt_entries;

/******************************************************************************
 * Fixed array based binomial heap
 ******************************************************************************/

#if defined(CONFIG_FBNR_HEAP)

static unsigned int     *hppt_fbnr_keys;
static struct fbnr_heap *hppt_fbnr_heap;

static void
hppt_fbnr_insert_bulk(void)
{
	unsigned int *k;
	int           n;

	fbnr_heap_clear(hppt_fbnr_heap);

	for (n = 0, k = hppt_fbnr_keys; n < hppt_entries.pt_nr; n++, k++)
		fbnr_heap_insert(hppt_fbnr_heap, (char *)k);
}

static int
hppt_fbnr_check_entries(const char *scheme)
{
	unsigned int cur, old;
	int          n;

	fbnr_heap_extract(hppt_fbnr_heap, (char *)&old);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		fbnr_heap_extract(hppt_fbnr_heap, (char *)&cur);

		if (old > cur) {
			fprintf(stderr, "Bogus heap %s scheme\n", scheme);
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_fbnr_validate(void)
{
	hppt_fbnr_heap = fbnr_heap_create(sizeof(*hppt_fbnr_keys),
	                                  hppt_entries.pt_nr,
	                                  pt_compare_min, pt_copy_key);
	if (!hppt_fbnr_heap)
		return EXIT_FAILURE;

	hppt_fbnr_insert_bulk();
	if (hppt_fbnr_check_entries("insert/extract"))
		return EXIT_FAILURE;

	memcpy(hppt_fbnr_heap->fbnr_tree.fabs_nodes.farr_slots,
	       hppt_fbnr_keys,
	       sizeof(*hppt_fbnr_keys * hppt_entries.pt_nr));
	fbnr_heap_build(hppt_fbnr_heap, hppt_entries.pt_nr);
	return hppt_fbnr_check_entries("build");
}

static int
hppt_fbnr_load(const char *pathname)
{
	unsigned int *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	hppt_fbnr_keys = malloc(sizeof(*k) * hppt_entries.pt_nr);
	if (!hppt_fbnr_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = hppt_fbnr_keys;
	while (!pt_iter_entry(&hppt_entries, k))
		k++;

	return hppt_fbnr_validate();
}

static void
hppt_fbnr_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *k;
	int              n;

	fbnr_heap_clear(hppt_fbnr_heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = hppt_fbnr_keys; n < hppt_entries.pt_nr; n++, k++)
		fbnr_heap_insert(hppt_fbnr_heap, (char *)k);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_fbnr_extract(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	unsigned int    cur;
	int             n;

	hppt_fbnr_insert_bulk();

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		fbnr_heap_extract(hppt_fbnr_heap, (char *)&cur);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_fbnr_build(unsigned long long *nsecs)
{
	struct timespec  start, elapse;

	memcpy(hppt_fbnr_heap->fbnr_tree.fabs_nodes.farr_slots,
	       hppt_fbnr_keys,
	       sizeof(*hppt_fbnr_keys * hppt_entries.pt_nr));

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	fbnr_heap_build(hppt_fbnr_heap, hppt_entries.pt_nr);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

#endif /* defined(CONFIG_FBNR_HEAP) */

/******************************************************************************
 * Fixed array based weak heap
 ******************************************************************************/

#if defined(CONFIG_FWK_HEAP)

static unsigned int    *hppt_fwk_keys;
static struct fwk_heap *hppt_fwk_heap;

static void
hppt_fwk_insert_bulk(void)
{
	unsigned int *k;
	int           n;

	fwk_heap_clear(hppt_fwk_heap);

	for (n = 0, k = hppt_fwk_keys; n < hppt_entries.pt_nr; n++, k++)
		fwk_heap_insert(hppt_fwk_heap, (char *)k);
}

static int
hppt_fwk_check_entries(const char *scheme)
{
	unsigned int cur, old;
	int          n;

	fwk_heap_extract(hppt_fwk_heap, (char *)&old);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		fwk_heap_extract(hppt_fwk_heap, (char *)&cur);

		if (old > cur) {
			fprintf(stderr, "Bogus heap %s scheme\n", scheme);
			return EXIT_FAILURE;
		}

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_fwk_validate(void)
{
	hppt_fwk_heap = fwk_heap_create(sizeof(*hppt_fwk_keys),
	                                hppt_entries.pt_nr,
	                                pt_compare_min, pt_copy_key);
	if (!hppt_fwk_heap)
		return EXIT_FAILURE;

	hppt_fwk_insert_bulk();
	if (hppt_fwk_check_entries("insert/extract"))
		return EXIT_FAILURE;

	memcpy(hppt_fwk_heap->fwk_nodes.farr_slots,
	       hppt_fwk_keys,
	       sizeof(*hppt_fwk_keys * hppt_entries.pt_nr));
	fwk_heap_build(hppt_fwk_heap, hppt_entries.pt_nr);

	return hppt_fwk_check_entries("build");
}

static int
hppt_fwk_load(const char *pathname)
{
	unsigned int *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	hppt_fwk_keys = malloc(sizeof(*k) * hppt_entries.pt_nr);
	if (!hppt_fwk_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = hppt_fwk_keys;
	while (!pt_iter_entry(&hppt_entries, k))
		k++;

	return hppt_fwk_validate();
}

static void
hppt_fwk_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	unsigned int    *k;
	int              n;

	fwk_heap_clear(hppt_fwk_heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0, k = hppt_fwk_keys; n < hppt_entries.pt_nr; n++, k++)
		fwk_heap_insert(hppt_fwk_heap, (char *)k);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_fwk_extract(unsigned long long *nsecs)
{
	struct timespec start, elapse;
	unsigned int    cur;
	int             n;

	hppt_fwk_insert_bulk();

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		fwk_heap_extract(hppt_fwk_heap, (char *)&cur);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_fwk_build(unsigned long long *nsecs)
{
	struct timespec  start, elapse;

	memcpy(hppt_fwk_heap->fwk_nodes.farr_slots,
	       hppt_fwk_keys,
	       sizeof(*hppt_fwk_keys * hppt_entries.pt_nr));

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	fwk_heap_build(hppt_fwk_heap, hppt_entries.pt_nr);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

#endif /* defined(CONFIG_FWK_HEAP) */

/******************************************************************************
 * Singly linked list based binomial heap
 ******************************************************************************/

#if defined(CONFIG_SBNM_HEAP)

struct hppt_sbnm_key {
	struct sbnm_heap_node node;
	unsigned int          value;
};

static struct hppt_sbnm_key *sbnm_heap_keys;
static unsigned int          sbnm_heap_min;

static int
hppt_sbnm_compare_min(const struct sbnm_heap_node *first,
                      const struct sbnm_heap_node *second)
{
	return pt_compare_min((char *)&((struct hppt_sbnm_key *)first)->value,
	                      (char *)&((struct hppt_sbnm_key *)second)->value);
}

static void
hppt_sbnm_insert_bulk(struct sbnm_heap *heap)
{
	int                   n;
	struct hppt_sbnm_key *k;

	sbnm_heap_init(heap, hppt_sbnm_compare_min);

	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		sbnm_heap_insert(heap, &k->node);
}

static int
hppt_sbnm_check_heap(struct sbnm_heap *heap)
{
	int                   n;
	struct hppt_sbnm_key *cur, *old;

	old = sbnm_heap_entry(sbnm_heap_extract(heap),
	                      struct hppt_sbnm_key, node);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = sbnm_heap_entry(sbnm_heap_extract(heap),
		                      struct hppt_sbnm_key, node);

		if (old->value > cur->value)
			return EXIT_FAILURE;

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_sbnm_validate(void)
{
	struct sbnm_heap      heap;
	int                   n;
	struct hppt_sbnm_key *k;

	hppt_sbnm_insert_bulk(&heap);

	if (hppt_sbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap insert / extract scheme\n");
		return EXIT_FAILURE;
	}

	hppt_sbnm_insert_bulk(&heap);
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= sbnm_heap_min;
		sbnm_heap_promote(&heap, &k->node);
	}
	if (hppt_sbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap promote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += sbnm_heap_min;

	hppt_sbnm_insert_bulk(&heap);
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += sbnm_heap_min;
		sbnm_heap_demote(&heap, &k->node);
	}
	if (hppt_sbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap demote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value -= sbnm_heap_min;

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
	sbnm_heap_min = UINT_MAX;
	while (!pt_iter_entry(&hppt_entries, &k->value)) {
		sbnm_heap_min = min(k->value, sbnm_heap_min);
		k++;
	}

	return hppt_sbnm_validate();
}

static void
hppt_sbnm_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct sbnm_heap heap;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	hppt_sbnm_insert_bulk(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_sbnm_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct sbnm_heap heap;
	int              n;

	hppt_sbnm_insert_bulk(&heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		sbnm_heap_extract(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_sbnm_remove(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_sbnm_key *k;
	struct sbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_sbnm_insert_bulk(&heap);

	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		sbnm_heap_remove(&heap, &k->node);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}
}

static void
hppt_sbnm_promote(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_sbnm_key *k;
	struct sbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_sbnm_insert_bulk(&heap);

	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= sbnm_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		sbnm_heap_promote(&heap, &k->node);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += sbnm_heap_min;
}

static void
hppt_sbnm_demote(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_sbnm_key *k;
	struct sbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_sbnm_insert_bulk(&heap);

	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += sbnm_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		sbnm_heap_demote(&heap, &k->node);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = sbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value -= sbnm_heap_min;
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
static unsigned int          dbnm_heap_min;

static int
hppt_dbnm_compare_min(const struct dbnm_heap_node *first,
                      const struct dbnm_heap_node *second)
{
	return pt_compare_min((char *)&((struct hppt_dbnm_key *)first)->value,
	                      (char *)&((struct hppt_dbnm_key *)second)->value);
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
hppt_dbnm_check_heap(struct dbnm_heap *heap)
{
	int                   n;
	struct hppt_dbnm_key *cur, *old;

	old = dbnm_heap_entry(dbnm_heap_extract(heap, hppt_dbnm_compare_min),
	                      struct hppt_dbnm_key, node);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = dbnm_heap_entry(dbnm_heap_extract(heap,
		                                        hppt_dbnm_compare_min),
		                      struct hppt_dbnm_key, node);

		if (old->value > cur->value)
			return EXIT_FAILURE;

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_dbnm_validate(void)
{
	struct dbnm_heap      heap;
#if 0
	int                   n;
	struct hppt_dbnm_key *k;
#endif


	hppt_dbnm_insert_bulk(&heap);

	if (hppt_dbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap insert / extract scheme\n");
		return EXIT_FAILURE;
	}

#if 0
	hppt_dbnm_insert_bulk(&heap);
	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= dbnm_heap_min;
		dbnm_heap_promote(&heap, &k->node, hppt_dbnm_compare_min);
	}

	if (hppt_dbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap promote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += dbnm_heap_min;
#endif

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
	dbnm_heap_min = UINT_MAX;
	while (!pt_iter_entry(&hppt_entries, &k->value)) {
		dbnm_heap_min = min(k->value, dbnm_heap_min);
		k++;
	}

	return hppt_dbnm_validate();
}

static void
hppt_dbnm_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct dbnm_heap heap;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	hppt_dbnm_insert_bulk(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_dbnm_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct dbnm_heap heap;
	int              n;

	hppt_dbnm_insert_bulk(&heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		dbnm_heap_extract(&heap, hppt_dbnm_compare_min);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_dbnm_remove(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_dbnm_key *k;
	struct dbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		hppt_dbnm_insert_bulk(&heap);

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		dbnm_heap_remove(&heap, &k->node, hppt_dbnm_compare_min);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}
}

static void
hppt_dbnm_promote(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_dbnm_key *k;
	struct dbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_dbnm_insert_bulk(&heap);

	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= dbnm_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		dbnm_heap_update(&k->node, hppt_dbnm_compare_min);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = dbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += dbnm_heap_min;
}

#endif /* defined(CONFIG_DBNM_HEAP) */

/******************************************************************************
 * Parented LCRS based binomial heap
 ******************************************************************************/

#if defined(CONFIG_PBNM_HEAP)

struct hppt_pbnm_key {
	struct pbnm_heap_node *node;
	unsigned int           value;
};

static struct falloc         pbnm_alloc;
static struct hppt_pbnm_key *pbnm_heap_keys;
static unsigned int          pbnm_heap_min;

static int
hppt_pbnm_compare_min(const struct pbnm_heap_node *restrict first,
                      const struct pbnm_heap_node *restrict second)
{
	return pt_compare_min((char *)&pbnm_heap_entry(first,
	                                               struct hppt_pbnm_key,
	                                               node)->value,
	                      (char *)&pbnm_heap_entry(second,
	                                               struct hppt_pbnm_key,
	                                               node)->value);
}

static int
hppt_pbnm_doinsert(struct pbnm_heap *heap, struct hppt_pbnm_key *key)
{
	key->node = falloc_alloc(&pbnm_alloc);
	//key->node = malloc(sizeof(*key->node));
	if (!key->node)
		return EXIT_FAILURE;

	pbnm_heap_init_node(key->node, &key->node);

	pbnm_heap_insert(heap, key->node);

	return EXIT_SUCCESS;
}

static struct hppt_pbnm_key *
hppt_pbnm_doextract(struct pbnm_heap *heap)
{
	struct hppt_pbnm_key *key;

	key = pbnm_heap_entry(pbnm_heap_extract(heap), typeof(*key), node);

	//free(key->node);
	falloc_free(&pbnm_alloc, key->node);

	return key;
}

static void
hppt_pbnm_doremove(struct pbnm_heap *heap, struct hppt_pbnm_key *key)
{
	pbnm_heap_remove(heap, key->node);

	//free(key->node);
	falloc_free(&pbnm_alloc, key->node);
}

static int
hppt_pbnm_insert_bulk(struct pbnm_heap *heap)
{
	int                   n;
	struct hppt_pbnm_key *k;

	pbnm_heap_init(heap, hppt_pbnm_compare_min);

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		if (hppt_pbnm_doinsert(heap, k))
			break;

	if (n == hppt_entries.pt_nr)
		return EXIT_SUCCESS;

	while (n--)
		//free(pbnm_heap_keys[n].node);
		falloc_free(&pbnm_alloc, pbnm_heap_keys[n].node);

	fprintf(stderr, "failed to allocate heap node\n");

	return EXIT_FAILURE;
}

static int
hppt_pbnm_check_heap(struct pbnm_heap *heap)
{
	int                   n;
	struct hppt_pbnm_key *cur, *old;

	old = hppt_pbnm_doextract(heap);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = hppt_pbnm_doextract(heap);

		if (old->value > cur->value)
			return EXIT_FAILURE;

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_pbnm_validate(void)
{
	struct pbnm_heap      heap;
	int                   n;
	struct hppt_pbnm_key *k;

	if (hppt_pbnm_insert_bulk(&heap))
		return EXIT_FAILURE;

	if (hppt_pbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap insert / extract scheme\n");
		return EXIT_FAILURE;
	}

	if (hppt_pbnm_insert_bulk(&heap))
		return EXIT_FAILURE;

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= pbnm_heap_min;
		pbnm_heap_promote(&heap, k->node);
	}
	if (hppt_pbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap promote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += pbnm_heap_min;

	if (hppt_pbnm_insert_bulk(&heap))
		return EXIT_FAILURE;

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += pbnm_heap_min;
		pbnm_heap_demote(&heap, k->node);
	}
	if (hppt_pbnm_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap demote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value -= pbnm_heap_min;

	return EXIT_SUCCESS;
}

static int
hppt_pbnm_load(const char *pathname)
{
	struct hppt_pbnm_key *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	pbnm_heap_keys = malloc(hppt_entries.pt_nr * sizeof(*pbnm_heap_keys));
	if (!pbnm_heap_keys)
		return EXIT_FAILURE;

	falloc_init(&pbnm_alloc, sizeof(*k->node));

	pt_init_entry_iter(&hppt_entries);

	k = pbnm_heap_keys;
	pbnm_heap_min = UINT_MAX;
	while (!pt_iter_entry(&hppt_entries, &k->value)) {
		pbnm_heap_min = min(k->value, pbnm_heap_min);
		k++;
	}

	return hppt_pbnm_validate();
}

static void
hppt_pbnm_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct pbnm_heap heap;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	hppt_pbnm_insert_bulk(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_pbnm_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct pbnm_heap heap;
	int              n;

	hppt_pbnm_insert_bulk(&heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		hppt_pbnm_doextract(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_pbnm_remove(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_pbnm_key *k;
	struct pbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_pbnm_insert_bulk(&heap);

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		hppt_pbnm_doremove(&heap, k);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}
}

static void
hppt_pbnm_promote(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_pbnm_key *k;
	struct pbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_pbnm_insert_bulk(&heap);

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= pbnm_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		pbnm_heap_promote(&heap, k->node);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		//free(k->node);
		falloc_free(&pbnm_alloc, k->node);
		k->value += pbnm_heap_min;
	}
}

static void
hppt_pbnm_demote(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_pbnm_key *k;
	struct pbnm_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_pbnm_insert_bulk(&heap);

	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += pbnm_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		pbnm_heap_demote(&heap, k->node);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = pbnm_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		//free(k->node);
		falloc_free(&pbnm_alloc, k->node);
		k->value -= pbnm_heap_min;
	}
}

#endif /* defined(CONFIG_PBNM_HEAP) */

/******************************************************************************
 * Singly linked list based pairing heap
 ******************************************************************************/

#if defined(CONFIG_SPAIR_HEAP)

struct hppt_spair_key {
	struct lcrs_node node;
	unsigned int     value;
};

static struct hppt_spair_key *spair_heap_keys;
static unsigned int           spair_heap_min;

static int
hppt_spair_compare_min(const struct lcrs_node *restrict first,
                       const struct lcrs_node *restrict second)
{
	return pt_compare_min((char *)&((struct hppt_spair_key *)
	                                first)->value,
	                      (char *)&((struct hppt_spair_key *)
	                                second)->value);
}

static void
hppt_spair_insert_bulk(struct spair_heap *heap)
{
	int                   n;
	struct hppt_spair_key *k;

	spair_heap_init(heap);

	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		spair_heap_insert(heap, &k->node, hppt_spair_compare_min);
}

static int
hppt_spair_check_heap(struct spair_heap *heap)
{
	int                    n;
	struct hppt_spair_key *cur, *old;

	old = spair_heap_entry(spair_heap_extract(heap, hppt_spair_compare_min),
	                       struct hppt_spair_key, node);

	for (n = 1; n < hppt_entries.pt_nr; n++) {
		cur = spair_heap_entry(
			spair_heap_extract(heap, hppt_spair_compare_min),
			struct hppt_spair_key, node);

		if (old->value > cur->value)
			return EXIT_FAILURE;

		old = cur;
	}

	return EXIT_SUCCESS;
}

static int
hppt_spair_validate(void)
{
	struct spair_heap      heap;
	int                    n;
	struct hppt_spair_key *k;

	hppt_spair_insert_bulk(&heap);
	if (hppt_spair_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap insert / extract scheme\n");
		return EXIT_FAILURE;
	}

	hppt_spair_insert_bulk(&heap);
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= spair_heap_min;
		spair_heap_promote(&heap, &k->node, hppt_spair_compare_min);
	}
	if (hppt_spair_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap promote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += spair_heap_min;

	hppt_spair_insert_bulk(&heap);
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += spair_heap_min;
		spair_heap_demote(&heap, &k->node, hppt_spair_compare_min);
	}
	if (hppt_spair_check_heap(&heap)) {
		fprintf(stderr, "Bogus heap demote scheme\n");
		return EXIT_FAILURE;
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value -= spair_heap_min;

	return EXIT_SUCCESS;
}

static int
hppt_spair_load(const char *pathname)
{
	struct hppt_spair_key *k;

	if (pt_open_entries(pathname, &hppt_entries))
		return EXIT_FAILURE;

	spair_heap_keys = malloc(hppt_entries.pt_nr * sizeof(*spair_heap_keys));
	if (!spair_heap_keys)
		return EXIT_FAILURE;

	pt_init_entry_iter(&hppt_entries);

	k = spair_heap_keys;
	spair_heap_min = UINT_MAX;
	while (!pt_iter_entry(&hppt_entries, &k->value)) {
		spair_heap_min = min(k->value, spair_heap_min);
		k++;
	}

	return hppt_spair_validate();
}

static void
hppt_spair_insert(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct spair_heap heap;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	hppt_spair_insert_bulk(&heap);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_spair_extract(unsigned long long *nsecs)
{
	struct timespec  start, elapse;
	struct spair_heap heap;
	int              n;

	hppt_spair_insert_bulk(&heap);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for (n = 0; n < hppt_entries.pt_nr; n++)
		spair_heap_extract(&heap, hppt_spair_compare_min);
	clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);
	*nsecs = pt_tspec2ns(&elapse);
}

static void
hppt_spair_remove(unsigned long long *nsecs)
{
	int                   n;
	struct hppt_spair_key *k;
	struct spair_heap      heap;
	struct timespec       start, elapse;

	*nsecs = 0;

	hppt_spair_insert_bulk(&heap);

	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		spair_heap_remove(&heap, &k->node, hppt_spair_compare_min);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}
}

static void
hppt_spair_promote(unsigned long long *nsecs)
{
	int                    n;
	struct hppt_spair_key *k;
	struct spair_heap      heap;
	struct timespec        start, elapse;

	*nsecs = 0;

	hppt_spair_insert_bulk(&heap);

	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value -= spair_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		spair_heap_promote(&heap, &k->node, hppt_spair_compare_min);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value += spair_heap_min;
}

static void
hppt_spair_demote(unsigned long long *nsecs)
{
	int                    n;
	struct hppt_spair_key *k;
	struct spair_heap      heap;
	struct timespec        start, elapse;

	*nsecs = 0;

	hppt_spair_insert_bulk(&heap);

	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++) {
		k->value += spair_heap_min;

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		spair_heap_demote(&heap, &k->node, hppt_spair_compare_min);
		clock_gettime(CLOCK_MONOTONIC_RAW, &elapse);

		elapse = pt_tspec_sub(&elapse, &start);
		*nsecs += pt_tspec2ns(&elapse);
	}

	/*
	 * Reset keys to their original values so that next computation loop
	 * gives consistent numbers...
	 */
	for (n = 0, k = spair_heap_keys; n < hppt_entries.pt_nr; n++, k++)
		k->value -= spair_heap_min;
}

#endif /* defined(CONFIG_SPAIR_HEAP) */

/******************************************************************************
 * Main measurment task handling
 ******************************************************************************/

static const struct hppt_iface hppt_algos[] = {
#if defined(CONFIG_FBNR_HEAP)
	{
		.hppt_name    = "fbnr",
		.hppt_load    = hppt_fbnr_load,
		.hppt_insert  = hppt_fbnr_insert,
		.hppt_extract = hppt_fbnr_extract,
		.hppt_remove  = NULL,
		.hppt_build   = hppt_fbnr_build
	},
#endif
#if defined(CONFIG_FWK_HEAP)
	{
		.hppt_name    = "fwk",
		.hppt_load    = hppt_fwk_load,
		.hppt_insert  = hppt_fwk_insert,
		.hppt_extract = hppt_fwk_extract,
		.hppt_remove  = NULL,
		.hppt_build   = hppt_fwk_build
	},
#endif
#if defined(CONFIG_SBNM_HEAP)
	{
		.hppt_name    = "sbnm",
		.hppt_load    = hppt_sbnm_load,
		.hppt_insert  = hppt_sbnm_insert,
		.hppt_extract = hppt_sbnm_extract,
		.hppt_remove  = hppt_sbnm_remove,
		.hppt_promote = hppt_sbnm_promote,
		.hppt_demote  = hppt_sbnm_demote
	},
#endif
#if defined(CONFIG_DBNM_HEAP)
	{
		.hppt_name    = "dbnm",
		.hppt_load    = hppt_dbnm_load,
		.hppt_insert  = hppt_dbnm_insert,
		.hppt_extract = hppt_dbnm_extract,
		.hppt_remove  = hppt_dbnm_remove,
		.hppt_promote = hppt_dbnm_promote
	},
#endif
#if defined(CONFIG_SPAIR_HEAP)
	{
		.hppt_name    = "spair",
		.hppt_load    = hppt_spair_load,
		.hppt_insert  = hppt_spair_insert,
		.hppt_extract = hppt_spair_extract,
		.hppt_remove  = hppt_spair_remove,
		.hppt_promote = hppt_spair_promote,
		.hppt_demote  = hppt_spair_demote
	},
#endif
#if defined(CONFIG_PBNM_HEAP)
	{
		.hppt_name    = "pbnm",
		.hppt_load    = hppt_pbnm_load,
		.hppt_insert  = hppt_pbnm_insert,
		.hppt_extract = hppt_pbnm_extract,
		.hppt_remove  = hppt_pbnm_remove,
		.hppt_promote = hppt_pbnm_promote,
		.hppt_demote  = hppt_pbnm_demote
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

	fprintf(stderr, "Invalid \"%s\" heap algorithm\n", algo_name);

	return NULL;
}

static int
pt_parse_scheme(const char               *arg,
                const char              **scheme,
                const struct hppt_iface  *algo)
{
	if (!strcmp(arg, "insert")) {
		if (!algo->hppt_insert)
			goto inval;
	}
	else if (!strcmp(arg, "extract")) {
		if (!algo->hppt_extract)
			goto inval;
	}
	else if (!strcmp(arg, "build")) {
		if (!algo->hppt_build)
			goto inval;
	}
	else if (!strcmp(arg, "remove")) {
		if (!algo->hppt_remove)
			goto inval;
	}
	else if (!strcmp(arg, "promote")) {
		if (!algo->hppt_promote)
			goto inval;
	}
	else if (!strcmp(arg, "demote")) {
		if (!algo->hppt_demote)
			goto inval;
	}
	else {
		fprintf(stderr, "Unknown \"%s\" heap scheme\n", arg);
		return EXIT_FAILURE;
	}

	*scheme = arg;

	return EXIT_SUCCESS;

inval:
	fprintf(stderr, "Invalid \"%s\" heap scheme\n", arg);
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

int main(int argc, char *argv[])
{
	const struct hppt_iface *algo;
	unsigned int             l, loops = 0;
	int                      prio = 0;
	const char              *scheme = "";
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
	if (argc < 3) {
		fprintf(stderr, "Invalid number of arguments\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	algo = hppt_setup_algo(argv[optind + 1]);
	if (!algo)
		return EXIT_FAILURE;

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (argc == 4) {
		if (pt_parse_scheme(argv[optind + 3], &scheme, algo))
			return EXIT_FAILURE;
	}

	if (algo->hppt_load(argv[optind]))
		return EXIT_FAILURE;

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	if ((!*scheme && algo->hppt_insert) || !strcmp(scheme, "insert")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_insert(&nsecs);
			printf("insert: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->hppt_extract) || !strcmp(scheme, "extract")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_extract(&nsecs);
			printf("extract: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->hppt_build) || !strcmp(scheme, "build")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_build(&nsecs);
			printf("build: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->hppt_remove) || !strcmp(scheme, "remove")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_remove(&nsecs);
			printf("remove: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->hppt_promote) || !strcmp(scheme, "promote")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_promote(&nsecs);
			printf("promote: nsec=%llu\n", nsecs);
		}
	}

	if ((!*scheme && algo->hppt_demote) || !strcmp(scheme, "demote")) {
		for (l = 0; l < loops; l++) {
			algo->hppt_demote(&nsecs);
			printf("demote: nsec=%llu\n", nsecs);
		}
	}

	return EXIT_SUCCESS;
}
