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

void lcrs_swap_down_node(struct lcrs_node *node, struct lcrs_node *child)
{
	assert(node);
	assert(node->lcrs_youngest);
	assert(lcrs_parent_node(child) == node);

	struct lcrs_node *tmp;
	struct lcrs_node *parent;

	if (child->lcrs_youngest) {
		/*
		 * "child" has children: update their parent pointer to make it
		 * point to "node".
		 */
		tmp = lcrs_eldest_sibling(child->lcrs_youngest);
		tmp->lcrs_sibling = lcrs_mktail_node(node);
	}

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

	tmp = lcrs_eldest_sibling(child);
	tmp->lcrs_sibling = lcrs_mktail_node(child);

	/*
	 * keep a reference to "node"'s parent since altering lcrs_sibling field
	 * below will prevent us from finding "node"'s parent.
	 */
	parent = lcrs_parent_node(node);

	tmp = child->lcrs_sibling;
	child->lcrs_sibling = node->lcrs_sibling;
	node->lcrs_sibling = tmp;

	if (!parent)
		return;

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
