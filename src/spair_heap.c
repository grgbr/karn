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

#include <karn/spair_heap.h>

static struct lcrs_node * spair_heap_join(struct lcrs_node *first,
                                          struct lcrs_node *second,
                                          lcrs_compare_fn  *compare)
{
	karn_assert(first);
	karn_assert(second);
	karn_assert(compare);

	if (compare(first, second) <= 0) {
		lcrs_join(second, first);

		return first;
	}

	lcrs_join(first, second);

	return second;
}

static struct lcrs_node *
spair_heap_merge_roots(struct lcrs_node *roots, lcrs_compare_fn *compare)
{
	karn_assert(roots);
	karn_assert(!lcrs_istail(roots));
	karn_assert(compare);

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

static struct lcrs_node *
spair_heap_remove_key(struct lcrs_node *root,
                      struct lcrs_node *key,
                      bool              isroot,
                      lcrs_compare_fn  *compare)
{
	karn_assert(root);
	karn_assert(key);
	karn_assert(compare);

	if (!lcrs_has_child(key)) {
		if (!isroot) {
			lcrs_split(key, lcrs_youngest_ref(lcrs_parent(key)));

			return root;
		}

		return NULL;
	}

	if (!isroot) {
		lcrs_split(key, lcrs_youngest_ref(lcrs_parent(key)));

		key = spair_heap_merge_roots(lcrs_youngest(key), compare);

		return spair_heap_join(root, key, compare);
	}

	return spair_heap_merge_roots(lcrs_youngest(key), compare);
}

static void
spair_heap_update_key(struct spair_heap *heap,
                      struct lcrs_node  *key,
                      bool               isroot,
                      lcrs_compare_fn   *compare)
{
	struct lcrs_node *node;

	node = spair_heap_remove_key(heap->spair_root, key, isroot, compare);

	lcrs_init(key);
	heap->spair_root = spair_heap_join(node, key, compare);
}

void spair_heap_merge(struct spair_heap *result,
                      struct spair_heap *source,
                      lcrs_compare_fn   *compare)
{
	spair_heap_assert(result);
	karn_assert(result->spair_count);
	spair_heap_assert(source);
	karn_assert(source->spair_count);
	karn_assert(compare);

	result->spair_count += source->spair_count;

	result->spair_root = spair_heap_join(result->spair_root,
	                                     source->spair_root, compare);
}

void spair_heap_insert(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	karn_assert(key);
	karn_assert(compare);

	heap->spair_count++;

	lcrs_init(key);

	if (!heap->spair_root) {
		heap->spair_root = key;

		return;
	}

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

struct lcrs_node * spair_heap_extract(struct spair_heap *heap,
                                      lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	karn_assert(heap->spair_count);
	karn_assert(compare);

	struct lcrs_node *root = heap->spair_root;

	heap->spair_count--;

	if (!lcrs_has_child(root)) {
		heap->spair_root = NULL;

		return root;
	}

	heap->spair_root = spair_heap_merge_roots(lcrs_youngest(root), compare);

	return root;
}

void spair_heap_remove(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	karn_assert(heap->spair_count);
	karn_assert(key);
	karn_assert(compare);

	heap->spair_count--;

	heap->spair_root = spair_heap_remove_key(heap->spair_root, key,
	                                         key == heap->spair_root,
	                                         compare);
}

void spair_heap_promote(struct spair_heap *heap,
                        struct lcrs_node  *key,
                        lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	karn_assert(heap->spair_count);
	karn_assert(key);
	karn_assert(compare);

	bool isroot = (key == heap->spair_root);

	if (isroot)
		return;

	if (compare(lcrs_parent(key), key) <= 0)
		return;

	spair_heap_update_key(heap, key, isroot, compare);
}

void spair_heap_demote(struct spair_heap *heap,
                       struct lcrs_node  *key,
                       lcrs_compare_fn   *compare)
{
	spair_heap_assert(heap);
	karn_assert(heap->spair_count);
	karn_assert(key);
	karn_assert(compare);

	if (heap->spair_count == 1)
		return;

	spair_heap_update_key(heap, key, key == heap->spair_root, compare);
}

void spair_heap_init(struct spair_heap *heap)
{
	heap->spair_root = NULL;
	heap->spair_count = 0;
}
