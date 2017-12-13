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

	if (compare(first, second) <= 0) {
		lcrs_join(second, first);

		return first;
	}

	lcrs_join(first, second);

	return second;
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

	result->spair_count += heap->spair_count;

	result->spair_root = spair_heap_join(result->spair_root,
	                                     heap->spair_root, compare);
}

void spair_heap_insert(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(key);
	assert(compare);

	heap->spair_count++;

	lcrs_init(key);

	if (!heap->spair_root) {
		heap->spair_root = key;

		return;
	}

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

static struct lcrs_node *
spair_heap_merge_roots(struct lcrs_node *roots, lcrs_compare_fn *compare)
{
	assert(roots);
	assert(!lcrs_istail(roots));
	assert(compare);

	struct lcrs_node *curr = roots;
	struct lcrs_node *tmp;
	struct lcrs_node *head = lcrs_mktail(NULL);

	/* First pass pairing: move left to right merging pairs of trees. */
	do {
		struct lcrs_node *nxt;

		nxt = curr->lcrs_sibling;
		if (lcrs_istail(nxt)) {
			curr->lcrs_sibling = head;
			head = curr;
			break;
		}

		tmp = nxt->lcrs_sibling;

		curr = spair_heap_join(curr, nxt, compare);

		curr->lcrs_sibling = head;
		head = curr;

		curr = tmp;
	} while (!lcrs_istail(curr));

	/*
	 * Second pass pairing: move right to left and merges the rightmost
	 * subtree with the remaining subtrees, one tree at a time.
	 */
	curr = head;
	head = head->lcrs_sibling;
	while (!lcrs_istail(head)) {
		tmp = head->lcrs_sibling;

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

	heap->spair_count--;

	if (!lcrs_has_child(root)) {
		heap->spair_root = NULL;

		return root;
	}

	heap->spair_root = spair_heap_merge_roots(lcrs_youngest(root), compare);

	return root;
}

static void spair_heap_remove_node(const struct lcrs_node *key)
{
	lcrs_split(key, &lcrs_parent(key)->lcrs_youngest);
}

void spair_heap_remove(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(key);
	assert(compare);

	bool isroot = (key == heap->spair_root);

	heap->spair_count--;

	if (!lcrs_has_child(key)) {
		if (!isroot)
			spair_heap_remove_node(key);
		else
			heap->spair_root = NULL;

		return;
	}

	if (!isroot) {
		spair_heap_remove_node(key);

		key = spair_heap_merge_roots(lcrs_youngest(key),
		                             compare);

		heap->spair_root = spair_heap_join(heap->spair_root, key,
		                                   compare);

		return;
	}

	heap->spair_root = spair_heap_merge_roots(lcrs_youngest(key),
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

		if (lcrs_has_child(key)) {
			struct lcrs_node *node;

			node = spair_heap_merge_roots(lcrs_youngest(key),
			                              compare);

			lcrs_init(key);
			key = spair_heap_join(node, key, compare);
		}
	}
	else {
		if (!lcrs_has_child(key))
			return;

		heap->spair_root = spair_heap_merge_roots(lcrs_youngest(key),
		                                          compare);
		key->lcrs_youngest =  lcrs_mktail(key);
	}

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

void spair_heap_init(struct spair_heap *heap)
{
	heap->spair_root = NULL;
	heap->spair_count = 0;
}
