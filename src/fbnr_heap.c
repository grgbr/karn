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

#include <karn/fbnr_heap.h>
#include <string.h>

#if defined(CONFIG_KARN_FBNR_HEAP_UTILS)

#define FBNR_HEAP_REGULAR_ORDER (true)
#define FBNR_HEAP_REVERSE_ORDER (false)

struct fbnr_heap_path {
	char         *fbnr_pnode;
	unsigned int  fbnr_cidx;
	char         *fbnr_cnode;
};

static void fbnr_heap_inorder_path(const struct fabs_tree *tree,
                                   struct fbnr_heap_path  *path,
                                   unsigned int            root_index,
                                   farr_compare_fn        *compare,
                                   bool                    regular)
{
	unsigned int ridx;

	path->fbnr_pnode = fabs_tree_node(tree, root_index);
	path->fbnr_cidx = fabs_tree_left_child_index(root_index);
	path->fbnr_cnode = fabs_tree_node(tree, path->fbnr_cidx);

	ridx = fabs_tree_right_child_index(root_index);
	if (ridx < fabs_tree_count(tree)) {
		char *right;

		right = fabs_tree_node(tree, ridx);
		if ((compare(right, path->fbnr_cnode) < 0) == regular) {
			path->fbnr_cidx = ridx;
			path->fbnr_cnode = right;
		}
	}
}

static char * fbnr_heap_topdwn_siftdown(const struct fabs_tree      *tree,
                                        const struct fbnr_heap_path *path,
                                        const char                  *node,
                                        farr_compare_fn             *compare,
                                        farr_copy_fn                *copy,
                                        bool                         regular)
{
	unsigned int  cnt = fabs_tree_count(tree);
	char         *empty = path->fbnr_pnode;
	char         *sibling = path->fbnr_cnode;
	unsigned int  sidx = path->fbnr_cidx;

	do {
		unsigned int  lidx;
		char         *left;

		copy(empty, sibling);
		empty = sibling;

		lidx = fabs_tree_left_child_index(sidx);
		if (lidx >= cnt)
			break;

		left = fabs_tree_node(tree, lidx);

		sidx = fabs_tree_right_child_index(sidx);
		if (sidx < cnt) {
			sibling = fabs_tree_node(tree, sidx);

			if ((compare(sibling, left) < 0) == regular)
				continue;
		}

		sidx = lidx;
		sibling = left;
	} while ((compare(sibling, node) < 0) == regular);

	return empty;
}

static void fbnr_heap_build_tree(struct fabs_tree *tree,
                                 unsigned int      count,
                                 farr_compare_fn  *compare,
                                 farr_copy_fn     *copy,
                                 bool              regular)
{
	karn_assert(count);
	karn_assert(count <= fabs_tree_nr(tree));

	unsigned int cnt = count / 2;

	/*
	 * Update count immediatly to prevent siftdown from complaining about
	 * empty heap.
	 */
	tree->fabs_count = count;

	/*
	 * Starting from the lowest heap level and moving upwards, shift the
	 * root of each subtree downward as in the extraction algorithm until
	 * the heap property is restored.
	 */
	while (cnt--) {
		struct fbnr_heap_path path;

		fbnr_heap_inorder_path(tree, &path, cnt, compare, regular);

		if ((compare(path.fbnr_cnode, path.fbnr_pnode) < 0) ==
		    regular) {
			char  tmp[fabs_tree_node_size(tree)];
			char *node;

			copy(tmp, path.fbnr_pnode);

			node = fbnr_heap_topdwn_siftdown(tree, &path, tmp,
			                                 compare, copy,
			                                 regular);

			copy(node, tmp);
		}
	}
}

#endif /* defined(CONFIG_KARN_FBNR_HEAP_UTILS) */

void fbnr_heap_insert(struct fbnr_heap *heap, const char *node)
{
	karn_assert(!fbnr_heap_full(heap));
	karn_assert(node);

	unsigned int     idx;
	farr_compare_fn *cmp = heap->fbnr_compare;
	farr_copy_fn    *cpy = heap->fbnr_copy;

	/*
	 * Next free slot is located at the right of the deepest node in the
	 * heap.
	 */
	idx = fabs_tree_bottom_index(&heap->fbnr_tree);
	while (idx != FABS_TREE_ROOT_INDEX) {
		/*
		 * Bubble next free slot up as long as node to insert is not
		 * heap ordered.
		 */
		unsigned int  pidx;
		const char   *pnode;

		pidx = fabs_tree_parent_index(idx);
		pnode = fabs_tree_node(&heap->fbnr_tree, pidx);
		if (cmp(pnode, node) <= 0)
			break;

		cpy(fabs_tree_node(&heap->fbnr_tree, idx), pnode);

		idx = pidx;
	}

	/* Child now points to the location where to swap "node" node into. */
	cpy(fabs_tree_node(&heap->fbnr_tree, idx), node);

	/* Update count of hosted nodes. */
	fabs_tree_credit(&heap->fbnr_tree);
}

void fbnr_heap_extract(struct fbnr_heap *heap, char *node)
{
	karn_assert(!fbnr_heap_empty(heap));
	karn_assert(node);

	heap->fbnr_copy(node, fabs_tree_root(&heap->fbnr_tree));

	if (fabs_tree_count(&heap->fbnr_tree) > 1) {
		struct fbnr_heap_path  path;
		const char            *last;

		fbnr_heap_inorder_path(&heap->fbnr_tree, &path,
		                       FABS_TREE_ROOT_INDEX,
		                       heap->fbnr_compare,
		                       FBNR_HEAP_REGULAR_ORDER);

		last = fabs_tree_last(&heap->fbnr_tree);

		if (heap->fbnr_compare(path.fbnr_cnode, last) < 0)
			node = fbnr_heap_topdwn_siftdown(&heap->fbnr_tree,
			                                 &path, last,
			                                 heap->fbnr_compare,
			                                 heap->fbnr_copy,
			                                 FBNR_HEAP_REGULAR_ORDER);
		else
			node = path.fbnr_pnode;

		heap->fbnr_copy(node, last);
	}

	/* Update count of present nodes. */
	fabs_tree_debit(&heap->fbnr_tree);
}

void fbnr_heap_build(struct fbnr_heap *heap, unsigned int count)
{
	fbnr_heap_assert(heap);

	fbnr_heap_build_tree(&heap->fbnr_tree, count, heap->fbnr_compare,
	                     heap->fbnr_copy, FBNR_HEAP_REGULAR_ORDER);
}

void fbnr_heap_init(struct fbnr_heap *heap,
                    char             *nodes,
                    size_t            node_size,
                    unsigned int      node_nr,
                    farr_compare_fn  *compare,
                    farr_copy_fn     *copy)
{
	karn_assert(heap);
	karn_assert(compare);
	karn_assert(copy);

	heap->fbnr_compare = compare;
	heap->fbnr_copy = copy;

	fabs_tree_init(&heap->fbnr_tree, nodes, node_size, node_nr);
}

void fbnr_heap_fini(struct fbnr_heap *heap __unused)
{
	karn_assert(heap);

	fabs_tree_fini(&heap->fbnr_tree);
}

struct fbnr_heap * fbnr_heap_create(size_t           node_size,
                                    unsigned int     node_nr,
                                    farr_compare_fn *compare,
                                    farr_copy_fn    *copy)
{
	karn_assert(node_size);
	karn_assert(node_nr);

	struct fbnr_heap *heap;

	heap = malloc(sizeof(*heap) + (node_size * node_nr));
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

#if defined(CONFIG_KARN_FBNR_HEAP_SORT)

static void fbnr_heap_botup_siftdown(const struct fabs_tree *tree,
                                     const char             *node,
                                     unsigned int            count,
                                     farr_compare_fn        *compare,
                                     farr_copy_fn           *copy)
{
	unsigned int  eidx = FABS_TREE_ROOT_INDEX;
	unsigned int  idx;
	char         *empty;
	unsigned int  depth;

	while (true) {
		idx = fabs_tree_left_child_index(eidx);
		if (idx >= count)
			break;

		eidx = fabs_tree_right_child_index(eidx);
		if (eidx >= count) {
			eidx = idx;
			break;
		}

		if (compare(fabs_tree_node(tree, eidx),
		            fabs_tree_node(tree, idx)) <= 0)
			eidx = idx;
	}

	while (true) {
		empty = fabs_tree_node(tree, eidx);
		if (compare(node, empty) <= 0)
			break;

		idx = fabs_tree_parent_index(eidx);
		if (idx == FABS_TREE_ROOT_INDEX)
			break;

		eidx = idx;
	}

	depth = fabs_tree_index_depth(eidx);
	while (depth--) {
		idx = fabs_tree_ancestor_index(eidx, depth);

		copy(fabs_tree_node(tree, fabs_tree_parent_index(idx)),
		     fabs_tree_node(tree, idx));
	}

	copy(empty, node);
}

void fbnr_heap_sort(char            *entries,
                    size_t           entry_size,
                    size_t           entry_nr,
                    farr_compare_fn *compare,
                    farr_copy_fn    *copy)
{
	if (entry_nr > 1) {
		struct fabs_tree  tree;
		char             *last;
		char              tmp[entry_size];

		fabs_tree_init(&tree, entries, entry_size, entry_nr);
		fbnr_heap_build_tree(&tree, entry_nr, compare, copy,
		                     FBNR_HEAP_REVERSE_ORDER);

		last = &entries[(entry_nr - 1) * entry_size];

		do {
			copy(tmp, fabs_tree_root(&tree));

			fbnr_heap_botup_siftdown(&tree, last, entry_nr, compare,
			                         copy);

			copy(last, tmp);

			last -= entry_size;
			entry_nr--;
		} while (entry_nr > 1);

		fabs_tree_fini(&tree);
	}
}

#endif /* defined(CONFIG_KARN_FBNR_HEAP_SORT) */
