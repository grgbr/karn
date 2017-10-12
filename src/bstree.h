/**
 * @file      bstree.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary search tree interface
 *
 * @defgroup bstree_fixed Fixed length array based binary search tree
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

#ifndef _BSTREE_H
#define _BSTREE_H

#include <array.h>
#include <stdbool.h>

/**
 * Fixed length array based binary search tree
 *
 * @ingroup bstree_fixed
 */
struct bstree_fixed {
	/** Number of nodes currently sitting into the tree */
	unsigned int       bst_count;
	/** Array of nodes contained in this tree */
	struct array_fixed bst_nodes;
};

/* Internal bstree_fixed consistency checker */
#define bstree_assert_fixed(_tree)                                         \
	assert(_tree);                                                     \
	assert((_tree)->bst_count <= array_fixed_nr(&(_tree)->bst_nodes)); \

/**
 * Test wether a bstree_fixed tree is empty or not
 *
 * @param tree bstree_fixed to test.
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup bstree_fixed
 */
static inline bool
bstree_fixed_empty(const struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	return !tree->bst_count;
}

/**
 * Test wether a bstree_fixed tree is full or not
 *
 * @param tree bstree_fixed to test.
 *
 * @retval true  full
 * @retval false not full
 *
 * @ingroup bstree_fixed
 */
static inline bool
bstree_fixed_full(const struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	return tree->bst_count == (array_fixed_nr(&tree->bst_nodes));
}

/**
 * Retrieve bstree_fixed node specified by index
 *
 * @param tree      bstree_fixed to retrieve node from
 * @param node_size size in bytes of a single node sitting into @p tree
 * @param index     index identifying the node to retrieve
 *
 * @return pointer to node
 *
 * @warning Behavior is undefined if @p index is out of bounds.
 *
 * @ingroup bstree_fixed
 */
static inline char *
bstree_fixed_node(const struct bstree_fixed *tree,
                  unsigned int               index)
{
	bstree_assert_fixed(tree);
	assert(index < array_fixed_nr(&tree->bst_nodes));

	return array_fixed_item(&tree->bst_nodes, index);
}

/**
 * Retrieve root node of specified bstree_fixed
 *
 * @param tree      bstree_fixed to retrieve root from
 * @param node_size size in bytes of a single node sitting into @p tree
 *
 * @return pointer to root node or %NULL if @p tree is empty
 *
 * @ingroup bstree_fixed
 */
static inline char *
bstree_fixed_root(const struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	if (bstree_fixed_empty(tree))
		return NULL;

	return array_fixed_item(&tree->bst_nodes, 0);
}

/**
 * Retrieve last (deepest) node of specified bstree_fixed
 *
 * @param tree      bstree_fixed to retrieve last node from
 * @param node_size size in bytes of a single node sitting into @p tree
 *
 * @return pointer to last node or %NULL if @p tree is empty
 *
 * @ingroup bstree_fixed
 */
static inline char *
bstree_fixed_last(const struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	if (bstree_fixed_empty(tree))
		return NULL;

	return array_fixed_item(&tree->bst_nodes, tree->bst_count - 1);
}

/**
 * Retrieve next node available for insertion from the specified bstree_fixed
 *
 * @param tree      bstree_fixed to retrieve next free node from
 * @param node_size size in bytes of a single node sitting into @p tree
 *
 * @return pointer to next free node or %NULL if @p tree is full
 *
 * @ingroup bstree_fixed
 */
static inline char *
bstree_fixed_bottom(const struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	if (bstree_fixed_full(tree))
		return NULL;

	return array_fixed_item(&tree->bst_nodes, tree->bst_count);
}

/**
 * Retrieve node's parent owned by the specified bstree_fixed
 *
 * @param tree      bstree_fixed owning @p node
 * @param node_size size in bytes of a single node sitting into @p tree
 * @param node      node to retrieve parent from
 *
 * @return pointer to parent of @p node or %NULL if @p node is root of @p tree
 *
 * @ingroup bstree_fixed
 */
static inline char *
bstree_fixed_parent(const struct bstree_fixed *tree,
                    const char                *node)
{
	bstree_assert_fixed(tree);
	assert(node);

	unsigned int idx = array_fixed_item_index(&tree->bst_nodes, node);

	if (!idx)
		return NULL;

	return array_fixed_item(&tree->bst_nodes, (idx - 1) / 2);
}

/**
 * bstree_fixed tree node siblings descriptor
 *
 * @ingroup bstree_fixed
 */
struct bstree_siblings {
	/** left child */
	char *bst_left;
	/** right child */
	char *bst_right;
};

/**
 * Retrieve node's siblings owned by the specified bstree_fixed
 *
 * @param tree      bstree_fixed owning @p node
 * @param node_size size in bytes of a single node sitting into @p tree
 * @param node      node to retrieve siblings from
 *
 * @return bstree_siblings structure with %NULL sibling pointer(s) if @p node is
 * located into the deepest @p tree level.
 *
 * @ingroup bstree_fixed
 */
static inline struct bstree_siblings
bstree_fixed_siblings(const struct bstree_fixed *tree,
                      const char                *node)
{
	bstree_assert_fixed(tree);
	assert(node);

	unsigned int           idx;
	struct bstree_siblings sibs = {
		.bst_left  = NULL,
		.bst_right = NULL
	};

	idx = (2 * array_fixed_item_index(&tree->bst_nodes, node)) + 1;

	if (idx < tree->bst_count)
		sibs.bst_left = array_fixed_item(&tree->bst_nodes, idx);

	if ((idx + 1) < tree->bst_count)
		sibs.bst_right = sibs.bst_left + tree->bst_nodes.arr_size;

	return sibs;
}

/**
 * Remove all nodes from specified bstree_fixed
 *
 * @param tree bstree_fixed to clear
 *
 * @ingroup bstree_fixed
 */
static inline void bstree_clear_fixed(struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);

	tree->bst_count = 0;
}

/**
 * Account for node insertion into specified bstree_fixed
 *
 * @param tree bstree_fixed
 *
 * @warning Behavior is undefined if @p tree is full.
 *
 * @ingroup bstree_fixed
 */
static inline void bstree_fixed_credit(struct bstree_fixed *tree)
{
	assert(tree);
	assert(tree->bst_count < array_fixed_nr(&tree->bst_nodes));

	tree->bst_count++;
}

/**
 * Account for node removal from specified bstree_fixed
 *
 * @param tree bstree_fixed
 *
 * @warning Behavior is undefined if @p tree is empty.
 *
 * @ingroup bstree_fixed
 */
static inline void bstree_fixed_debit(struct bstree_fixed *tree)
{
	bstree_assert_fixed(tree);
	assert(tree->bst_count > 0);

	tree->bst_count--;
}

/**
 * Initialize a bstree_fixed
 *
 * @param tree    bstree_fixed to initialize
 * @param nodes   underlying memory area containing nodes
 * @param node_nr maximum number of nodes @p tree may contain
 *
 * @p nodes must point to a memory area large enough to contain at least @p nr
 * nodes
 *
 * @warning Behavior is undefined when called with a zero @p nr.
 *
 * @ingroup bstree_fixed
 */
static inline void bstree_init_fixed(struct bstree_fixed *tree,
                                     char                *nodes,
                                     size_t               node_size,
                                     unsigned int         node_nr)
{
	assert(tree);
	assert(nodes);
	assert(node_size);
	assert(node_nr);

	tree->bst_count = 0;
	array_init_fixed(&tree->bst_nodes, nodes, node_size, node_nr);
}

/**
 * Release resources allocated by a bstree_fixed
 *
 * @param tree bstree_fixed to release resources for
 *
 * @ingroup bstree_fixed
 */
static inline void bstree_fini_fixed(struct bstree_fixed *tree __unused)
{
	bstree_assert_fixed(tree);

	array_fini_fixed(&tree->bst_nodes);
}

#endif
