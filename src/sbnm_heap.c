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

#include "sbnm_heap.h"
#include <assert.h>

static struct sbnm_heap_node *
sbnm_heap_parent_node(const struct sbnm_heap      *heap,
                      const struct sbnm_heap_node *node)
{
	struct lcrs_node *parent;

	parent = lcrs_parent(&node->sbnm_lcrs);
	if (parent == &heap->sbnm_dummy)
		return NULL;

	return sbnm_heap_node_from_lcrs(parent);
}

static struct sbnm_heap_node *
sbnm_heap_next_node(const struct sbnm_heap_node *node)
{
	struct lcrs_node *nxt;

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
	assert(first);
	assert(second);
	assert(first != second);
	assert(first->sbnm_rank == second->sbnm_rank);
	assert(compare);

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
	assert(heap->sbnm_count);

	const struct sbnm_heap_node *key;
	const struct sbnm_heap_node *curr;

	key = sbnm_heap_node_from_lcrs(lcrs_youngest(&heap->sbnm_dummy));
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

static struct lcrs_node *
sbnm_heap_1way_merge_roots(struct lcrs_node     *root,
                           struct lcrs_node     *siblings,
                           sbnm_heap_compare_fn *compare)
{
	assert(root);
	assert(siblings);
	assert(compare);

	struct sbnm_heap_node *rnode = sbnm_heap_node_from_lcrs(root);

	while (!lcrs_istail(siblings)) {
		struct sbnm_heap_node *curr;
		struct lcrs_node      *nxt;

		curr = sbnm_heap_node_from_lcrs(siblings);
		if (rnode->sbnm_rank != curr->sbnm_rank)
			break;

		nxt = lcrs_next(siblings);

		rnode = sbnm_heap_join(rnode, curr, compare);

		siblings = nxt;
	}

	lcrs_assign_next(&rnode->sbnm_lcrs, siblings);

	return &rnode->sbnm_lcrs;
}

static struct sbnm_heap_node *
sbnm_heap_2way_merge_roots(struct lcrs_node     **restrict first,
                           struct lcrs_node     **restrict second,
                           sbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(*first);
	assert(second);
	assert(*second);
	assert(first != second);
	assert(*first != *second);
	assert(compare);

	struct sbnm_heap_node *restrict fst = sbnm_heap_node_from_lcrs(*first);
	struct sbnm_heap_node *restrict snd = sbnm_heap_node_from_lcrs(*second);

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
sbnm_heap_merge_roots(struct lcrs_node     **result,
                      struct lcrs_node      *source,
                      sbnm_heap_compare_fn  *compare)
{
	assert(result);
	assert(!lcrs_istail(*result));
	assert(source);
	assert(!lcrs_istail(source));
	assert(*result != source);
	assert(compare);

	struct lcrs_node      *res = *result;
	struct sbnm_heap_node *tmp;

	tmp = sbnm_heap_2way_merge_roots(&res, &source, compare);
	*result = &tmp->sbnm_lcrs;

	while (!(lcrs_istail(res) || lcrs_istail(source))) {
		struct sbnm_heap_node *tail;

		tmp = sbnm_heap_2way_merge_roots(&res, &source, compare);

		assert(!lcrs_istail(*result));
		tail = sbnm_heap_node_from_lcrs(*result);
		assert(tail->sbnm_rank <= tmp->sbnm_rank);

		if (tail->sbnm_rank != tmp->sbnm_rank)
			result = lcrs_next_ref(*result);
		else
			tmp = sbnm_heap_join(tail, tmp, compare);

		*result = &tmp->sbnm_lcrs;
	}

	if (lcrs_istail(res))
		res = source;

	*result = sbnm_heap_1way_merge_roots(*result, res, compare);

	return *result;
}

static void
sbnm_heap_remove_root(struct sbnm_heap  *heap,
                      struct lcrs_node **previous,
                      struct lcrs_node  *root)
{
	assert(heap);
	assert(previous);
	assert(root);
	assert(*previous == root);

	struct lcrs_node *dummy = &heap->sbnm_dummy;
	struct lcrs_node *trees;

	trees = lcrs_mktail(dummy);

	heap->sbnm_count--;

	*previous = lcrs_next(root);

	root = lcrs_youngest(root);
	while (!lcrs_istail(root)) {
		struct lcrs_node *nxt = lcrs_next(root);

		lcrs_assign_next(root, trees);
		trees = root;

		root = nxt;
	}

	if (!lcrs_has_child(dummy)) {
		lcrs_assign_youngest(dummy, trees);

		return;
	}

	if (lcrs_istail(trees))
		return;

	sbnm_heap_merge_roots(lcrs_youngest_ref(dummy), trees,
	                      heap->sbnm_compare);
}

static void
sbnm_heap_remove_key(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_assert(heap);
	assert(heap->sbnm_count);

	struct lcrs_node **prev;
	
	while (true) {
		struct sbnm_heap_node *parent;

		parent = sbnm_heap_parent_node(heap, key);
		if (!parent)
			break;

		sbnm_heap_swap(parent, key);
	}

	prev = lcrs_youngest_ref(&heap->sbnm_dummy);
	while (*prev != &key->sbnm_lcrs)
		prev = lcrs_next_ref(*prev);

	sbnm_heap_remove_root(heap, prev, &key->sbnm_lcrs);
}

void
sbnm_heap_insert(struct sbnm_heap *heap, struct sbnm_heap_node *key)
{
	sbnm_heap_assert(heap);
	assert(key);

	struct lcrs_node     *dummy = &heap->sbnm_dummy;
	sbnm_heap_compare_fn *cmp = heap->sbnm_compare;
	struct lcrs_node     *node;

	lcrs_init(&key->sbnm_lcrs);
	key->sbnm_rank = 0;

	heap->sbnm_count++;

	node = sbnm_heap_1way_merge_roots(&key->sbnm_lcrs,
	                                  lcrs_youngest(dummy), cmp);
	lcrs_assign_youngest(dummy, node);
}

struct sbnm_heap_node *
sbnm_heap_extract(struct sbnm_heap *heap)
{
	sbnm_heap_assert(heap);
	assert(heap->sbnm_count);

	struct lcrs_node      **prev;
	struct sbnm_heap_node  *key;
	struct sbnm_heap_node  *curr;

	prev = lcrs_youngest_ref(&heap->sbnm_dummy);
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
	assert(heap->sbnm_count);

	sbnm_heap_compare_fn *cmp = heap->sbnm_compare;

	while (true) {
		struct sbnm_heap_node *parent;

		parent = sbnm_heap_parent_node(heap, key);
		if (!parent)
			break;

		if (cmp(parent, key) <= 0)
			break;

		sbnm_heap_swap(parent, key);
	}
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
	assert(result->sbnm_count);
	sbnm_heap_assert(source);
	assert(source->sbnm_count);
	assert(result->sbnm_compare == source->sbnm_compare);

	struct lcrs_node *dummy = &result->sbnm_dummy;
	struct lcrs_node *node;

	result->sbnm_count += source->sbnm_count;

	node = sbnm_heap_merge_roots(lcrs_youngest_ref(dummy),
	                             lcrs_youngest(&source->sbnm_dummy),
	                             result->sbnm_compare);
	lcrs_assign_parent(node, dummy);
}
