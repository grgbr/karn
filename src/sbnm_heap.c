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

/*
 * TODO:
 *   - refactor sibling list iteration
 *   - refactor siftup
 */

#include "sbnm_heap.h"
#include <assert.h>

static struct sbnm_heap_node *
sbnm_heap_join_trees(struct sbnm_heap_node *first,
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

static void
sbnm_heap_swap(struct sbnm_heap_node *parent, struct sbnm_heap_node *node)
{
	assert(parent);
	assert(node);

	struct sbnm_heap_node *tmp = parent->sbnm_parent;
	unsigned int           order;

	if (tmp) {
		/* parent is not a binomial tree root node. */
		if (tmp->sbnm_eldest == parent)
			tmp->sbnm_eldest = node;
		else
			sbnm_heap_preceding_sibling(tmp->sbnm_eldest,
			                            parent)->sbnm_sibling = node;
	}
	node->sbnm_parent = tmp;

	if (parent->sbnm_eldest == node) {
		tmp = node->sbnm_eldest;
		if (tmp)
			/* node is not located at deepest level. */
			tmp->sbnm_parent = parent;
		node->sbnm_eldest = parent;

		parent->sbnm_parent = node;
		parent->sbnm_eldest = tmp;
	}
	else {
		tmp = parent->sbnm_eldest;
		tmp->sbnm_parent = node;

		sbnm_heap_preceding_sibling(tmp, node)->sbnm_sibling = parent;

		parent->sbnm_parent = node;
		parent->sbnm_eldest = node->sbnm_eldest;
		node->sbnm_eldest = tmp;
	}

	tmp = node->sbnm_sibling;
	node->sbnm_sibling = parent->sbnm_sibling;
	parent->sbnm_sibling = tmp;

	assert(parent->sbnm_order > 0);
	order = node->sbnm_order;
	node->sbnm_order = parent->sbnm_order;
	parent->sbnm_order = order;
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

static void
sbnm_heap_siftdown(struct sbnm_heap      *heap,
                   struct sbnm_heap_node *key,
                   sbnm_heap_compare_fn  *compare)
{
	struct sbnm_heap_node *child;

	if (!key->sbnm_eldest)
		return;

	child = sbnm_heap_inorder_child(key->sbnm_eldest, compare);
	if (compare(key, child) < 0)
		return;

	if (!key->sbnm_parent) {
		struct sbnm_heap_node *root = heap->sbnm_trees;

		/*
		 * Key is a root node and it will bubble down its own
		 * tree: setup proper root nodes list links in advance.
		 */
		if (key != root) {
			struct sbnm_heap_node *prev;

			/*
			 * Iterate over the list of root nodes untill
			 * key is found then setup proper (future) link
			 * from preceding list root node.
			 */
			do {
				prev = root;
				root = root->sbnm_sibling;
				assert(root);
			} while (root != key);

			prev->sbnm_sibling = child;
		}
		else
			/*
			 * First list node should point to key's
			 * subtree since it will become root after
			 * bubbling down.
			 */
			heap->sbnm_trees = child;
	}

	do {
		sbnm_heap_swap(key, child);

		if (!key->sbnm_eldest)
			break;

		child = sbnm_heap_inorder_child(key->sbnm_eldest, compare);
	} while (compare(key, child) > 0);
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

	struct sbnm_heap_node *prev;
	struct sbnm_heap_node *root;

	if (key->sbnm_parent && (compare(key->sbnm_parent, key) > 0)) {
		struct sbnm_heap_node *old_root;

		/* Bubble key up. */
		do {
			old_root = key->sbnm_parent;
			sbnm_heap_swap(key->sbnm_parent, key);
		} while (key->sbnm_parent &&
		         (compare(key->sbnm_parent, key) > 0));

		if (key->sbnm_parent)
			/* No need to update list of root nodes. */
			return;

		/*
		 * Key bubbled up to root node location : rebuild list of root
		 * nodes.
		 */
		root = heap->sbnm_trees;
		if (root == old_root) {
			/* Key should be linked as first list node. */
			heap->sbnm_trees = key;
			return;
		}

		/*
		 * Iterate over the list of root nodes untill old root (the one
		 * before last key swap) is found then restore proper link from
		 * preceding list root node.
		 */
		do {
			prev = root;
			root = root->sbnm_sibling;
			assert(root);
		} while (root != old_root);

		prev->sbnm_sibling = key;
	}

	sbnm_heap_siftdown(heap, key, compare);
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

	while (cur) {
		struct sbnm_heap_node *nxt;

		if (key->sbnm_order != cur->sbnm_order)
			break;

		nxt = cur->sbnm_sibling;
		key = sbnm_heap_join_trees(key, cur, compare);

		cur = nxt;
	}

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

		return sbnm_heap_join_trees(fst, snd, compare);
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

struct sbnm_heap_node *
sbnm_heap_merge_trees(struct sbnm_heap_node *first,
                      struct sbnm_heap_node *second,
                      sbnm_heap_compare_fn  *compare)
{
	assert(first);
	assert(second);
	assert(compare);

	struct sbnm_heap_node  *head, *tail;
	struct sbnm_heap_node **prev;
	struct sbnm_heap_node  *tmp;

	head = sbnm_heap_merge_roots(&first, &second, compare);
	prev = &head;
	tail = head;

	while (first && second) {
		tmp = sbnm_heap_merge_roots(&first, &second, compare);

		assert(tail->sbnm_order <= tmp->sbnm_order);

		if (tail->sbnm_order == tmp->sbnm_order) {
			*prev = sbnm_heap_join_trees(tail, tmp, compare);
			tail = *prev;
		}
		else {
			prev = &tail->sbnm_sibling;
			tail->sbnm_sibling = tmp;
			tail = tmp;
		}
	}

	if (!first)
		first = second;

	while (first && (tail->sbnm_order == first->sbnm_order)) {
		assert(tail->sbnm_order <= first->sbnm_order);

		tmp = first->sbnm_sibling;
		*prev = sbnm_heap_join_trees(tail, first, compare);
		tail = *prev;
		first = tmp;
	}

	tail->sbnm_sibling = first;

	return head;
}

struct sbnm_heap_node *
sbnm_heap_peek(const struct sbnm_heap *heap, sbnm_heap_compare_fn *compare)
{
	/* TODO: optimize by always keeping a pointer to minimum root. */
	assert(heap->sbnm_trees);

	return sbnm_heap_inorder_child(heap->sbnm_trees, compare);
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

static struct sbnm_heap_node *
sbnm_heap_preceding_inorder_child(struct sbnm_heap_node  *eldest,
                                  struct sbnm_heap_node **previous,
                                  sbnm_heap_compare_fn   *compare)
{
	assert(eldest);
	assert(previous);
	assert(compare);

	struct sbnm_heap_node *inorder = eldest;
	struct sbnm_heap_node *prev = eldest;

	*previous = NULL;

	eldest = eldest->sbnm_sibling;
	while (eldest) {
		if (compare(eldest, inorder) < 0) {
			*previous = prev;
			inorder = eldest;
		}

		prev = eldest;
		eldest = eldest->sbnm_sibling;
	}

	return inorder;
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

	if (previous)
		previous->sbnm_sibling = root->sbnm_sibling;
	else
		heap->sbnm_trees = root->sbnm_sibling;

	if (root->sbnm_eldest) {
		struct sbnm_heap_node *roots;

		roots = sbnm_heap_reverse_children(root->sbnm_eldest);

		if (heap->sbnm_trees)
			heap->sbnm_trees =
				sbnm_heap_merge_trees(heap->sbnm_trees,
				                      roots, compare);
		else
			heap->sbnm_trees = roots;
	}

	heap->sbnm_count--;
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

	struct sbnm_heap_node *key;
	struct sbnm_heap_node *prev;

	key = sbnm_heap_preceding_inorder_child(heap->sbnm_trees, &prev,
	                                        compare);

	sbnm_heap_remove_root(heap, prev, key, compare);

	return key;
}
