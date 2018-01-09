#ifndef _KARN_HPAIR_HEAP_H
#define _KARN_HPAIR_HEAP_H

#include <plbst.h>

struct hpair_node {
	struct plbst_node hpair_plbst;
};

typedef int (hpair_compare_fn)(const struct hpair_node *first,
                               const struct hpair_node *second);

struct hpair_heap {
	unsigned int       hpair_count;
	struct plbst_node *hpair_root;
	hpair_compare_fn  *hpair_compare;
};

extern void hpair_heap_insert(struct hpair_heap *heap, struct hpair_node *key);

extern struct hpair_node * hpair_heap_extract(struct hpair_heap *heap);

extern void hpair_heap_merge(struct hpair_heap *result,
                             struct hpair_heap *source);

extern void hpair_heap_remove(struct hpair_heap *heap, struct hpair_node *key);

extern void hpair_heap_promote(struct hpair_heap *heap, struct hpair_node *key);

extern void hpair_heap_demote(struct hpair_heap *heap, struct hpair_node *key);

static inline void
hpair_heap_init(struct hpair_heap *heap, hpair_compare_fn *compare)
{
	heap->hpair_count = 0;
	heap->hpair_root = NULL;
	heap->hpair_compare = compare;
}

#endif
