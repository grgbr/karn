/**
 * @file      lcrs.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Nov 2017
 * @copyright GNU Public License v3
 *
 * Left child right sibling interface
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
#ifndef _LCRS_H
#define _LCRS_H

#include <utils.h>
#include <stdbool.h>
#include <stdint.h>

struct lcrs_node {
	struct lcrs_node *lcrs_sibling;
	struct lcrs_node *lcrs_youngest;
} __align(sizeof(uintptr_t));

typedef int (lcrs_compare_fn)(const struct lcrs_node *restrict first,
                              const struct lcrs_node *restrict second);

#define LCRS_TAIL_NODE ((uintptr_t)1U)

static inline bool lcrs_istail_node(const struct lcrs_node *node)
{
	assert(node);

	return ((uintptr_t)node & LCRS_TAIL_NODE);
}

static inline struct lcrs_node * lcrs_mktail_node(struct lcrs_node *node)
{
	return (struct lcrs_node *)((uintptr_t)node | LCRS_TAIL_NODE);
}

static inline struct lcrs_node * lcrs_untail_node(struct lcrs_node *node)
{
	assert(node);

	return (struct lcrs_node *)((uintptr_t)node & ~LCRS_TAIL_NODE);
}

static inline struct lcrs_node *
lcrs_previous_sibling(const struct lcrs_node *restrict node,
                      struct lcrs_node       *restrict start)
{
	assert(!lcrs_istail_node(node));
	assert(!lcrs_istail_node(start));
	assert(start != node);

	while (start->lcrs_sibling != node) {
		assert(!lcrs_istail_node(start->lcrs_sibling));
		start = start->lcrs_sibling;
	}

	return start;
}

static inline struct lcrs_node * lcrs_next_sibling(const struct lcrs_node *node)
{
	assert(!lcrs_istail_node(node));

	return node->lcrs_sibling;
}

static inline bool
lcrs_node_has_child(const struct lcrs_node * node)
{
	assert(!lcrs_istail_node(node));

	return node->lcrs_youngest;
}

static inline struct lcrs_node *
lcrs_youngest_sibling(const struct lcrs_node *node)
{
	assert(!lcrs_istail_node(node));

	return node->lcrs_youngest;
}

static inline struct lcrs_node *
lcrs_eldest_sibling(const struct lcrs_node *node)
{
	assert(!lcrs_istail_node(node));

	while (!lcrs_istail_node(node->lcrs_sibling))
		node = node->lcrs_sibling;

	return (struct lcrs_node *)node;
}

static inline struct lcrs_node * lcrs_parent_node(const struct lcrs_node *node)
{
	assert(!lcrs_istail_node(node));

	return lcrs_untail_node(lcrs_eldest_sibling(node)->lcrs_sibling);
}

static inline void lcrs_join_tree(struct lcrs_node *restrict tree,
                                  struct lcrs_node *restrict parent)
{
	assert(!lcrs_istail_node(tree));
	assert(!lcrs_istail_node(parent));

	if (parent->lcrs_youngest)
		tree->lcrs_sibling = parent->lcrs_youngest;
	else
		tree->lcrs_sibling = lcrs_mktail_node(parent);

	parent->lcrs_youngest = tree;
}

static inline void lcrs_init_node(struct lcrs_node *node)
{
	assert(node);

	node->lcrs_sibling = lcrs_mktail_node(NULL);
	node->lcrs_youngest = NULL;
}

extern void lcrs_split_tree(const struct lcrs_node *restrict tree,
                            struct lcrs_node       *restrict parent);

extern void lcrs_swap_down_node(struct lcrs_node *node,
                                struct lcrs_node *child);

#endif
