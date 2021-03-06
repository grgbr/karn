/**
 * @file      lcrs.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Nov 2017
 * @copyright GNU Public License v3
 *
 * Left child right sibling implementation
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
#include <karn/lcrs.h>

static void
lcrs_substitute_siblings(const struct lcrs_node *restrict node,
                         struct lcrs_node       *restrict start,
                         struct lcrs_node       *restrict substitute)
{
	karn_assert(substitute);

	lcrs_previous(node, start)->lcrs_sibling = substitute;
}

static void lcrs_swap_ref(struct lcrs_node **restrict first,
                          struct lcrs_node **restrict second)
{
	struct lcrs_node *tmp = *first;

	*first = *second;
	*second = tmp;
}

struct lcrs_node *
lcrs_swap_down(struct lcrs_node *node, struct lcrs_node *child)
{
	karn_assert(node != child);
	karn_assert(lcrs_has_child(node));
	karn_assert(lcrs_parent(child) == node);

	struct lcrs_node *tmp = node->lcrs_youngest;

	if (lcrs_has_child(child)) {
		/*
		 * "child" has children: update their parent pointer to make it
		 * point to "node", the future child.
		 */
		lcrs_assign_parent(child->lcrs_youngest, node);
		node->lcrs_youngest = child->lcrs_youngest;
	}
	else
		node->lcrs_youngest = lcrs_mktail(node);

	/* make "node" a child of "child". */
	if (child != tmp) {
		/*
		 * "child" is not the youngest amongst its siblings: update its
		 * preceding sibling pointer.
		 */
		lcrs_substitute_siblings(child, tmp, node);
		child->lcrs_youngest = tmp;
	}
	else
		child->lcrs_youngest = node;

	/* Update "child" siblings parent pointer to "child". */
	lcrs_assign_parent(child, child);

	/*
	 * keep a reference to "node"'s parent since altering lcrs_sibling field
	 * below will prevent us from finding "node"'s parent.
	 */
	tmp = lcrs_parent(node);

	/* swap "node" and "child" next sibling pointer. */
	lcrs_swap_ref(&child->lcrs_sibling, &node->lcrs_sibling);

	if (!tmp)
		return NULL;

	/* "node" has a parent: make "child" a child of it. */
	if (node != tmp->lcrs_youngest)
		/*
		 * "node" is not the youngest amongst its siblings: update its
		 * preceding sibling pointer.
		 */
		lcrs_substitute_siblings(node, tmp->lcrs_youngest, child);
	else
		tmp->lcrs_youngest = child;

	return tmp;
}
