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

	child = bstree_fixed_bottom(&heap->bheap_tree, nodesz);
	parent = bstree_fixed_parent(&heap->bheap_tree, nodesz, child);

	while (parent && compare(node, parent) <= 0) {
		memcpy(child, parent, nodesz);

		child = parent;
		parent = bstree_fixed_parent(&heap->bheap_tree, nodesz, child);
	}

	memcpy(child, node, nodesz);

	bstree_fixed_credit(&heap->bheap_tree);
}

static char * bheap_fixed_unorder_child(const struct bstree_fixed *tree,
                                        const char                *parent,
                                        const char                *node,
                                        size_t                     node_size,
                                        array_compare_fn          *compare)
{
		struct bstree_siblings  sibs;
		char                   *child;

		sibs = bstree_fixed_siblings(tree, node_size, parent);

		if (!sibs.bst_left)
			child = sibs.bst_right;
		else if (!sibs.bst_right)
			child = sibs.bst_left;
		else if (compare(sibs.bst_left, sibs.bst_right) <= 0)
			child = sibs.bst_left;
		else
			child = sibs.bst_right;

		if (!child || (compare(node, child) <= 0))
			return NULL;

		return child;
}

static char * bheap_siftdown_fixed(const struct bstree_fixed *tree,
                                   char                      *parent,
                                   char                      *child,
                                   const char                *node,
                                   size_t                     node_size,
                                   array_compare_fn          *compare)
{
	do {
		memcpy(parent, child, node_size);

		parent = child;
		child = bheap_fixed_unorder_child(tree, parent, node, node_size,
		                                  compare);
	} while (child);

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

	parent = bstree_fixed_root(tree, nodesz);
	memcpy(node, parent, nodesz);

	last = bstree_fixed_last(tree, nodesz);
	child = bheap_fixed_unorder_child(tree, parent, last, nodesz, compare);
	if (child)
		parent = bheap_siftdown_fixed(tree, parent, child, last, nodesz,
		                              compare);

	memcpy(parent, last, nodesz);

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

	n = count / 2;
	while (n--) {
		char *node;
		char *child;

		node = bstree_fixed_node(tree, nodesz, n);

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
