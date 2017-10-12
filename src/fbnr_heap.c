/**
 * @file      fbnr_heap.c
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

#include "fbnr_heap.h"
#include <string.h>

void fbnr_heap_insert(struct fbnr_heap *heap, const char *node)
{
	fbnr_heap_assert(heap);
	assert(!bstree_fixed_full(&heap->fbnr_tree));
	assert(node);

	char             *child;
	char             *parent;
	array_compare_fn *compare = heap->fbnr_compare;
	array_copy_fn    *copy = heap->fbnr_copy;

	/*
	 * Next free slot is located at the right of the deepest node in the
	 * heap.
	 */
	child = bstree_fixed_bottom(&heap->fbnr_tree);

	/* Start bubbling nodes up from the next free slot's parent node. */
	parent = bstree_fixed_parent(&heap->fbnr_tree, child);

	while (parent && compare(node, parent) <= 0) {
		/* Bubble current child up while out of order. */
		copy(child, parent);

		/* Go up one level. */
		child = parent;
		parent = bstree_fixed_parent(&heap->fbnr_tree, child);
	}

	/* Child now points to the location where to swap "node" node into. */
	copy(child, node);

	/* Update count of hosted nodes. */
	bstree_fixed_credit(&heap->fbnr_tree);
}

/*
 * Given parent node, return child node which is not in order with node passed
 * in argument.
 * Will return NULL if no child available or child if is in order.
 */
static char * fbnr_heap_unorder_child(const struct bstree_fixed *tree,
                                      const char                *parent,
                                      const char                *node,
                                      array_compare_fn          *compare)
{
	struct bstree_siblings  sibs;
	char                   *child;

	/* Fetch sibling, i.e. left and right children. */
	sibs = bstree_fixed_siblings(tree, parent);

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
static char * fbnr_heap_siftdown(const struct bstree_fixed *tree,
                                 char                      *parent,
                                 char                      *child,
                                 const char                *node,
                                 array_compare_fn          *compare,
                                 array_copy_fn             *copy)
{
	assert(parent);
	assert(child);
	assert(node);

	do {
		/* Bubble child up: child location is now free. */
		copy(parent, child);

		/* Iterate down. */
		parent = child;

		/*
		 * Detect wether both children are in order or not and select
		 * the appropriate one.
		 */
		child = fbnr_heap_unorder_child(tree, parent, node, compare);
	} while (child);

	/* Return location where to swap "node" node into. */
	return parent;
}

void fbnr_heap_extract(struct fbnr_heap *heap, char *node)
{
	fbnr_heap_assert(heap);
	assert(!bstree_fixed_empty(&heap->fbnr_tree));
	assert(node);

	struct bstree_fixed *tree = &heap->fbnr_tree;
	char                *parent, *child;
	const char          *last;
	array_compare_fn    *compare = heap->fbnr_compare;
	array_copy_fn       *copy = heap->fbnr_copy;

	/* Extract root and copy into node passed in argument. */
	parent = bstree_fixed_root(tree);

	copy(node, parent);

	/* Starting from root location, bubble nodes down while not in order. */
	last = bstree_fixed_last(tree);
	child = fbnr_heap_unorder_child(tree, parent, last, compare);
	if (child)
		parent = fbnr_heap_siftdown(tree, parent, child, last, compare,
		                            copy);

	/* Parent now points to location where to copy last node into. */
	copy(parent, last);

	/* Update count of present nodes. */
	bstree_fixed_debit(tree);
}

void fbnr_heap_build(struct fbnr_heap *heap, unsigned int count)
{
	fbnr_heap_assert(heap);
	assert(count);
	assert(count <= array_fixed_nr(&heap->fbnr_tree.bst_nodes));

	struct bstree_fixed *tree = &heap->fbnr_tree;
	unsigned int         n;
	array_compare_fn    *compare = heap->fbnr_compare;
	array_copy_fn       *copy = heap->fbnr_copy;

	/*
	 * Update count immediatly to prevent siftdown from complaining about
	 * empty heap.
	 */
	tree->bst_count = count;

	/*
	 * Starting from the lowest heap level and moving upwards, shift the
	 * root of each subtree downward as in the extraction algorithm until
	 * the heap property is restored.
	 */
	n = count / 2;
	while (n--) {
		char *node;
		char *child;

		node = bstree_fixed_node(tree, n);

		/*
		 * Since subtrees located under "node" node have already been
		 * heapified, the current subtree (located at "node" node) can
		 * be * heapified by sending its root down along the path of in
		 * ordered * children.
		 */
		child = fbnr_heap_unorder_child(tree, node, node, compare);
		if (child) {
			char tmp[tree->bst_nodes.arr_size];

			copy(tmp, node);

			node = fbnr_heap_siftdown(tree, node, child, tmp,
			                          compare, copy);
			copy(node, tmp);
		}
	}
}

void fbnr_heap_init(struct fbnr_heap *heap,
                    char             *nodes,
                    size_t            node_size,
                    unsigned int      node_nr,
                    array_compare_fn *compare,
                    array_copy_fn    *copy)
{
	assert(heap);
	assert(compare);
	assert(copy);

	heap->fbnr_compare = compare;
	heap->fbnr_copy = copy;

	bstree_init_fixed(&heap->fbnr_tree, nodes, node_size, node_nr);
}

void fbnr_heap_fini(struct fbnr_heap *heap __unused)
{
	assert(heap);

	bstree_fini_fixed(&heap->fbnr_tree);
}

struct fbnr_heap * fbnr_heap_create(size_t            node_size,
                                    unsigned int      node_nr,
                                    array_compare_fn *compare,
                                    array_copy_fn    *copy)
{
	assert(node_size);
	assert(node_nr);

	struct fbnr_heap *heap = malloc(sizeof(*heap) +
	                                  (node_size * node_nr));

	if (!heap)
		return NULL;

	fbnr_heap_init(heap, (char *)&heap[1], node_size, node_nr, compare,
	               copy);

	return heap;
}

void fbnr_heap_destroy(struct fbnr_heap *heap)
{
	fbnr_heap_fini(heap);

	free(heap);
}
