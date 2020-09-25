/**
 * @file      sbnm_heap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based binomial heap implementation
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

#include <karn/sbnm_heap.h>

static struct sbnm_heap_node *
sbnm_heap_parent_node(const struct sbnm_heap_node *node)
{
	const struct lcrs_node *parent;

	parent = lcrs_parent(&node->sbnm_lcrs);
	if (!parent)
		return NULL;

	return sbnm_heap_node_from_lcrs(parent);
}

static struct sbnm_heap_node *
sbnm_heap_next_node(const struct sbnm_heap_node *node)
{
	const struct lcrs_node *nxt;

	nxt = lcrs_next(&node->sbnm_lcrs);
	if (lcrs_istail(nxt))
		return NULL;

	return sbnm_heap_node_from_lcrs(nxt);
}

static struct sbnm_heap_node *
sbnm_heap_join(struct sbnm_heap_node *first,
               struct sbnm_heap_node *second,
               sbnm_heap_compare_fn  *compare)
{
	karn_assert(first);
	karn_assert(second);
	karn_assert(first != second);
	karn_assert(first->sbnm_rank == second->sbnm_rank);
	karn_assert(compare);

	struct sbnm_heap_node *parent;
	struct sbnm_heap_node *child;

	if (compare(first, second) <= 0) {
		parent = first;
		child = second;
	}
	else {
		parent = second;
		child = first;
	}

	lcrs_join(&child->sbnm_lcrs, &parent->sbnm_lcrs);

	parent->sbnm_rank++;

	return parent;
}

static void
sbnm_heap_swap(struct sbnm_heap_node *node, struct sbnm_heap_node *child)
{
	unsigned int rank = node->sbnm_rank;

	lcrs_swap_down(&node->sbnm_lcrs, &child->sbnm_lcrs);

	node->sbnm_rank = child->sbnm_rank;
	child->sbnm_rank = rank;
}

struct sbnm_heap_node *
sbnm_heap_peek(const struct sbnm_heap *heap)
{
	sbnm_heap_assert(heap);
	karn_assert(heap->sbnm_count);

	const struct sbnm_heap_node *key;
	const struct sbnm_heap_node *curr;

	key = sbnm_heap_node_from_lcrs(heap->sbnm_roots);
	curr = key;

	while (true) {
		const struct sbnm_heap_node *nxt;

		nxt = sbnm_heap_next_node(curr);
		if (!nxt)
			break;

		if (heap->sbnm_compare(nxt, key) < 0)
			key = nxt;

		curr = nxt;
	}

	return (struct sbnm_heap_node *)key;
}

static inline struct lcrs_node *
sbnm_heap_1way_merge_roots(struct lcrs_node     *node,
                           struct lcrs_node     *siblings,
                           sbnm_heap_compare_fn *compare)
{
	karn_assert(node);
	karn_assert(siblings);
	karn_assert(compare);

	struct sbnm_heap_node *youngest = sbnm_heap_node_from_lcrs(node);

	while (!lcrs_istail(siblings)) {
		struct sbnm_heap_node *curr;

		curr = sbnm_heap_node_from_lcrs(siblings);
		if (youngest->sbnm_rank != curr->sbnm_rank)
			break;

		node = lcrs_next(siblings);

		youngest = sbnm_heap_join(youngest, curr, compare);

		siblings = node;
	}

	lcrs_assign_next(&youngest->sbnm_lcrs, siblings);

	return &youngest->sbnm_lcrs;
}

static inline struct sbnm_heap_node *
sbnm_heap_2way_merge_roots(struct lcrs_node     **first,
                           struct lcrs_node     **second,
                           sbnm_heap_compare_fn  *compare)
{
	karn_assert(first);
	karn_assert(*first);
	karn_assert(second);
	karn_assert(*second);
	karn_assert(first != second);
	karn_assert(*first != *second);
	karn_assert(compare);

	struct sbnm_heap_node *fst = sbnm_heap_node_from_lcrs(*first);
	struct sbnm_heap_node *snd = sbnm_heap_node_from_lcrs(*second);

	if (fst->sbnm_rank == snd->sbnm_rank) {
		*first = lcrs_next(*first);
		*second = lcrs_next(*second);

		return sbnm_heap_join(fst, snd, compare);
	}
	else if (fst->sbnm_rank < snd->sbnm_rank) {
		*first = lcrs_next(*first);

		return fst;
	}
	else {
		*second = lcrs_next(*second);

		return snd;
	}
}

static struct lcrs_node *
sbnm_heap_merge_roots(struct lcrs_node     *first,
                      struct lcrs_node     *second,
                      sbnm_heap_compare_fn *compare)
{
	karn_assert(first);
	karn_assert(!lcrs_istail(first));
	karn_assert(second);
	karn_assert(!lcrs_istail(second));
	karn_assert(first != second);
	karn_assert(compare);

	struct lcrs_node  *res;
	struct lcrs_node **tail = &res;

	res = &sbnm_heap_2way_merge_roots(&first, &second, compare)->sbnm_lcrs;

	while (!(lcrs_istail(first) || lcrs_istail(second))) {
		struct sbnm_heap_node *tmp;
		struct sbnm_heap_node *last;

		tmp = sbnm_heap_2way_merge_roots(&first, &second, compare);

		karn_assert(!lcrs_istail(*tail));
		last = sbnm_heap_node_from_lcrs(*tail);
		karn_assert(last->sbnm_rank <= tmp->sbnm_rank);

		if (last->sbnm_rank != tmp->sbnm_rank)
			tail = lcrs_next_ref(*tail);
		else
			tmp = sbnm_heap_join(last, tmp, compare);

		*tail = &tmp->sbnm_lcrs;
	}

	if (lcrs_istail(first))
		first = second;

	*tail = sbnm_heap_1way_merge_roots(*tail, first, compare);

	return res;
}


static void
sbnm_heap_remove_root(struct sbnm_heap  *heap,
                      struct lcrs_node **previous,
                      struct lcrs_node  *root)
{
	karn_assert(heap);
	karn_assert(previous);
	karn_assert(root);

	struct lcrs_node *trees;

	heap->sbnm_count--;

	*previous = lcrs_next(root);

	root = lcrs_youngest(root);
	trees = lcrs_mktail(NULL);
	while (!lcrs_istail(root)) {
		struct lcrs_node *nxt = lcrs_next(root);

		lcrs_assign_next(root, trees);
		trees = root;

		root = nxt;
	}

	if (lcrs_istail(heap->sbnm_roots)) {
		heap->sbnm_roots = trees;

		return;
	}

	if (lcrs_istail(trees))
		return;

	heap->sbnm_roots = sbnm_heap_merge_roots(heap->sbnm_roots, trees,
	                                         heap->sbnm_compare);
}

static void
sbnm_heap_remove_key(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_assert(heap);
	karn_assert(heap->sbnm_count);

	struct lcrs_node *root = &key->sbnm_lcrs;
	
	while (true) {
		struct sbnm_heap_node *parent;

		parent = sbnm_heap_parent_node(key);
		if (!parent)
			break;

		root = &parent->sbnm_lcrs;

		sbnm_heap_swap(parent, key);
	}

	sbnm_heap_remove_root(heap, lcrs_previous_ref(root, &heap->sbnm_roots),
	                      &key->sbnm_lcrs);
}

void
sbnm_heap_insert(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_assert(heap);
	karn_assert(key);

	lcrs_init(&key->sbnm_lcrs);
	key->sbnm_rank = 0;

	heap->sbnm_count++;

	heap->sbnm_roots = sbnm_heap_1way_merge_roots(&key->sbnm_lcrs,
	                                              heap->sbnm_roots,
	                                              heap->sbnm_compare);
}

struct sbnm_heap_node *
sbnm_heap_extract(struct sbnm_heap *heap)
{
	sbnm_heap_assert(heap);
	karn_assert(heap->sbnm_count);

	struct lcrs_node      **prev = &heap->sbnm_roots;
	struct sbnm_heap_node  *key;
	struct sbnm_heap_node  *curr;

	key = sbnm_heap_node_from_lcrs(*prev);
	curr = key;

	while (true) {
		struct sbnm_heap_node *nxt;

		nxt = sbnm_heap_next_node(curr);
		if (!nxt)
			break;

		if (heap->sbnm_compare(nxt, key) < 0) {
			prev = lcrs_next_ref(&curr->sbnm_lcrs);
			key = nxt;
		}

		curr = nxt;
	}

	sbnm_heap_remove_root(heap, prev, &key->sbnm_lcrs);

	return key;
}

void
sbnm_heap_remove(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_remove_key(heap, key);
}

void
sbnm_heap_promote(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_assert(heap);
	karn_assert(heap->sbnm_count);

	sbnm_heap_compare_fn  *cmp = heap->sbnm_compare;
	struct sbnm_heap_node *parent;
	struct lcrs_node      *root = NULL;

	parent = sbnm_heap_parent_node(key);
	while (parent && (cmp(parent, key) > 0)) {
		root = &parent->sbnm_lcrs;
		sbnm_heap_swap(parent, key);

		parent = sbnm_heap_parent_node(key);
	}

	if (!parent && root)
		*lcrs_previous_ref(root, &heap->sbnm_roots) = &key->sbnm_lcrs;
}

void
sbnm_heap_demote(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_remove_key(heap, key);

	sbnm_heap_insert(heap, key);
}

void
sbnm_heap_merge(struct sbnm_heap *result, struct sbnm_heap *source)
{
	sbnm_heap_assert(result);
	karn_assert(result->sbnm_count);
	sbnm_heap_assert(source);
	karn_assert(source->sbnm_count);
	karn_assert(result->sbnm_compare == source->sbnm_compare);

	result->sbnm_count += source->sbnm_count;

	result->sbnm_roots = sbnm_heap_merge_roots(result->sbnm_roots,
	                                           source->sbnm_roots,
	                                           result->sbnm_compare);
}
