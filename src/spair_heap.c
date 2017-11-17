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

#define SPAIR_HEAP_TAIL_NODE ((uintptr_t)1U)

static struct spair_heap_node *
spair_heap_tail_sibling(struct spair_heap_node *node)
{
	return (struct spair_heap_node *)((uintptr_t)node |
	                                  SPAIR_HEAP_TAIL_NODE);
}

static bool spair_heap_istail_sibling(const struct spair_heap_node *node)
{
	return !!((uintptr_t)node & SPAIR_HEAP_TAIL_NODE);
}

static struct spair_heap_node *
spair_heap_untail_sibling(const struct spair_heap_node *node)
{
	return (struct spair_heap_node *)((uintptr_t)node &
	                                  ~SPAIR_HEAP_TAIL_NODE);
}

static struct spair_heap_node * spair_heap_join(struct spair_heap_node *first,
                                                struct spair_heap_node *second,
                                                spair_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct spair_heap_node *parent;
	struct spair_heap_node *child;

	if (compare(first, second) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	if (parent->spair_youngest)
		child->spair_sibling = parent->spair_youngest;
	else
		child->spair_sibling = spair_heap_tail_sibling(parent);

	parent->spair_youngest = child;

	return parent;
}

void spair_heap_merge(struct spair_heap     *result,
                      struct spair_heap     *heap,
                      spair_heap_compare_fn *compare)
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

void spair_heap_insert(struct spair_heap      *heap,
                       struct spair_heap_node *node,
                       spair_heap_compare_fn  *compare)
{
	spair_heap_assert(heap);
	assert(node);
	assert(compare);

	node->spair_youngest = NULL;
	node->spair_sibling = spair_heap_tail_sibling(NULL);

	if (heap->spair_root)
		heap->spair_root = spair_heap_join(heap->spair_root, node,
		                                   compare);
	else
		heap->spair_root = node;

	heap->spair_count++;
}

static struct spair_heap_node *
spair_heap_merge_roots(struct spair_heap_node *roots,
                       spair_heap_compare_fn  *compare)
{
	assert(roots);
	assert(!spair_heap_istail_sibling(roots));
	assert(compare);

	struct spair_heap_node *curr = roots;
	struct spair_heap_node *head = spair_heap_tail_sibling(NULL);

	/* First pass pairing: move left to right merging pairs of trees. */
	do {
		struct spair_heap_node *nxt;
		bool                    tail;

		nxt = curr->spair_sibling;
		tail = spair_heap_istail_sibling(nxt);

		if (!tail) {
			struct spair_heap_node *tmp = nxt->spair_sibling;

			curr = spair_heap_join(curr, nxt, compare);

			curr->spair_sibling = head;
			head = curr;

			curr = tmp;
		}
		else {
			curr->spair_sibling = head;
			head = curr;
			break;
		}
	} while (!spair_heap_istail_sibling(curr));

	/*
	 * Second pass pairing: move right to left and merges the rightmost
	 * subtree with the remaining subtrees, one tree at a time.
	 */
	curr = head;
	head = head->spair_sibling;
	while (!spair_heap_istail_sibling(head)) {
		struct spair_heap_node *tmp = head->spair_sibling;

		curr = spair_heap_join(curr, head, compare);
		head = tmp;
	}

	return curr;
}

struct spair_heap_node * spair_heap_extract(struct spair_heap     *heap,
                                            spair_heap_compare_fn *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(compare);

	struct spair_heap_node *root = heap->spair_root;

	if (root->spair_youngest)
		heap->spair_root = spair_heap_merge_roots(root->spair_youngest,
		                                          compare);
	else
		heap->spair_root = NULL;

	heap->spair_count--;

	return root;
}

static void spair_heap_remove_node(struct spair_heap_node *node)
{
	assert(node);

	struct spair_heap_node *prev = node;
	struct spair_heap_node *parent;

	do {
		prev = prev->spair_sibling;
	} while (!spair_heap_istail_sibling(prev));

	parent = spair_heap_untail_sibling(prev);

	if (node != parent->spair_youngest) {
		prev = parent->spair_youngest;
		while (prev->spair_sibling != node) {
			assert(!spair_heap_istail_sibling(prev));
			prev = prev->spair_sibling;
		}

		prev->spair_sibling = node->spair_sibling;

		return;
	}

	if (!spair_heap_istail_sibling(node->spair_sibling))
		parent->spair_youngest = node->spair_sibling;
	else
		parent->spair_youngest = NULL;
}

void spair_heap_remove(struct spair_heap      *heap,
                       struct spair_heap_node *node,
                       spair_heap_compare_fn  *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(node);
	assert(compare);

	bool isroot = (node == heap->spair_root);

	heap->spair_count--;

	if (!node->spair_youngest) {
		if (!isroot)
			spair_heap_remove_node(node);
		else
			heap->spair_root = NULL;

		return;
	}

	if (!isroot) {
		spair_heap_remove_node(node);
		node = spair_heap_merge_roots(node->spair_youngest, compare);
		heap->spair_root = spair_heap_join(heap->spair_root, node,
		                                   compare);

		return;
	}

	heap->spair_root = spair_heap_merge_roots(node->spair_youngest,
	                                          compare);
}

void spair_heap_promote(struct spair_heap      *heap,
                        struct spair_heap_node *key,
                        spair_heap_compare_fn  *compare)
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

void spair_heap_demote(struct spair_heap      *heap,
                       struct spair_heap_node *key,
                       spair_heap_compare_fn  *compare)
{
	spair_heap_assert(heap);
	assert(heap->spair_count);
	assert(key);
	assert(compare);

	if (key != heap->spair_root) {
		spair_heap_remove_node(key);

		if (key->spair_youngest) {
			struct spair_heap_node *node;

			node = spair_heap_merge_roots(key->spair_youngest,
			                              compare);
			key->spair_youngest = NULL;
			key->spair_sibling = spair_heap_tail_sibling(NULL);
			key = spair_heap_join(node, key, compare);
		}
	}
	else {
		if (!key->spair_youngest)
			return;

		heap->spair_root = spair_heap_merge_roots(key->spair_youngest,
		                                          compare);
		key->spair_youngest = NULL;
	}

	heap->spair_root = spair_heap_join(heap->spair_root, key, compare);
}

void spair_heap_init(struct spair_heap *heap)
{
	heap->spair_root = NULL;
	heap->spair_count = 0;
}
