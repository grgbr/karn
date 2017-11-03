#include "fwk_heap.h"

static unsigned int fwk_heap_parent_index(unsigned int index)
{
	assert(index);

	return index / 2;
}

static unsigned int fwk_heap_left_index(const struct fbmp *rbits,
                                        unsigned int       index)
{
	return (2 * index) + (unsigned int)fbmp_test(rbits, index);
}

static unsigned int fwk_heap_right_index(const struct fbmp *rbits,
                                         unsigned int       index)
{
	return (2 * index) + 1 - (unsigned int)fbmp_test(rbits, index);
}

static bool fwk_heap_isleft_child(const struct fbmp *rbits,
                                  unsigned int       index)
{
	return (!!(index & 1)) == fbmp_test(rbits,
	                                    fwk_heap_parent_index(index));
}

/*
 * Return index of the so-called distinguished ancestor of node specified by
 * index.
 *
 * The distinguished ancestor of a node located at "index" is the parent of
 * "index" if "index" points to a right child, and the distinguished ancestor of
 * the parent of "index" if "index" is a left child.
 */
static unsigned int fwk_heap_dancestor_index(const struct fwk_heap *heap,
                                             unsigned int           index)
{
	/* Move up untill index points to a right child. */
	while (fwk_heap_isleft_child(&heap->fwk_rbits, index))
		index = fwk_heap_parent_index(index);

	/* Then return its parent. */
	return fwk_heap_parent_index(index);
}

static bool fwk_heap_join(struct fwk_heap *heap,
                          unsigned int     dancestor,
                          unsigned int     child)
{
	char *dnode;
	char *cnode;

	dnode = farr_slot(&heap->fwk_nodes, dancestor);
	cnode = farr_slot(&heap->fwk_nodes, child);

	if (heap->fwk_compare(cnode, dnode) < 0) {
		char tmp[farr_slot_size(&heap->fwk_nodes)];

		heap->fwk_copy(tmp, dnode);
		heap->fwk_copy(dnode, cnode);
		heap->fwk_copy(cnode, tmp);

		fbmp_toggle(&heap->fwk_rbits, child);

		return false;
	}

	return true;
}

void fwk_heap_insert(struct fwk_heap *heap, const char *node)
{
	assert(!fwk_heap_full(heap));

	unsigned int idx = heap->fwk_count;

	heap->fwk_copy(farr_slot(&heap->fwk_nodes, idx), node);

	fbmp_clear(&heap->fwk_rbits, idx);

	if (idx) {
		if (!(idx & 1))
			/*
			 * If this leaf is the only child of its parent, we make
			 * it a left child by updating the reverse bit at the
			 * parent to save an unnecessary element comparison.
			 */
			fbmp_clear(&heap->fwk_rbits,
			           fwk_heap_parent_index(idx));

		/* Sift inserted node up, i.e. reestablish heap ordering. */
		do {
			unsigned int didx;

			didx = fwk_heap_dancestor_index(heap, idx);
			if (fwk_heap_join(heap, didx, idx))
				break;

			idx = didx;
		} while (idx != FWK_HEAP_ROOT_INDEX);
	}

	heap->fwk_count++;
}

void fwk_heap_extract(struct fwk_heap *heap, char *node)
{
	assert(!fwk_heap_empty(heap));

	unsigned int cnt = --heap->fwk_count;

	heap->fwk_copy(node, farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX));

	heap->fwk_copy(farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX),
	               farr_slot(&heap->fwk_nodes, cnt));

	/* Sift new root node dow, i.e. reestablish heap ordering. */
	if (cnt > 1) {
		unsigned int idx;

		idx = fwk_heap_right_index(&heap->fwk_rbits,
		                           FWK_HEAP_ROOT_INDEX);
		while (true) {
			unsigned int cidx;

			cidx = fwk_heap_left_index(&heap->fwk_rbits, idx);
			if (cidx >= cnt)
				break;

			idx = cidx;
		}

		while (idx != FWK_HEAP_ROOT_INDEX) {
			fwk_heap_join(heap, FWK_HEAP_ROOT_INDEX, idx);
			idx = fwk_heap_parent_index(idx);
		}
	}
}

void fwk_heap_build(struct fwk_heap *heap, unsigned int count)
{
	heap->fwk_count = count;

	fbmp_clear_all(&heap->fwk_rbits, farr_nr(&heap->fwk_nodes));

	while (--count) {
		unsigned int didx;

		didx = fwk_heap_dancestor_index(heap, count);
		fwk_heap_join(heap, didx, count);
	}
}

int fwk_heap_init(struct fwk_heap *heap,
                  char            *nodes,
                  size_t           node_size,
                  unsigned int     node_nr,
                  farr_compare_fn *compare,
                  farr_copy_fn    *copy)
{
	int err;
	
	err = fbmp_init(&heap->fwk_rbits, node_nr);
	if (err)
		return err;

	heap->fwk_compare = compare;
	heap->fwk_copy = copy;
	heap->fwk_count = 0;

	farr_init(&heap->fwk_nodes, nodes, node_size, node_nr);

	return 0;
}

void fwk_heap_fini(struct fwk_heap *heap)
{
	farr_fini(&heap->fwk_nodes);
	fbmp_fini(&heap->fwk_rbits);
}
