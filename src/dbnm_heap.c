/**
 * @file      dbnm_heap.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list based binomial heap implementation
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

#include "dlist.h"
#include "dbnm_heap.h"

static struct dbnm_heap_node *
dbnm_heap_sbl2node(struct dlist_node *sibling)
{
	return dlist_entry(sibling, struct dbnm_heap_node, dbnm_sibling);
}

static struct dbnm_heap_node *
dbnm_heap_join(struct dbnm_heap_node *first,
               struct dbnm_heap_node *second,
               dbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);
	assert(first->dbnm_order == second->dbnm_order);

	struct dbnm_heap_node *root;
	struct dbnm_heap_node *subtree;

	if (compare(first, second) <= 0) {
		root = first;
		subtree = second;
	}
	else {
		root = second;
		subtree = first;
	}

	if (root->dbnm_child)
		dlist_insert(root->dbnm_child, &subtree->dbnm_sibling);
	else
		dlist_init(&subtree->dbnm_sibling);

	subtree->dbnm_parent = root;

	root->dbnm_child = &subtree->dbnm_sibling;
	root->dbnm_order++;

	return root;
}

void dbnm_heap_insert(struct dbnm_heap      *heap,
                      struct dbnm_heap_node *key,
                      dbnm_heap_compare_fn  *compare)
{
	dbnm_heap_assert(heap);
	assert(key);
	assert(compare);

	struct dlist_node *cur = dlist_next(&heap->dbnm_roots);

	key->dbnm_parent = NULL;
	key->dbnm_child = NULL;
	key->dbnm_order = 0;

	while (cur != &heap->dbnm_roots) {
		struct dbnm_heap_node *hcur;
		struct dlist_node    *nxt;

		hcur = dbnm_heap_sbl2node(cur);
		if (key->dbnm_order != hcur->dbnm_order)
			break;

		nxt = dlist_next(cur);

		key = dbnm_heap_join(key, hcur, compare);

		cur = nxt;
	}

	dlist_inject(&heap->dbnm_roots, &key->dbnm_sibling, cur);

	heap->dbnm_count++;

	return;
}

static struct dbnm_heap_node *
dbnm_heap_inorder_child(struct dlist_node       *child,
                        const struct dlist_node *end,
                        dbnm_heap_compare_fn    *compare)
{
	assert(child);
	assert(end);
	assert(compare);

	struct dbnm_heap_node *inorder = dbnm_heap_sbl2node(child);
	struct dbnm_heap_node *hcur;

	while (true) {
		child = child->dlist_next;
		if (child == end)
			break;

		hcur = dbnm_heap_sbl2node(child);
		if (compare(hcur, inorder) < 0)
			inorder = hcur;
	}

	return inorder;
}

struct dbnm_heap_node *
dbnm_heap_peek(const struct dbnm_heap *heap, dbnm_heap_compare_fn *compare)
{
	/* TODO: optimize by always keeping a pointer to minimum root. */
	assert(!dbnm_heap_empty(heap));
	assert(compare);

	return dbnm_heap_inorder_child(heap->dbnm_roots.dlist_next,
	                               &heap->dbnm_roots, compare);
}

static struct dbnm_heap_node *
dbnm_heap_select_root(struct dlist_node    *first,
                      struct dlist_node    *second,
                      dbnm_heap_compare_fn *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct dbnm_heap_node *fst;
	struct dbnm_heap_node *snd;

	fst = dbnm_heap_sbl2node(first);
	assert(!fst->dbnm_parent);

	snd = dbnm_heap_sbl2node(second);
	assert(!snd->dbnm_parent);

	if (fst->dbnm_order == snd->dbnm_order) {
		dlist_remove(first);
		dlist_remove(second);
		return dbnm_heap_join(fst, snd, compare);
	}
	else if (fst->dbnm_order < snd->dbnm_order) {
		dlist_remove(first);
		return fst;
	}
	else {
		dlist_remove(second);
		return snd;
	}
}

static void
dbnm_heap_reverse_children(struct dlist_node *result, struct dlist_node *eldest)
{
	assert(result);
	assert(eldest);

	struct dlist_node *cur = dlist_next(eldest);

	dlist_init(result);

	while (cur != eldest) {
		struct dlist_node *next = dlist_next(cur);

		dbnm_heap_sbl2node(cur)->dbnm_parent = NULL;

		dlist_append(result, cur);

		cur = next;
	}

	dbnm_heap_sbl2node(eldest)->dbnm_parent = NULL;
	dlist_insert(result, eldest);
}

static void
dbnm_heap_merge_roots(struct dlist_node     *result,
                      struct dbnm_heap_node *source,
                      dbnm_heap_compare_fn  *compare)
{
	assert(result);
	assert (!dlist_empty(result));
	assert(source);
	assert(!source->dbnm_parent);
	assert(compare);

	struct dbnm_heap_node *tail;

	tail = dbnm_heap_sbl2node(dlist_prev(result));
	assert(tail->dbnm_order <= source->dbnm_order);

	if (tail->dbnm_order == source->dbnm_order) {
		dlist_remove(&tail->dbnm_sibling);
		source = dbnm_heap_join(tail, source, compare);
	}

	dlist_insert(result, &source->dbnm_sibling);
}

void
dbnm_heap_merge_trees(struct dlist_node   *result,
                     struct dlist_node    *source,
                     dbnm_heap_compare_fn *compare)
{
	assert(!dlist_empty(result));
	assert(!dlist_empty(source));

	struct dlist_node     res = DLIST_INIT(res);
	struct dbnm_heap_node *src;

	dlist_splice(&res, dlist_next(result), dlist_prev(result));

	src = dbnm_heap_select_root(dlist_next(&res),
	                           dlist_next(source), compare);
	dlist_insert(result, &src->dbnm_sibling);

	while (!(dlist_empty(&res) || dlist_empty(source))) {
		src = dbnm_heap_select_root(dlist_next(&res),
		                           dlist_next(source), compare);

		dbnm_heap_merge_roots(result, src, compare);
	}

	if (dlist_empty(source))
		source = &res;

	while (!dlist_empty(source)) {
		src = dbnm_heap_sbl2node(dlist_next(source));
		dlist_remove(&src->dbnm_sibling);

		dbnm_heap_merge_roots(result, src, compare);
	}
}

static void
dbnm_heap_remove_key(struct dbnm_heap     *heap,
                    struct dbnm_heap_node *key,
                    dbnm_heap_compare_fn  *compare)
{
	assert(heap);
	assert(key);
	assert(compare);

	struct dlist_node *child = key->dbnm_child;

	dlist_remove(&key->dbnm_sibling);

	if (child) {
		struct dlist_node *roots = &heap->dbnm_roots;

		if (!dlist_empty(roots)) {
			struct dlist_node children;

			dbnm_heap_reverse_children(&children, child);
			dbnm_heap_merge_trees(roots, &children, compare);
		}
		else
			dbnm_heap_reverse_children(roots, child);
	}

	heap->dbnm_count--;
}

struct dbnm_heap_node *
dbnm_heap_extract(struct dbnm_heap *heap, dbnm_heap_compare_fn *compare)
{
	dbnm_heap_assert(heap);
	assert(heap->dbnm_count);
	assert(compare);

	struct dlist_node    *roots = &heap->dbnm_roots;
	struct dbnm_heap_node *key;

	key = dbnm_heap_inorder_child(dlist_next(roots), roots, compare);

	dbnm_heap_remove_key(heap, key, compare);

	return key;
}

static void dbnm_heap_swap(struct dbnm_heap_node *parent,
                           struct dbnm_heap_node *node)
{
	assert(parent);
	assert(!dlist_empty(&parent->dbnm_sibling));
	assert(parent->dbnm_child);
	assert(parent->dbnm_order);
	assert(node);
	assert(node->dbnm_parent = parent);

	struct dbnm_heap_node *ancestor = parent->dbnm_parent;
	struct dbnm_heap_node *child = dbnm_heap_sbl2node(node->dbnm_child);
	unsigned int          order = node->dbnm_order;

	if (ancestor && (ancestor->dbnm_child == &parent->dbnm_sibling))
		ancestor->dbnm_child = &node->dbnm_sibling;

	if (dlist_empty(&node->dbnm_sibling)) {
		assert(!child);
		assert(parent->dbnm_child == &node->dbnm_sibling);

		dlist_replace(&parent->dbnm_sibling, &node->dbnm_sibling);
		dlist_init(&parent->dbnm_sibling);
	}
	else {
		struct dlist_node tmp = node->dbnm_sibling;

		dlist_replace(&parent->dbnm_sibling, &node->dbnm_sibling);
		dlist_replace(&tmp, &parent->dbnm_sibling);
	}

	if (parent->dbnm_child == &node->dbnm_sibling)
		node->dbnm_child = &parent->dbnm_sibling;
	else
		node->dbnm_child = parent->dbnm_child;

	node->dbnm_parent = ancestor;
	node->dbnm_order = parent->dbnm_order;

	parent->dbnm_parent = node;
	parent->dbnm_order = order;

	if (child) {
		parent->dbnm_child = &child->dbnm_sibling;
		child->dbnm_parent = parent;
	}
	else
		parent->dbnm_child = NULL;
}

void dbnm_heap_remove(struct dbnm_heap      *heap,
                      struct dbnm_heap_node *key,
                      dbnm_heap_compare_fn  *compare)
{
	while (key->dbnm_parent)
		dbnm_heap_swap(key->dbnm_parent, key);

	dbnm_heap_remove_key(heap, key, compare);
}

void dbnm_heap_update(struct dbnm_heap_node *key, dbnm_heap_compare_fn *compare)
{
	assert(key);
	assert(compare);

	if (key->dbnm_parent && (compare(key->dbnm_parent, key) > 0)) {
		/* Bubble up. */
		do {
			dbnm_heap_swap(key->dbnm_parent, key);
		} while (key->dbnm_parent &&
		         (compare(key->dbnm_parent, key) > 0));

		return;
	}

	while (key->dbnm_child) {
		/* Bubble down. */
		struct dbnm_heap_node *child;

		child = dbnm_heap_inorder_child(key->dbnm_child,
		                                key->dbnm_child, compare);
		if (compare(key, child) < 0)
			break;

		dbnm_heap_swap(key, child);
	}
}
