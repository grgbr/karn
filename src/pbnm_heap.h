#ifndef _PBNM_HEAP_H
#define _PBNM_HEAP_H

#include <utils.h>
#include <stdbool.h>

struct pbnm_heap_node {
	struct pbnm_heap_node  *pbnm_sibling;
	struct pbnm_heap_node  *pbnm_parent;
	struct pbnm_heap_node  *pbnm_youngest;
	unsigned int            pbnm_rank;
	struct pbnm_heap_node **pbnm_handle;
};

#define pbnm_heap_entry(_node, _type, _member) \
	containerof((_node)->pbnm_handle, _type, _member)

static inline void
pbnm_heap_init_node(struct pbnm_heap_node *node, struct pbnm_heap_node **handle)
{
	node->pbnm_handle = handle;
	*handle = node;
}

typedef int (pbnm_heap_compare_fn)(const struct pbnm_heap_node *restrict first,
                                   const struct pbnm_heap_node *restrict second);

struct pbnm_heap {
	struct pbnm_heap_node *pbnm_roots;
	unsigned int           pbnm_count;
	pbnm_heap_compare_fn  *pbnm_compare;
};

#define PBNM_HEAP_INIT(_compare)          \
	{                                 \
		.pbnm_roots = NULL,       \
		.pbnm_count = 0,          \
		.pbnm_compare = _compare  \
	}

#define pbnm_heap_assert(_heap)                             \
	assert(_heap);                                      \
	assert(!(_heap)->pbnm_roots ^ (_heap)->pbnm_count); \
	assert((_heap)->pbnm_compare)

extern void pbnm_heap_insert(struct pbnm_heap      *heap,
                             struct pbnm_heap_node *key);

extern struct pbnm_heap_node * pbnm_heap_peek(struct pbnm_heap *heap);

extern struct pbnm_heap_node * pbnm_heap_extract(struct pbnm_heap *heap);

extern void pbnm_heap_remove(struct pbnm_heap      *heap,
                             struct pbnm_heap_node *key);

extern void pbnm_heap_promote(struct pbnm_heap      *heap,
                              struct pbnm_heap_node *key);

extern void pbnm_heap_demote(struct pbnm_heap      *heap,
                             struct pbnm_heap_node *key);

extern void pbnm_heap_merge(struct pbnm_heap *result,
                            struct pbnm_heap *source);

static inline unsigned int
pbnm_heap_count(const struct pbnm_heap* heap)
{
	pbnm_heap_assert(heap);

	return heap->pbnm_count;
}

static inline bool
pbnm_heap_empty(const struct pbnm_heap* heap)
{
	pbnm_heap_assert(heap);

	return heap->pbnm_count == 0;
}

static inline void
pbnm_heap_init(struct pbnm_heap *heap, pbnm_heap_compare_fn *compare)
{
	assert(heap);
	assert(compare);

	heap->pbnm_roots = NULL;
	heap->pbnm_count = 0;
	heap->pbnm_compare = compare;
}

static inline void
pbnm_heap_fini(struct pbnm_heap *heap __unused)
{
	pbnm_heap_assert(heap);
}

#endif
