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

#define LCRS_TAIL_NODE ((uintptr_t)1U)

static inline bool lcrs_istail_node(const struct lcrs_node *node)
{
	assert(node);

	return !!((uintptr_t)node & LCRS_TAIL_NODE);
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
                      struct lcrs_node *restrict       start)
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

static inline bool
lcrs_node_has_child(const struct lcrs_node * node)
{
	return node->lcrs_youngest;
}

static inline struct lcrs_node * lcrs_eldest_sibling(struct lcrs_node *node)
{
	assert(!lcrs_istail_node(node));

	while (!lcrs_istail_node(node->lcrs_sibling))
		node = node->lcrs_sibling;

	return node;
}

static inline struct lcrs_node * lcrs_parent_node(struct lcrs_node *node)
{
	return lcrs_untail_node(lcrs_eldest_sibling(node)->lcrs_sibling);
}

static inline bool lcrs_isroot_node(const struct lcrs_node *node)
{
	return node->lcrs_sibling == lcrs_mktail_node(NULL);
}

static inline void lcrs_attach_node(struct lcrs_node *restrict node,
                                    struct lcrs_node *restrict parent)
{
	assert(node);
	assert(parent);

	node->lcrs_sibling = parent->lcrs_youngest;
	parent->lcrs_youngest = node;
}

static inline void lcrs_init_node(struct lcrs_node *node)
{
	assert(node);

	node->lcrs_sibling = lcrs_mktail_node(NULL);
	node->lcrs_youngest = NULL;
}

extern void lcrs_swap_down_node(struct lcrs_node *node,
                                struct lcrs_node *child);

#endif
