/**
 * @file      bheap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary heap implementation
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

#include "bheap.h"
#include <string.h>

void bheap_insert_fixed(struct bheap_fixed *heap,
                        const char         *node,
                        array_compare_fn   *compare)
{
	assert(heap);
	assert(heap->node_size);
	assert(!bstree_fixed_full(&heap->bheap_tree));
	assert(node);
	assert(compare);

	char   *child;
	char   *parent;
	size_t  nodesz = heap->node_size;

	/*
	 * Next free slot is located at the right of the deepest node in the heap.
	 */
	child = bstree_fixed_bottom(&heap->bheap_tree, nodesz);

	/* Start bubbling nodes up from the next free slot's parent node. */
	parent = bstree_fixed_parent(&heap->bheap_tree, nodesz, child);

	while (parent && compare(node, parent) <= 0) {
		/* Bubble current child up while out of order. */
		memcpy(child, parent, nodesz);

		/* Go up one level. */
		child = parent;
		parent = bstree_fixed_parent(&heap->bheap_tree, nodesz, child);
	}

	/* Child now points to the location where to swap "node" node into. */
	memcpy(child, node, nodesz);

	/* Update count of present nodes. */
	bstree_fixed_credit(&heap->bheap_tree);
}

/*
 * Given parent node, return child node which is not in order with node passed
 * in argument.
 * Will return NULL if no child available or child if is in order.
 */
static char * bheap_fixed_unorder_child(const struct bstree_fixed *tree,
                                        const char                *parent,
                                        const char                *node,
                                        size_t                     node_size,
                                        array_compare_fn          *compare)
{
		struct bstree_siblings  sibs;
		char                   *child;

		/* Fetch sibling, i.e. left and right children. */
		sibs = bstree_fixed_siblings(tree, node_size, parent);

		if (!sibs.bst_left)
			/* No left child. */
			child = sibs.bst_right;
		else if (!sibs.bst_right)
			/* No right child. */
			child = sibs.bst_left;
		else if (compare(sibs.bst_left, sibs.bst_right) <= 0)
			/* First in order child is the left one. */
			child = sibs.bst_left;
		else
			/* First in order child is the right one. */
			child = sibs.bst_right;

		if (!child ||                    /* No child at all. */
		    (compare(node, child) <= 0)) /* Child is in order. */
			return NULL;

		return child;
}

/*
 * Find location where to swap "node" node into by iterating down the
 * underlying binary search tree starting from "parent" node along
 * "child" node direction.
 * Precondition: "child" node is out of order with respect to "node".
 */
static char * bheap_siftdown_fixed(const struct bstree_fixed *tree,
                                   char                      *parent,
                                   char                      *child,
                                   const char                *node,
                                   size_t                     node_size,
                                   array_compare_fn          *compare)
{
	assert(parent);
	assert(child);
	assert(node);

	do {
		/* Bubble child up: child location is now free. */
		memcpy(parent, child, node_size);

		/* Iterate down. */
		parent = child;

		/*
		 * Detect wether both children are in order or not and select the
		 * appropriate one.
		 */
		child = bheap_fixed_unorder_child(tree, parent, node, node_size,
		                                  compare);
	} while (child);

	/* Return location where to swap "node" node into. */
	return parent;
}

void bheap_extract_fixed(struct bheap_fixed *heap,
                         char               *node,
                         array_compare_fn   *compare)
{
	assert(heap);
	assert(!bstree_fixed_empty(&heap->bheap_tree));
	assert(node);
	assert(compare);

	size_t               nodesz = heap->node_size;
	struct bstree_fixed *tree = &heap->bheap_tree;
	char                *parent, *child;
	const char          *last = bstree_fixed_last(tree, nodesz);

	/* Extract root and copy into node passed in argument. */
	parent = bstree_fixed_root(tree, nodesz);
	memcpy(node, parent, nodesz);

	/* Starting from root location, bubble nodes down while not in order. */
	last = bstree_fixed_last(tree, nodesz);
	child = bheap_fixed_unorder_child(tree, parent, last, nodesz, compare);
	if (child)
		parent = bheap_siftdown_fixed(tree, parent, child, last, nodesz,
		                              compare);

	/* Parent now points to location where to copy last (deepest) node into. */
	memcpy(parent, last, nodesz);

	/* Update count of present nodes. */
	bstree_fixed_debit(tree);
}

void bheap_build_fixed(struct bheap_fixed *heap,
                       unsigned int        count,
                       array_compare_fn   *compare)
{
	assert(heap);
	assert(count);
	assert(count <= array_fixed_nr(&heap->bheap_tree.bst_nodes));
	assert(compare);

	unsigned int         n;
	size_t               nodesz = heap->node_size;
	struct bstree_fixed *tree = &heap->bheap_tree;

	/*
	 * Update count immediatly to prevent siftdown from complaining about
	 * empty heap.
	 */
	tree->bst_count = count;

	/*
	 * Starting from the lowest heap level and moving upwards, shift the root
	 * of each subtree downward as in the extraction algorithm until the heap
	 * property is restored.
	 */
	n = count / 2;
	while (n--) {
		char *node;
		char *child;

		node = bstree_fixed_node(tree, nodesz, n);

		/*
		 * Since subtrees located under "node" node have already been
		 * heapified, the current subtree (located at "node" node) can be
		 * heapified by sending its root down along the path of in ordered
		 * children.
		 */
		child = bheap_fixed_unorder_child(tree, node, node, nodesz, compare);
		if (child) {
			char tmp[nodesz];

			memcpy(tmp, node, nodesz);
			node = bheap_siftdown_fixed(tree, node, child, tmp, nodesz,
			                            compare);
			memcpy(node, tmp, nodesz);
		}
	}
}

void bheap_init_fixed(struct bheap_fixed *heap,
                      char               *nodes,
                      size_t              node_size,
                      unsigned int        nr)
{
	assert(node_size);

	bstree_init_fixed(&heap->bheap_tree, nodes, nr);

	heap->node_size = node_size;
}

void bheap_fini_fixed(struct bheap_fixed *heap __unused)
{
	assert(heap);
	assert(heap->node_size);

	bstree_fini_fixed(&heap->bheap_tree);
}

struct bheap_fixed * bheap_create_fixed(size_t node_size, unsigned int nr)
{
	assert(node_size);
	assert(nr);

	struct bheap_fixed *heap = malloc(sizeof(*heap) + (node_size * nr));

	if (!heap)
		return NULL;

	bheap_init_fixed(heap, (char *)&heap[1], node_size, nr);

	return heap;
}

void bheap_destroy_fixed(struct bheap_fixed *heap)
{
	bheap_fini_fixed(heap);

	free(heap);
}
