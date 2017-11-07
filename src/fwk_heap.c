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

/*
 * Making this "inline" improves build runtime performance by around 90% upon
 * random input (~ 25% for extraction)...
 */
static inline bool fwk_heap_join(struct farr     *nodes,
                                 struct fbmp     *rbits,
                                 unsigned int     dancestor,
                                 unsigned int     child,
                                 farr_compare_fn *compare,
                                 farr_copy_fn    *copy)
{
	char *dnode;
	char *cnode;

	dnode = farr_slot(nodes, dancestor);
	cnode = farr_slot(nodes, child);

	if (compare(cnode, dnode) < 0) {
		char tmp[farr_slot_size(nodes)];

		copy(tmp, dnode);
		copy(dnode, cnode);
		copy(cnode, tmp);

		fbmp_toggle(rbits, child);

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

			if (fwk_heap_join(&heap->fwk_nodes, &heap->fwk_rbits,
			                  didx, idx, heap->fwk_compare,
			                  heap->fwk_copy))
				break;

			idx = didx;
		} while (idx != FWK_HEAP_ROOT_INDEX);
	}

	heap->fwk_count++;
}

static void fwk_heap_siftdown(struct farr     *nodes,
                              struct fbmp     *rbits,
                              unsigned int     count,
                              farr_compare_fn *compare,
                              farr_copy_fn    *copy)
{
	unsigned int idx;

	idx = fwk_heap_right_index(rbits, FWK_HEAP_ROOT_INDEX);

	while (true) {
		unsigned int cidx;

		cidx = fwk_heap_left_index(rbits, idx);
		if (cidx >= count)
			break;

		idx = cidx;
	}

	while (idx != FWK_HEAP_ROOT_INDEX) {
		fwk_heap_join(nodes, rbits, FWK_HEAP_ROOT_INDEX, idx, compare,
		              copy);

		idx = fwk_heap_parent_index(idx);
	}
}

void fwk_heap_extract(struct fwk_heap *heap, char *node)
{
	assert(!fwk_heap_empty(heap));

	heap->fwk_count--;

	heap->fwk_copy(node, farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX));

	heap->fwk_copy(farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX),
	               farr_slot(&heap->fwk_nodes, heap->fwk_count));

	/* Sift new root node down, i.e. reestablish heap ordering. */
	if (heap->fwk_count > 1)
		fwk_heap_siftdown(&heap->fwk_nodes, &heap->fwk_rbits,
		                  heap->fwk_count, heap->fwk_compare,
		                  heap->fwk_copy);
}

void fwk_heap_clear(struct fwk_heap *heap)
{
	heap->fwk_count = 0;
	fbmp_clear_all(&heap->fwk_rbits, farr_nr(&heap->fwk_nodes));
}

static void fwk_heap_make(struct farr     *nodes,
                          struct fbmp     *rbits,
                          unsigned int     count,
                          farr_compare_fn *compare,
                          farr_copy_fn    *copy)
{
	while (--count) {
		unsigned int didx;
#if 0
		didx = fwk_heap_dancestor_index(heap, count);
#else
		didx = count >> (__builtin_ctz(count) + 1);
#endif
		fwk_heap_join(nodes, rbits, didx, count, compare, copy);
	}
}

void fwk_heap_build(struct fwk_heap *heap, unsigned int count)
{
	heap->fwk_count = count;

	fbmp_clear_all(&heap->fwk_rbits, farr_nr(&heap->fwk_nodes));

	fwk_heap_make(&heap->fwk_nodes, &heap->fwk_rbits, count,
	              heap->fwk_compare, heap->fwk_copy);
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

struct fwk_heap * fwk_heap_create(size_t           node_size,
                                  unsigned int     node_nr,
                                  farr_compare_fn *compare,
                                  farr_copy_fn    *copy)
{
	assert(node_size);
	assert(node_nr);

	struct fwk_heap *heap;
	int              err;

	heap = malloc(sizeof(*heap) + (node_size * node_nr));
	if (!heap)
		return NULL;

	err = fwk_heap_init(heap, (char *)&heap[1], node_size, node_nr, compare,
	                    copy);
	if (err) {
		free(heap);
		errno = -err;

		return NULL;
	}

	return heap;
}

void fwk_heap_destroy(struct fwk_heap *heap)
{
	fwk_heap_fini(heap);

	free(heap);
}

#if defined(CONFIG_FWK_HEAP_SORT)

int fwk_heap_sort(char            *entries,
                  size_t           entry_size,
                  size_t           entry_nr,
                  farr_compare_fn *compare,
                  farr_copy_fn    *copy)
{
	if (entry_nr > 1) {
		struct farr  nodes;
		struct fbmp  rbits;
		char        *root;
		char        *last;
		char         tmp[entry_size];

		if (fbmp_init(&rbits, entry_nr))
			return -ENOMEM;

		farr_init(&nodes, entries, entry_size, entry_nr);

		fwk_heap_make(&nodes, &rbits, entry_nr, compare, copy);

		root = farr_slot(&nodes, FWK_HEAP_ROOT_INDEX);
		last = &entries[(entry_nr - 1) * entry_size];

		while (true) {
			copy(tmp, root);
			copy(root, last);
			copy(last, tmp);

			if (--entry_nr <= 1)
				break;

			fwk_heap_siftdown(&nodes, &rbits, entry_nr, compare,
			                  copy);

			last -= entry_size;
		}

		farr_fini(&nodes);
		fbmp_fini(&rbits);
	}

	return 0;
}

#endif /* defined(CONFIG_FWK_HEAP_SORT) */

#if 0
	if (count > 1)
		fwk_heap_topdown_build(heap);
#endif
#if 0
static void fwk_heap_topdown_build(struct fwk_heap *heap)
{
#if 0
	unsigned int j = (2 * index) + 1;

	printf("in %u\n", index);
	while (j < (heap->fwk_count / 2)) {
		fwk_heap_topdown_build(heap, j);
		j *= 2;
	}

	fwk_heap_siftdown(heap, index, heap->fwk_count);
	printf("out %u\n", index);
#endif

	unsigned int stack[32];
	unsigned int s = 0;
	unsigned int index = FWK_HEAP_ROOT_INDEX;
	unsigned int cnt = heap->fwk_count / 2;

	do {
		if (index < cnt) {
			stack[s++] = index;
			index = (2 * index) + 1;
			continue;
		}

		if (!s--)
			break;
		index = stack[s];

		fwk_heap_siftdown(heap, index, heap->fwk_count);

		index *= 2;
		if (!index)
			break;
	} while (true);
}
#endif
