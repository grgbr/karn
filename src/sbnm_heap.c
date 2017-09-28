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
sbnm_heap_preceding_sibling(struct sbnm_heap_node       *eldest,
                            const struct sbnm_heap_node *sibling)
{
	assert(eldest);
	assert(sibling);

	while (eldest->sbnm_sibling != sibling) {
		eldest = eldest->sbnm_sibling;
		assert(eldest);
	}

	return eldest;
}

static struct sbnm_heap_node *
sbnm_heap_inorder_child(struct sbnm_heap_node *eldest,
                        sbnm_heap_compare_fn  *compare)
{
	assert(eldest);
	assert(compare);

	struct sbnm_heap_node *inorder = eldest;

	eldest = eldest->sbnm_sibling;
	while (eldest) {
		if (compare(eldest, inorder) < 0)
			inorder = eldest;
		eldest = eldest->sbnm_sibling;
	}

	return inorder;
}

static struct sbnm_heap_node *
sbnm_heap_join(struct sbnm_heap_node *first,
               struct sbnm_heap_node *second,
               sbnm_heap_compare_fn  *compare)
{
	assert(first->sbnm_order == second->sbnm_order);

	struct sbnm_heap_node *root;
	struct sbnm_heap_node *child;

	if (compare(first, second) <= 0) {
		root = first;
		child = second;
	}
	else {
		root = second;
		child = first;
	}

	child->sbnm_parent = root;
	child->sbnm_sibling = root->sbnm_eldest;

	root->sbnm_eldest = child;
	root->sbnm_order++;

	return root;
}

static void
sbnm_heap_swap(struct sbnm_heap_node *parent, struct sbnm_heap_node *node)
{
	assert(parent);
	assert(parent->sbnm_order > 0);
	assert(node);

	struct sbnm_heap_node *tmp = parent->sbnm_parent;
	unsigned int           order = parent->sbnm_order;

	node->sbnm_parent = tmp;

	if (tmp) {
		/* parent is not a binomial tree root node. */
		if (tmp->sbnm_eldest != parent) {
			tmp = sbnm_heap_preceding_sibling(tmp->sbnm_eldest,
			                                  parent);
			tmp->sbnm_sibling = node;
		}
		else
			tmp->sbnm_eldest = node;
	}

	tmp = parent->sbnm_eldest;
	parent->sbnm_eldest = node->sbnm_eldest;

	if (tmp == node) {
		tmp = node->sbnm_eldest;
		if (tmp)
			/* node is not located at deepest level. */
			tmp->sbnm_parent = parent;
		node->sbnm_eldest = parent;
	}
	else {
		tmp->sbnm_parent = node;
		sbnm_heap_preceding_sibling(tmp, node)->sbnm_sibling = parent;
		node->sbnm_eldest = tmp;
	}

	tmp = parent->sbnm_sibling;

	parent->sbnm_sibling = node->sbnm_sibling;
	parent->sbnm_parent = node;
	parent->sbnm_order = node->sbnm_order;

	node->sbnm_sibling = tmp;
	node->sbnm_order = order;
}

static void
sbnm_heap_restore_root(struct sbnm_heap      *heap,
                       struct sbnm_heap_node *old_root,
                       struct sbnm_heap_node *new_root)
{
	struct sbnm_heap_node *roots = heap->sbnm_trees;

	if (roots == old_root) {
		/*
		 * First list node should point to key's subtree since it will
		 * become root after bubbling down.
		 */
		heap->sbnm_trees = new_root;

		return;
	}

	/*
	 * Iterate over the list of root nodes untill key is found then setup
	 * proper (future) link from preceding list root node.
	 */
	sbnm_heap_preceding_sibling(roots, old_root)->sbnm_sibling = new_root;
}

static void
sbnm_heap_update_next(struct sbnm_heap      *heap,
                      struct sbnm_heap_node *key,
                      sbnm_heap_compare_fn  *compare)
{
	if (compare(key, heap->sbnm_next) < 0)
		heap->sbnm_next = key;
}

static void
sbnm_heap_find_next(struct sbnm_heap      *heap,
                    struct sbnm_heap_node *key,
                    sbnm_heap_compare_fn  *compare)
{
	while (key->sbnm_sibling) {
		key = key->sbnm_sibling;
		sbnm_heap_update_next(heap, key, compare);
	}
}

void
sbnm_heap_update(struct sbnm_heap      *heap,
                 struct sbnm_heap_node *key,
                 sbnm_heap_compare_fn  *compare)

{
	assert(heap);
	assert(heap->sbnm_trees);
	assert(key);
	assert(compare);

	struct sbnm_heap_node *root;

	/*
	 * Bubble up.
	 */
	if (key->sbnm_parent) {
		if (compare(key->sbnm_parent, key) > 0) {
			/* While bubbling key up, keep a reference to old root. */
			do {
				root = key->sbnm_parent;
				sbnm_heap_swap(key->sbnm_parent, key);
			} while (key->sbnm_parent &&
				 (compare(key->sbnm_parent, key) > 0));

			if (!key->sbnm_parent) {
				/*
				 * Key bubbled up to root node location : rebuild list
				 * of root nodes.
				 */
				sbnm_heap_restore_root(heap, root, key);
				sbnm_heap_update_next(heap, key, compare);
			}

			return;
		}
	}
	else
		sbnm_heap_update_next(heap, key, compare);

	/*
	 * Bubble down.
	 */
	if (!key->sbnm_eldest)
		/* Nothing to do. */
		return;

	/* Save a reference to futur root node. */
	root = sbnm_heap_inorder_child(key->sbnm_eldest, compare);

	if (compare(key, root) < 0)
		/* In order: nothing's left to do. */
		return;

	if (!key->sbnm_parent) {
		/*
		 * Key is a root node and it will bubble down its own tree:
		 * setup proper root nodes list links in advance.
		 */
		sbnm_heap_restore_root(heap, key, root);

		if (key != heap->sbnm_next) {
			struct sbnm_heap_node *node = heap->sbnm_trees;

			heap->sbnm_next = node;
			sbnm_heap_find_next(heap, node, compare);
		}
		else
			sbnm_heap_update_next(heap, root, compare);
	}

	/* Restore heap order property by bubbling key down. */
	do {
		sbnm_heap_swap(key, root);

		if (!key->sbnm_eldest)
			break;

		root = sbnm_heap_inorder_child(key->sbnm_eldest, compare);
	} while (compare(key, root) > 0);

}

void
sbnm_heap_insert(struct sbnm_heap      *heap,
                 struct sbnm_heap_node *key,
                 sbnm_heap_compare_fn  *compare)
{
	struct sbnm_heap_node *cur = heap->sbnm_trees;

	key->sbnm_parent = NULL;
	key->sbnm_eldest = NULL;
	key->sbnm_sibling = NULL;
	key->sbnm_order = 0;

	if (cur) {
		do {
			struct sbnm_heap_node *nxt;

			if (key->sbnm_order != cur->sbnm_order)
				break;

			nxt = cur->sbnm_sibling;
			key = sbnm_heap_join(key, cur, compare);

			cur = nxt;
		} while (cur);

		if (!key->sbnm_parent && (compare(key, heap->sbnm_next) <= 0))
			heap->sbnm_next = key;
	}
	else
		heap->sbnm_next = key;

	key->sbnm_sibling = cur;
	heap->sbnm_trees = key;

	heap->sbnm_count++;

	return;
}

static struct sbnm_heap_node *
sbnm_heap_merge_roots(struct sbnm_heap_node **first,
                      struct sbnm_heap_node **second,
                      sbnm_heap_compare_fn   *compare)
{
	struct sbnm_heap_node *fst = *first;
	struct sbnm_heap_node *snd = *second;

	if (fst->sbnm_order == snd->sbnm_order) {
		*first = (*first)->sbnm_sibling;
		*second = (*second)->sbnm_sibling;

		return sbnm_heap_join(fst, snd, compare);
	}
	else if (fst->sbnm_order < snd->sbnm_order) {
		*first = (*first)->sbnm_sibling;
		return fst;
	}
	else {
		*second = (*second)->sbnm_sibling;
		return snd;
	}
}

void
sbnm_heap_merge_trees(struct sbnm_heap      *result,
                      struct sbnm_heap_node *tree,
                      sbnm_heap_compare_fn  *compare)
{
	assert(result);
	assert(tree);
	assert(compare);

	struct sbnm_heap_node  *res = result->sbnm_trees;
	struct sbnm_heap_node  *head, *tail;
	struct sbnm_heap_node **prev;
	struct sbnm_heap_node  *tmp;

	head = sbnm_heap_merge_roots(&res, &tree, compare);
	prev = &head;
	tail = head;

	result->sbnm_next = head;

	while (res && tree) {
		tmp = sbnm_heap_merge_roots(&res, &tree, compare);

		assert(tail->sbnm_order <= tmp->sbnm_order);

		if (tail->sbnm_order == tmp->sbnm_order) {
			*prev = sbnm_heap_join(tail, tmp, compare);
			tail = *prev;
		}
		else {
			prev = &tail->sbnm_sibling;
			tail->sbnm_sibling = tmp;
			tail = tmp;
		}

		sbnm_heap_update_next(result, tail, compare);
	}

	if (!res)
		res = tree;

	while (res && (tail->sbnm_order == res->sbnm_order)) {
		assert(tail->sbnm_order <= res->sbnm_order);

		tmp = res->sbnm_sibling;
		*prev = sbnm_heap_join(tail, res, compare);
		res = tmp;

		tail = *prev;
		sbnm_heap_update_next(result, tail, compare);
	}

	tail->sbnm_sibling = res;

	sbnm_heap_find_next(result, tail, compare);

	result->sbnm_trees = head;
}

static struct sbnm_heap_node *
sbnm_heap_reverse_children(struct sbnm_heap_node *eldest)
{
	assert(eldest);

	struct sbnm_heap_node *head = NULL;

	do {
		struct sbnm_heap_node *nxt = eldest->sbnm_sibling;

		eldest->sbnm_parent = NULL;
		eldest->sbnm_sibling = head;

		head = eldest;

		eldest = nxt;
	} while (eldest);

	return head;
}

static void
sbnm_heap_remove_root(struct sbnm_heap            *heap,
                      struct sbnm_heap_node       *previous,
                      const struct sbnm_heap_node *root,
                      sbnm_heap_compare_fn        *compare)
{
	assert(heap);
	assert(root);
	assert(compare);

	heap->sbnm_count--;

	if (previous)
		previous->sbnm_sibling = root->sbnm_sibling;
	else
		heap->sbnm_trees = root->sbnm_sibling;

	if (root->sbnm_eldest) {
		struct sbnm_heap_node *roots;

		roots = sbnm_heap_reverse_children(root->sbnm_eldest);

		if (heap->sbnm_trees) {
			sbnm_heap_merge_trees(heap, roots, compare);
			return;
		}

		heap->sbnm_trees = roots;
	}

	previous = heap->sbnm_trees;
	if (!previous)
		return;

	heap->sbnm_next = previous;
	sbnm_heap_find_next(heap, previous, compare);
}

void
sbnm_heap_remove(struct sbnm_heap      *heap,
                 struct sbnm_heap_node *key,
                 sbnm_heap_compare_fn  *compare)
{
	sbnm_heap_assert(heap);
	assert(heap->sbnm_count > 0);
	assert(key);
	assert(compare);

	struct sbnm_heap_node *root = key;

	while (key->sbnm_parent) {
		root = key->sbnm_parent;
		sbnm_heap_swap(key->sbnm_parent, key);
	}

	if (root != heap->sbnm_trees)
		root = sbnm_heap_preceding_sibling(heap->sbnm_trees, root);
	else
		root = NULL;

	sbnm_heap_remove_root(heap, root, key, compare);
}

struct sbnm_heap_node *
sbnm_heap_extract(struct sbnm_heap *heap, sbnm_heap_compare_fn *compare)
{
	assert(heap);
	assert(heap->sbnm_trees);
	assert(heap->sbnm_count);
	assert(compare);

	struct sbnm_heap_node *key = heap->sbnm_next;
	struct sbnm_heap_node *prev = NULL;

	if (key != heap->sbnm_trees)
		prev = sbnm_heap_preceding_sibling(heap->sbnm_trees, key);

	sbnm_heap_remove_root(heap, prev, key, compare);

	return key;
}
