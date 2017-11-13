/**
 * @file      fwk_heap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      02 Nov 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based weak heap implementation
 *
 * This file is part of Karn
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fwk_heap.h"

#define FWK_HEAP_REGULAR_ORDER (true)
#define FWK_HEAP_REVERSE_ORDER (false)

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

static unsigned int fwk_heap_bottom_index(const struct fwk_heap *heap)
{
	return heap->fwk_count;
}

static bool fwk_heap_isleft_child(const struct fbmp *rbits,
                                  unsigned int       index)
{
	return (!!(index & 1)) == fbmp_test(rbits,
	                                    fwk_heap_parent_index(index));
}

static bool fwk_heap_single_leaf(unsigned int index)
{
	return !(index & 1);
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
	/* Root has no ancestor... */
	assert(index);

	/* Move up untill index points to a right child. */
	while (fwk_heap_isleft_child(&heap->fwk_rbits, index))
		index = fwk_heap_parent_index(index);

	/* Then return its parent. */
	return fwk_heap_parent_index(index);
}

/*
 * Join weak sub-heaps rooted at "node" node and its distinguished ancestor
 * "dancestor" into a single one rooted at "dancestor" (ensuring proper nodes
 * ordering).
 *
 * Let Ai and Aj be 2 nodes in a weak heap such that the element at Ai is
 * smaller than or equal to every element in the left subtree of Aj.
 * Conceptually, Aj and its right subtree form a weak heap, while Ai and the
 * left subtree of Aj form another weak heap (note that Ai cannot be a
 * descendant of Aj). If the element at Aj is smaller than that at Ai, the 2
 * elements are swapped and Rj (Aj's reverse bit) is flipped. As a result, the
 * element at Aj will be smaller than or equal to every element in its right
 * subtree, and the element at Ai will be smaller than or equal to every element
 * in the subtree rooted at Aj.

 * Making this "inline" improves build runtime performance by around 90% upon
 * random input (~ 25% for extraction)...
 */
static inline bool fwk_heap_join(const struct farr *nodes,
                                 const struct fbmp *rbits,
                                 unsigned int       dancestor,
                                 unsigned int       node,
                                 farr_compare_fn   *compare,
                                 farr_copy_fn      *copy,
                                 bool               regular)
{
	char *cnode;
	char *dnode;

	cnode = farr_slot(nodes, node);
	dnode = farr_slot(nodes, dancestor);

	if ((compare(cnode, dnode) < 0) == regular) {
		/*
		 * Swap node and its distinguished ancestor to restore proper
		 * heap ordering.
		 */
		char tmp[farr_slot_size(nodes)];

		copy(tmp, cnode);
		copy(cnode, dnode);
		copy(dnode, tmp);

		/* Flip former distinguished ancestor's children. */
		fbmp_toggle(rbits, node);

		return false;
	}

	return true;
}

void fwk_heap_insert(struct fwk_heap *heap, const char *node)
{
	assert(!fwk_heap_full(heap));
	assert(node);

	unsigned int       idx = fwk_heap_bottom_index(heap);
	farr_copy_fn      *cpy = heap->fwk_copy;
	farr_compare_fn   *cmp = heap->fwk_compare;
	const struct farr *nodes = &heap->fwk_nodes;
	const struct fbmp *rbits = &heap->fwk_rbits;

	cpy(farr_slot(nodes, idx), node);

	fbmp_clear(rbits, idx);

	if (idx) {
		if (fwk_heap_single_leaf(idx))
			/*
			 * If this leaf is the only child of its parent, we make
			 * it a left child by updating the reverse bit at the
			 * parent to save an unnecessary element comparison.
			 */
			fbmp_clear(rbits, fwk_heap_parent_index(idx));

		/*
		 * Sift inserted node up, i.e. reestablish heap ordering between
		 * element initially located at idx and those located at its
		 * ancestors.
		 */
		do {
			unsigned int didx;

			/* Find distinguished ancestor for current node */
			didx = fwk_heap_dancestor_index(heap, idx);

			/*
			 * Join both sub-heaps rooted at current node and its
			 * distinguished ancestor.
			 */
			if (fwk_heap_join(nodes, rbits, didx, idx, cmp, cpy,
			                  FWK_HEAP_REGULAR_ORDER))
				break;

			/* Iterate up to root. */
			idx = didx;
		} while (idx != FWK_HEAP_ROOT_INDEX);
	}

	heap->fwk_count++;
}

/*
 * Re-establish weak-heap ordering between elements located at root and all its
 * descendants (i.e., in root's right subtree since it has no left child).
 *
 * Starting from the right child of root, the last node on the left spine of the
 * right subtree of root is identified by repeatedly visiting left children
 * until reaching a node that has no left child.
 * The path from this node to the right child of root is traversed upwards,
 * and join operations are repeatedly performed between root and the nodes along
 * this path.
 * The correctness of the siftdown operation follows from the fact that, after
 * each join, the element at root is less than or equal to every element in
 * the left subtree of the node considered in the next join.
 */
static void fwk_heap_siftdown(const struct farr *nodes,
                              const struct fbmp *rbits,
                              unsigned int       count,
                              farr_compare_fn   *compare,
                              farr_copy_fn      *copy,
                              bool               regular)
{
	unsigned int idx;

	idx = fwk_heap_right_index(rbits, FWK_HEAP_ROOT_INDEX);

	/* Identify last node of root's right subtree left spine. */
	while (true) {
		unsigned int cidx;

		cidx = fwk_heap_left_index(rbits, idx);
		if (cidx >= count)
			break;

		idx = cidx;
	}

	/*
	 * Now, traverse back up to the root, restoring weak heap property at
	 * each visited node.
	 */
	while (idx != FWK_HEAP_ROOT_INDEX) {
		fwk_heap_join(nodes, rbits, FWK_HEAP_ROOT_INDEX, idx, compare,
		              copy, regular);

		idx = fwk_heap_parent_index(idx);
	}
}

void fwk_heap_extract(struct fwk_heap *heap, char *node)
{
	assert(!fwk_heap_empty(heap));
	assert(node);

	const struct farr *nodes = &heap->fwk_nodes;
	char              *root = farr_slot(nodes, FWK_HEAP_ROOT_INDEX);
	farr_copy_fn      *cpy = heap->fwk_copy;
	unsigned int       cnt = --heap->fwk_count;

	/* Copy root node to caller specified location. */
	cpy(node, root);

	/* Copy last node to root location. */
	cpy(root, farr_slot(nodes, cnt));

	/* Sift new root node down, i.e. reestablish heap ordering. */
	if (cnt > 1)
		fwk_heap_siftdown(nodes, &heap->fwk_rbits, cnt,
		                  heap->fwk_compare, cpy,
		                  FWK_HEAP_REGULAR_ORDER);
}

void fwk_heap_clear(struct fwk_heap *heap)
{
	heap->fwk_count = 0;

	fbmp_clear_all(&heap->fwk_rbits, farr_nr(&heap->fwk_nodes));
}

/*
 *
 * Compute index of the distinguished ancestor of node specified by index in an
 * optimized manner.
 *
 * When reverse bits are known to be 0 and set bottom-up while scanning nodes,
 * distinguished ancestor can be computed from the array index by considering
 * the position of the least-significant 1-bit.
 *
 * Unpredictable result if a 0 "index" is passed as argument.
 */
static unsigned int fwk_heap_fast_dancestor_index(unsigned int index)
{
	assert(index);

	return index >> (__builtin_ctz(index) + 1);
}

/*
 * Perform bottom-up construction of a weak heap from the content of "nodes".
 *
 * Nodes are visited one by one in reverse order, and the two weak heaps rooted
 * at a node and its distinguished ancestor are joined.
 */
static void fwk_heap_make(const struct farr *nodes,
                          const struct fbmp *rbits,
                          unsigned int       count,
                          farr_compare_fn   *compare,
                          farr_copy_fn      *copy,
                          bool               regular)
{
	while (--count)
		fwk_heap_join(nodes, rbits,
		              fwk_heap_fast_dancestor_index(count), count,
		              compare, copy, regular);
}

void fwk_heap_build(struct fwk_heap *heap, unsigned int count)
{
	fwk_heap_assert(heap);
	assert(count);

	heap->fwk_count = count;

	fbmp_clear_all(&heap->fwk_rbits, farr_nr(&heap->fwk_nodes));

	fwk_heap_make(&heap->fwk_nodes, &heap->fwk_rbits, count,
	              heap->fwk_compare, heap->fwk_copy,
	              FWK_HEAP_REGULAR_ORDER);
}

int fwk_heap_init(struct fwk_heap *heap,
                  char            *nodes,
                  size_t           node_size,
                  unsigned int     node_nr,
                  farr_compare_fn *compare,
                  farr_copy_fn    *copy)
{
	assert(heap);
	assert(compare);
	assert(copy);

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
	assert(heap);

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

		fwk_heap_make(&nodes, &rbits, entry_nr, compare, copy,
		              FWK_HEAP_REVERSE_ORDER);

		root = farr_slot(&nodes, FWK_HEAP_ROOT_INDEX);
		last = &entries[(entry_nr - 1) * entry_size];

		while (true) {
			copy(tmp, root);
			copy(root, last);
			copy(last, tmp);

			if (--entry_nr <= 1)
				break;

			fwk_heap_siftdown(&nodes, &rbits, entry_nr, compare,
			                  copy, FWK_HEAP_REVERSE_ORDER);

			last -= entry_size;
		}

		farr_fini(&nodes);
		fbmp_fini(&rbits);
	}

	return 0;
}

#endif /* defined(CONFIG_FWK_HEAP_SORT) */
