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
#ifndef _KARN_LCRS_H
#define _KARN_LCRS_H

#include <karn/common.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef CONFIG_KARN_LCRS
#error LCRS configuration disabled !
#endif

struct lcrs_node {
	struct lcrs_node *lcrs_sibling;
	struct lcrs_node *lcrs_youngest;
} __align(sizeof(uintptr_t));

typedef int (lcrs_compare_fn)(const struct lcrs_node *restrict first,
                              const struct lcrs_node *restrict second);

#define LCRS_TAIL ((uintptr_t)1U)

#define LCRS_INIT(_node)                                                   \
	{                                                                  \
		.lcrs_sibling  = (struct lcrs_node *)(LCRS_TAIL),          \
		.lcrs_youngest = (struct lcrs_node *)((uintptr_t)(_node) | \
		                                      LCRS_TAIL)           \
	}

static inline bool lcrs_istail(const struct lcrs_node *node)
{
	karn_assert(node);

	return ((uintptr_t)node & LCRS_TAIL);
}

static inline struct lcrs_node * lcrs_mktail(const struct lcrs_node *node)
{
	return (struct lcrs_node *)((uintptr_t)node | LCRS_TAIL);
}

static inline struct lcrs_node * lcrs_untail(struct lcrs_node *node)
{
	karn_assert(node);

	return (struct lcrs_node *)((uintptr_t)node & ~LCRS_TAIL);
}

static inline struct lcrs_node *
lcrs_next(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return node->lcrs_sibling;
}

static inline struct lcrs_node **
lcrs_next_ref(struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return &node->lcrs_sibling;
}

static inline void
lcrs_assign_next(struct lcrs_node       *restrict node,
                 const struct lcrs_node *restrict sibling)
{
	karn_assert(!lcrs_istail(node));

	node->lcrs_sibling = (struct lcrs_node *)sibling;
}

static inline struct lcrs_node *
lcrs_previous(const struct lcrs_node *restrict node,
              const struct lcrs_node *restrict start)
{
	karn_assert(!lcrs_istail(node));
	karn_assert(!lcrs_istail(start));
	karn_assert(start != node);

	while (start->lcrs_sibling != node) {
		karn_assert(!lcrs_istail(start->lcrs_sibling));
		start = start->lcrs_sibling;
	}

	return (struct lcrs_node *)start;
}

static inline struct lcrs_node **
lcrs_previous_ref(const struct lcrs_node  *restrict node,
                  struct lcrs_node       **restrict start)
{
	karn_assert(!lcrs_istail(node));
	karn_assert(!lcrs_istail(*start));

	while (*start != node)
		start = lcrs_next_ref(*start);

	return (struct lcrs_node **)start;
}

static inline bool
lcrs_has_child(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return node->lcrs_youngest != lcrs_mktail(node);
}

static inline struct lcrs_node *
lcrs_youngest(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return node->lcrs_youngest;
}

static inline struct lcrs_node **
lcrs_youngest_ref(struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return &node->lcrs_youngest;
}

static inline struct lcrs_node *
lcrs_eldest(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	while (!lcrs_istail(node->lcrs_sibling))
		node = node->lcrs_sibling;

	return (struct lcrs_node *)node;
}

static inline bool
lcrs_has_parent(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return node->lcrs_sibling != lcrs_mktail(NULL);
}

static inline struct lcrs_node *
lcrs_parent(const struct lcrs_node *node)
{
	karn_assert(!lcrs_istail(node));

	return lcrs_untail(lcrs_eldest(node)->lcrs_sibling);
}

static inline void
lcrs_assign_parent(struct lcrs_node       *restrict node,
                   const struct lcrs_node *restrict parent)
{
	lcrs_eldest(node)->lcrs_sibling = lcrs_mktail(parent);
}

static inline void
lcrs_assign_youngest(struct lcrs_node       *restrict node,
                     const struct lcrs_node *restrict youngest)
{
	karn_assert(!lcrs_istail(node));

	node->lcrs_youngest = (struct lcrs_node *)youngest;
}

static inline void
lcrs_join(struct lcrs_node *restrict tree, struct lcrs_node *restrict parent)
{
	karn_assert(!lcrs_istail(tree));
	karn_assert(!lcrs_istail(parent));

	tree->lcrs_sibling = parent->lcrs_youngest;
	lcrs_assign_youngest(parent, tree);
}

static inline void
lcrs_split(const struct lcrs_node  *restrict tree,
           struct lcrs_node       **restrict previous)
{
	karn_assert(!lcrs_istail(tree));
	karn_assert(previous);
	karn_assert(!lcrs_istail(*previous));
	karn_assert(lcrs_parent(tree) == lcrs_parent(*previous));

	while (*previous != tree)
		previous = &(*previous)->lcrs_sibling;

	*previous = tree->lcrs_sibling;
}

static inline void
lcrs_init(struct lcrs_node *node)
{
	karn_assert(node);

	node->lcrs_sibling = lcrs_mktail(NULL);
	node->lcrs_youngest = lcrs_mktail(node);
}

extern struct lcrs_node * lcrs_swap_down(struct lcrs_node *node,
                                         struct lcrs_node *child);

#define lcrs_foreach_sibling(_youngest, _sibling)   \
	for ((_sibling) = (_youngest);              \
	     !lcrs_istail(_sibling);                \
	     (_sibling) = (_sibling)->lcrs_sibling)

#define lcrs_foreach_child(_node, _child) \
	lcrs_foreach_sibling((_node)->lcrs_youngest, _child)

#define lcrs_foreach_sibling_safe(_youngest, _sibling, _tmp)              \
	for ((_sibling) = (_youngest), (_tmp) = (_sibling)->lcrs_sibling; \
	     !lcrs_istail(_sibling);                                      \
	     (_sibling) = (_tmp), (_tmp) = (_sibling)->lcrs_sibling)

#define lcrs_foreach_child_safe(_node, _child, _tmp) \
	lcrs_foreach_sibling_safe((_node)->lcrs_youngest, _child, _tmp)

#define lcrs_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

#endif /* _KARN_LCRS_H */
