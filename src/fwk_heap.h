#ifndef _FWK_HEAP_H
#define _FWK_HEAP_H

#include "fbmp.h"
#include "farr.h"

struct fwk_heap {
	/** Node comparator */
	farr_compare_fn *fwk_compare;
	/** Node copier */
	farr_copy_fn    *fwk_copy;
	unsigned int     fwk_count;
	struct fbmp      fwk_rbits;
	struct farr      fwk_nodes;
};

#define FWK_HEAP_ROOT_INDEX (0U)

static inline unsigned int fwk_heap_count(const struct fwk_heap *heap)
{
	return heap->fwk_count;
}

static inline unsigned int fwk_heap_nr(const struct fwk_heap *heap)
{
	return farr_nr(&heap->fwk_nodes);
}

static inline unsigned int fwk_heap_empty(const struct fwk_heap *heap)
{
	return !fwk_heap_count(heap);
}

static inline unsigned int fwk_heap_full(const struct fwk_heap *heap)
{
	return fwk_heap_count(heap) == fwk_heap_nr(heap);
}

static inline char * fwk_heap_peek(const struct fwk_heap *heap)
{
	return farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX);
}

extern void fwk_heap_insert(struct fwk_heap *heap, const char *node);

extern void fwk_heap_extract(struct fwk_heap *heap, char *node);

extern void fwk_heap_build(struct fwk_heap *heap, unsigned int count);

extern void fwk_heap_fini(struct fwk_heap *heap);

extern int fwk_heap_init(struct fwk_heap *heap,
                         char            *nodes,
                         size_t           node_size,
                         unsigned int     node_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy);

#endif
