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
#include "lcrs.h"

void lcrs_split_tree(const struct lcrs_node *restrict tree,
                     struct lcrs_node       *restrict parent)
{
	assert(tree != parent);
	assert(!lcrs_istail_node(tree));
	assert(lcrs_parent_node(tree) == parent);
	assert(lcrs_node_has_child(parent));

	if (tree != parent->lcrs_youngest) {
		struct lcrs_node *prev;

		prev = lcrs_previous_sibling(tree, parent->lcrs_youngest);
		prev->lcrs_sibling = tree->lcrs_sibling;

		return;
	}

	if (!lcrs_istail_node(tree->lcrs_sibling))
		parent->lcrs_youngest = tree->lcrs_sibling;
	else
		parent->lcrs_youngest = NULL;
}

void lcrs_swap_down_node(struct lcrs_node *node, struct lcrs_node *child)
{
	assert(node != child);
	assert(lcrs_node_has_child(node));
	assert(lcrs_parent_node(child) == node);

	struct lcrs_node *tmp;
	struct lcrs_node *parent;

	if (lcrs_node_has_child(child)) {
		/*
		 * "child" has children: update their parent pointer to make it
		 * point to "node", the future child.
		 */
		tmp = lcrs_eldest_sibling(child->lcrs_youngest);
		tmp->lcrs_sibling = lcrs_mktail_node(node);
	}

	/* make "node" a child of "node". */
	tmp = node->lcrs_youngest;
	node->lcrs_youngest = child->lcrs_youngest;
	if (child != tmp) {
		/*
		 * "child" is not the youngest amongst its siblings: update its
		 * preceding sibling pointer.
		 */
		child->lcrs_youngest = tmp;
		tmp = lcrs_previous_sibling(child, tmp);
		tmp->lcrs_sibling = node;
	}
	else
		child->lcrs_youngest = node;

	/* Update "child" siblings parent pointer to "node". */
	tmp = lcrs_eldest_sibling(child);
	tmp->lcrs_sibling = lcrs_mktail_node(child);

	/*
	 * keep a reference to "node"'s parent since altering lcrs_sibling field
	 * below will prevent us from finding "node"'s parent.
	 */
	parent = lcrs_parent_node(node);

	/* swap "node" and "child" next sibling pointer. */
	tmp = child->lcrs_sibling;
	child->lcrs_sibling = node->lcrs_sibling;
	node->lcrs_sibling = tmp;

	if (!parent)
		return;

	/* "node" has a parent: make "child" a child of it. */
	if (node != parent->lcrs_youngest) {
		/*
		 * "node" is not the youngest amongst its siblings: update its
		 * preceding sibling pointer.
		 */
		tmp = lcrs_previous_sibling(node, parent->lcrs_youngest);
		tmp->lcrs_sibling = child;
	}
	else
		parent->lcrs_youngest = child;
}
