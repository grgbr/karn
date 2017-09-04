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

/* TODO:
 *   - test subtree order consistency
 *   - test root nodes linking when updating key
 *   - test remove node
 */

#include "dlist.h"
#include "bnm_heap.h"

static struct bnm_heap_node *
bnm_heap_sbl2node(struct dlist_node *sibling)
{
	return dlist_entry(sibling, struct bnm_heap_node, bnm_sibling);
}

static struct bnm_heap_node *
bnm_heap_join(struct bnm_heap_node *first,
              struct bnm_heap_node *second,
              bnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);
	assert(first->bnm_order == second->bnm_order);

	struct bnm_heap_node *root;
	struct bnm_heap_node *subtree;

	if (compare(first, second) <= 0) {
		root = first;
		subtree = second;
	}
	else {
		root = second;
		subtree = first;
	}

	if (root->bnm_child)
		dlist_insert(root->bnm_child, &subtree->bnm_sibling);
	else
		dlist_init(&subtree->bnm_sibling);

	subtree->bnm_parent = root;

	root->bnm_child = &subtree->bnm_sibling;
	root->bnm_order++;

	return root;
}

void bnm_heap_insert(struct bnm_heap      *heap,
                     struct bnm_heap_node *key,
                     bnm_heap_compare_fn  *compare)
{
	bnm_heap_assert(heap);
	assert(key);
	assert(compare);

	struct dlist_node *cur = dlist_next(&heap->bnm_roots);

	key->bnm_parent = NULL;
	key->bnm_child = NULL;
	key->bnm_order = 0;

	while (cur != &heap->bnm_roots) {
		struct bnm_heap_node *hcur;
		struct dlist_node    *nxt;

		hcur = bnm_heap_sbl2node(cur);
		if (key->bnm_order != hcur->bnm_order)
			break;

		nxt = dlist_next(cur);

		key = bnm_heap_join(key, hcur, compare);

		cur = nxt;
	}

	dlist_inject(&heap->bnm_roots, &key->bnm_sibling, cur);

	heap->bnm_count++;

	return;
}

static struct bnm_heap_node *
bnm_heap_inorder_child(struct dlist_node       *child,
                       const struct dlist_node *end,
                       bnm_heap_compare_fn     *compare)
{
	assert(child);
	assert(end);
	assert(compare);

	struct bnm_heap_node *inorder = bnm_heap_sbl2node(child);
	struct bnm_heap_node *hcur;

	while (true) {
		child = child->dlist_next;
		if (child == end)
			break;

		hcur = bnm_heap_sbl2node(child);
		if (compare(hcur, inorder) < 0)
			inorder = hcur;
	}

	return inorder;
}

struct bnm_heap_node *
bnm_heap_peek(const struct bnm_heap *heap, bnm_heap_compare_fn *compare)
{
	/* TODO: optimize by always keeping a pointer to minimum root. */
	assert(!bnm_heap_empty(heap));
	assert(compare);

	return bnm_heap_inorder_child(heap->bnm_roots.dlist_next,
	                              &heap->bnm_roots, compare);
}

static struct bnm_heap_node *
bnm_heap_select_root(struct dlist_node   *first,
                     struct dlist_node   *second,
                     bnm_heap_compare_fn *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct bnm_heap_node *fst;
	struct bnm_heap_node *snd;

	fst = bnm_heap_sbl2node(first);
	assert(!fst->bnm_parent);

	snd = bnm_heap_sbl2node(second);
	assert(!snd->bnm_parent);

	if (fst->bnm_order == snd->bnm_order) {
		dlist_remove(first);
		dlist_remove(second);
		return bnm_heap_join(fst, snd, compare);
	}
	else if (fst->bnm_order < snd->bnm_order) {
		dlist_remove(first);
		return fst;
	}
	else {
		dlist_remove(second);
		return snd;
	}
}

static void
bnm_heap_reverse_children(struct dlist_node *result, struct dlist_node *eldest)
{
	assert(result);
	assert(eldest);

	struct dlist_node *cur = dlist_next(eldest);

	dlist_init(result);

	while (cur != eldest) {
		struct dlist_node *next = dlist_next(cur);

		bnm_heap_sbl2node(cur)->bnm_parent = NULL;

		dlist_append(result, cur);

		cur = next;
	}

	bnm_heap_sbl2node(eldest)->bnm_parent = NULL;
	dlist_insert(result, eldest);
}

static void
bnm_heap_merge_roots(struct dlist_node    *result,
                     struct bnm_heap_node *source,
                     bnm_heap_compare_fn  *compare)
{
	assert(result);
	assert (!dlist_empty(result));
	assert(source);
	assert(!source->bnm_parent);
	assert(compare);

	struct bnm_heap_node *tail;

	tail = bnm_heap_sbl2node(dlist_prev(result));
	assert(tail->bnm_order <= source->bnm_order);

	if (tail->bnm_order == source->bnm_order) {
		dlist_remove(&tail->bnm_sibling);
		source = bnm_heap_join(tail, source, compare);
	}

	dlist_insert(result, &source->bnm_sibling);
}

void
bnm_heap_merge_trees(struct dlist_node   *result,
                     struct dlist_node   *source,
                     bnm_heap_compare_fn *compare)
{
	assert(!dlist_empty(result));
	assert(!dlist_empty(source));

	struct dlist_node     res = DLIST_INIT(res);
	struct bnm_heap_node *src;

	dlist_splice(&res, dlist_next(result), dlist_prev(result));

	src = bnm_heap_select_root(dlist_next(&res),
	                           dlist_next(source), compare);
	dlist_insert(result, &src->bnm_sibling);

	while (!(dlist_empty(&res) || dlist_empty(source))) {
		src = bnm_heap_select_root(dlist_next(&res),
		                           dlist_next(source), compare);

		bnm_heap_merge_roots(result, src, compare);
	}

	if (dlist_empty(source))
		source = &res;

	while (!dlist_empty(source)) {
		src = bnm_heap_sbl2node(dlist_next(source));
		dlist_remove(&src->bnm_sibling);

		bnm_heap_merge_roots(result, src, compare);
	}
}

struct bnm_heap_node *
bnm_heap_extract(struct bnm_heap *heap, bnm_heap_compare_fn *compare)
{
	bnm_heap_assert(heap);
	assert(heap->bnm_count);
	assert(compare);

	struct dlist_node    *roots = &heap->bnm_roots;
	struct dlist_node    *child;
	struct bnm_heap_node *key;

	key = bnm_heap_inorder_child(dlist_next(roots), roots, compare);
	dlist_remove(&key->bnm_sibling);

	child = key->bnm_child;
	if (child) {
		if (!dlist_empty(roots)) {
			struct dlist_node children;

			bnm_heap_reverse_children(&children, child);
			bnm_heap_merge_trees(roots, &children, compare);
		}
		else
			bnm_heap_reverse_children(roots, child);
	}

	heap->bnm_count--;

	return key;
}

static void bnm_heap_swap(struct bnm_heap_node *parent,
                          struct bnm_heap_node *node)
{
	assert(parent);
	assert(!dlist_empty(&parent->bnm_sibling));
	assert(parent->bnm_child);
	assert(parent->bnm_order);
	assert(node);
	assert(node->bnm_parent = parent);

	struct bnm_heap_node *ancestor = parent->bnm_parent;
	struct bnm_heap_node *child = bnm_heap_sbl2node(node->bnm_child);
	unsigned int          order = node->bnm_order;

	if (ancestor && (ancestor->bnm_child == &parent->bnm_sibling))
		ancestor->bnm_child = &node->bnm_sibling;

	if (dlist_empty(&node->bnm_sibling)) {
		assert(!child);
		assert(parent->bnm_child == &node->bnm_sibling);

		dlist_replace(&parent->bnm_sibling, &node->bnm_sibling);
		dlist_init(&parent->bnm_sibling);
	}
	else {
		struct dlist_node tmp = node->bnm_sibling;

		dlist_replace(&parent->bnm_sibling, &node->bnm_sibling);
		dlist_replace(&tmp, &parent->bnm_sibling);
	}

	if (parent->bnm_child == &node->bnm_sibling)
		node->bnm_child = &parent->bnm_sibling;
	else
		node->bnm_child = parent->bnm_child;

	node->bnm_parent = ancestor;
	node->bnm_order = parent->bnm_order;

	parent->bnm_parent = node;
	parent->bnm_order = order;

	if (child) {
		parent->bnm_child = &child->bnm_sibling;
		child->bnm_parent = parent;
	}
	else
		parent->bnm_child = NULL;
}

void bnm_heap_update(struct bnm_heap_node *key, bnm_heap_compare_fn *compare)

{
	assert(key);
	assert(compare);

	if (key->bnm_parent && (compare(key->bnm_parent, key) > 0)) {
		/* Bubble up. */
		do {
			bnm_heap_swap(key->bnm_parent, key);
		} while (key->bnm_parent &&
		         (compare(key->bnm_parent, key) > 0));

		return;
	}

	while (key->bnm_child) {
		/* Bubble down. */
		struct bnm_heap_node *child;

		child = bnm_heap_inorder_child(key->bnm_child, key->bnm_child,
		                               compare);
		if (compare(key, child) < 0)
			break;

		bnm_heap_swap(key, child);
	}
}
