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

	char   *cnode;
	char   *pnode;
	size_t  nodesz = heap->node_size;

	cnode = bstree_fixed_bottom(&heap->bheap_tree, nodesz);
	pnode = bstree_fixed_parent(&heap->bheap_tree, nodesz, cnode);

	while (pnode && compare(node, pnode) <= 0) {
		memcpy(cnode, pnode, nodesz);

		cnode = pnode;
		pnode = bstree_fixed_parent(&heap->bheap_tree, nodesz, cnode);
	}

	memcpy(cnode, node, nodesz);

	bstree_fixed_credit(&heap->bheap_tree);
}

void bheap_extract_fixed(struct bheap_fixed *heap,
                         char               *node,
                         array_compare_fn   *compare)
{
	assert(heap);
	assert(heap->node_size);
	assert(!bstree_fixed_empty(&heap->bheap_tree));
	assert(node);
	assert(compare);

	size_t  nodesz = heap->node_size;
	char   *cnode = bstree_fixed_root(&heap->bheap_tree, nodesz);
	char   *pnode;

	memcpy(node, cnode, nodesz);

	node = bstree_fixed_last(&heap->bheap_tree, nodesz);

	while (true) {
		struct bstree_siblings  sibs;

		pnode = cnode;

		sibs = bstree_fixed_siblings(&heap->bheap_tree, nodesz, pnode);
		if (!(sibs.bst_left || sibs.bst_right))
			break;

		if (!sibs.bst_left)
			cnode = sibs.bst_right;
		else if (!sibs.bst_right)
			cnode = sibs.bst_left;
		else
			cnode = (compare(sibs.bst_left, sibs.bst_right) <= 0) ?
			        sibs.bst_left : sibs.bst_right;

		if (compare(node, cnode) <= 0)
			break;

		memcpy(pnode, cnode, nodesz);
	}

	memcpy(pnode, node, nodesz);

	bstree_fixed_debit(&heap->bheap_tree);
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
