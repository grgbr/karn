/**
 * @file      spair_heap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      15 Nov 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based pairing heap implementation
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

#include "spair_heap.h"

static struct lcrs_node * spair_heap_join(struct lcrs_node *first,
                                          struct lcrs_node *second,
                                          lcrs_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct lcrs_node *parent;
	struct lcrs_node *child;

	if (compare(first, second) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	lcrs_join_tree(child, parent);

	return parent;
}

void spair_heap_merge(struct spair_heap *result,
                      struct spair_heap *heap,
                      lcrs_compare_fn   *compare)
{
	spair_heap_assert(result);
	assert(result->spair_count);
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(compare);

	result->spair_root = spair_heap_join(result->spair_root,
	                                     heap->spair_root, compare);

	result->spair_count += heap->spair_count;
}

void spair_heap_insert(struct spair_heap *heap,
                       struct lcrs_node  *node,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(node);
	assert(compare);

	lcrs_init_node(node);

	if (heap->spair_root)
		heap->spair_root = spair_heap_join(heap->spair_root, node,
		                                   compare);
	else
		heap->spair_root = node;

	heap->spair_count++;
}

static struct lcrs_node *
spair_heap_merge_roots(struct lcrs_node *roots, lcrs_compare_fn *compare)
{
	assert(roots);
	assert(!lcrs_istail_node(roots));
	assert(compare);

	struct lcrs_node *curr = roots;
	struct lcrs_node *head = lcrs_mktail_node(NULL);

	/* First pass pairing: move left to right merging pairs of trees. */
	do {
		struct lcrs_node *nxt;

		nxt = curr->lcrs_sibling;

		if (!lcrs_istail_node(nxt)) {
			struct lcrs_node *tmp = nxt->lcrs_sibling;

			curr = spair_heap_join(curr, nxt, compare);

			curr->lcrs_sibling = head;
			head = curr;

			curr = tmp;
		}
		else {
			curr->lcrs_sibling = head;
			head = curr;
			break;
		}
	} while (!lcrs_istail_node(curr));

	/*
	 * Second pass pairing: move right to left and merges the rightmost
	 * subtree with the remaining subtrees, one tree at a time.
	 */
	curr = head;
	head = head->lcrs_sibling;
	while (!lcrs_istail_node(head)) {
		struct lcrs_node *tmp = head->lcrs_sibling;

		curr = spair_heap_join(curr, head, compare);
		head = tmp;
	}

	return curr;
}

struct lcrs_node * spair_heap_extract(struct spair_heap *heap,
                                      lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(compare);

	struct lcrs_node *root = heap->spair_root;

	if (lcrs_node_has_child(root))
		heap->spair_root =
			spair_heap_merge_roots(lcrs_youngest_sibling(root),
			                       compare);
	else
		heap->spair_root = NULL;

	heap->spair_count--;

	return root;
}

static void spair_heap_remove_node(const struct lcrs_node *node)
{
	lcrs_split_tree(node, lcrs_parent_node(node));
}

void spair_heap_remove(struct spair_heap *heap,
                       struct lcrs_node  *node,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(node);
	assert(compare);

	bool isroot = (node == heap->spair_root);

	heap->spair_count--;

	if (!lcrs_node_has_child(node)) {
		if (!isroot)
			spair_heap_remove_node(node);
		else
			heap->spair_root = NULL;

		return;
	}

	if (!isroot) {
		spair_heap_remove_node(node);

		node = spair_heap_merge_roots(lcrs_youngest_sibling(node),
		                              compare);

		heap->spair_root = spair_heap_join(heap->spair_root, node,
		                                   compare);

		return;
	}

	heap->spair_root = spair_heap_merge_roots(lcrs_youngest_sibling(node),
	                                          compare);
}

void spair_heap_promote(struct spair_heap *heap,
                        struct lcrs_node  *key,
                        lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(key);
	assert(compare);

	if (key == heap->spair_root)
		return;

	spair_heap_remove_node(key);

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

void spair_heap_demote(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(key);
	assert(compare);

	if (key != heap->spair_root) {
		spair_heap_remove_node(key);

		if (lcrs_node_has_child(key)) {
			struct lcrs_node *node;

			node = spair_heap_merge_roots(lcrs_youngest_sibling(key),
			                              compare);
			key->lcrs_youngest = NULL;
			key->lcrs_sibling = lcrs_mktail_node(NULL);
			key = spair_heap_join(node, key, compare);
		}
	}
	else {
		if (!lcrs_node_has_child(key))
			return;

		heap->spair_root = spair_heap_merge_roots(lcrs_youngest_sibling(key),
		                                          compare);
		key->lcrs_youngest = NULL;
	}

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

void spair_heap_init(struct spair_heap *heap)
{
	heap->spair_root = NULL;
	heap->spair_count = 0;
}
