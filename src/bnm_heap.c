/**
 * @file      bnm_heap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Binomial heap implementation
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

#include "bnm_heap.h"
#include <assert.h>

static struct bnm_heap_node *
bnm_heap_join_trees(struct bnm_heap_node *first,
                    struct bnm_heap_node *second,
                    bnm_heap_compare_fn  *compare)
{
	assert(first->bnm_order == second->bnm_order);

	struct bnm_heap_node *root;
	struct bnm_heap_node *child;

	if (compare(first, second) <= 0) {
		root = first;
		child = second;
	}
	else {
		root = second;
		child = first;
	}

	child->bnm_parent = root;
	child->bnm_sibling = root->bnm_eldest;

	root->bnm_eldest = child;
	root->bnm_order++;

	return root;
}

void bnm_heap_insert(struct bnm_heap      *heap,
                     struct bnm_heap_node *key,
                     bnm_heap_compare_fn  *compare)
{
	struct bnm_heap_node *cur = heap->bnm_trees;

	key->bnm_parent = NULL;
	key->bnm_eldest = NULL;
	key->bnm_sibling = NULL;
	key->bnm_order = 0;

	while (cur) {
		struct bnm_heap_node *nxt;

		if (key->bnm_order != cur->bnm_order)
			break;

		nxt = cur->bnm_sibling;
		key = bnm_heap_join_trees(key, cur, compare);

		cur = nxt;
	}

	key->bnm_sibling = cur;
	heap->bnm_trees = key;

	heap->bnm_count++;

	return;
}

static struct bnm_heap_node *
bnm_heap_merge_roots(struct bnm_heap_node **first,
                     struct bnm_heap_node **second,
                     bnm_heap_compare_fn   *compare)
{
	struct bnm_heap_node *fst = *first;
	struct bnm_heap_node *snd = *second;

	if (fst->bnm_order == snd->bnm_order) {
		*first = (*first)->bnm_sibling;
		*second = (*second)->bnm_sibling;

		return bnm_heap_join_trees(fst, snd, compare);
	}
	else if (fst->bnm_order < snd->bnm_order) {
		*first = (*first)->bnm_sibling;
		return fst;
	}
	else {
		*second = (*second)->bnm_sibling;
		return snd;
	}
}

static struct bnm_heap_node *
bnm_heap_merge_trees(struct bnm_heap_node *first,
                     struct bnm_heap_node *second,
                     bnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct bnm_heap_node  *head, *tail;
	struct bnm_heap_node **prev;
	struct bnm_heap_node  *tmp;


	head = bnm_heap_merge_roots(&first, &second, compare);
	prev = &head;
	tail = head;

	while (first && second) {
		tmp = bnm_heap_merge_roots(&first, &second, compare);

		assert(tail->bnm_order <= tmp->bnm_order);

		if (tail->bnm_order == tmp->bnm_order) {
			*prev = bnm_heap_join_trees(tail, tmp, compare);
			tail = *prev;
		}
		else {
			prev = &tail->bnm_sibling;
			tail->bnm_sibling = tmp;
			tail = tmp;
		}
	}

	if (!first)
		first = second;

	while (first && (tail->bnm_order == first->bnm_order)) {
		assert(tail->bnm_order <= first->bnm_order);

		tmp = first->bnm_sibling;
		*prev = bnm_heap_join_trees(tail, first, compare);
		tail = *prev;
		first = tmp;
	}

	tail->bnm_sibling = first;

	return head;
}

struct bnm_heap_node *
bnm_heap_peek(const struct bnm_heap *heap, bnm_heap_compare_fn *compare)
{
	/* TODO: optimize by always keeping a pointer to minimum root. */
	assert(heap->bnm_trees);

	struct bnm_heap_node *key = heap->bnm_trees;
	struct bnm_heap_node *root = key->bnm_sibling;

	while (root) {
		if (compare(root, key) < 0)
			key = root;

		root = root->bnm_sibling;
	}

	return key;
}

struct bnm_heap_node *
bnm_heap_extract(struct bnm_heap *heap, bnm_heap_compare_fn *compare)
{
	assert(heap);
	assert(heap->bnm_trees);
	assert(heap->bnm_count);
	assert(compare);

	struct bnm_heap_node *key = heap->bnm_trees;
	struct bnm_heap_node *key_prev = NULL;
	struct bnm_heap_node *node = key;
	struct bnm_heap_node *root = key->bnm_sibling;

	while (root) {
		if (compare(root, key) < 0) {
			key_prev = node;
			key = root;
		}

		node = root;
		root = root->bnm_sibling;
	}

	if (key_prev)
		key_prev->bnm_sibling = key->bnm_sibling;
	else
		heap->bnm_trees = key->bnm_sibling;

	node = key->bnm_eldest;
	if (node) {
		do {
			struct bnm_heap_node *nxt = node->bnm_sibling;

			node->bnm_parent = NULL;
			node->bnm_sibling = root;
			root = node;

			node = nxt;
		} while (node);

		if (heap->bnm_trees)
			heap->bnm_trees = bnm_heap_merge_trees(heap->bnm_trees,
			                                       root, compare);
		else
			heap->bnm_trees = root;
	}

	heap->bnm_count--;

	return key;
}

void bnm_heap_merge(struct bnm_heap     *result,
                    struct bnm_heap     *source,
                    bnm_heap_compare_fn *compare)
{
	assert(result);
	assert(result->bnm_trees);
	assert(result->bnm_count);
	assert(source);
	assert(source->bnm_trees);
	assert(source->bnm_count);
	assert(compare);

	result->bnm_trees = bnm_heap_merge_trees(result->bnm_trees,
	                                         source->bnm_trees, compare);

	result->bnm_count += source->bnm_count;
}
