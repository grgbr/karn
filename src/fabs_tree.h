/**
 * @file      fabs_tree.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary search tree interface
 *
 * @defgroup fabs_tree Fixed length array based binary search tree
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

#ifndef _FABS_TREE_H
#define _FABS_TREE_H

#include <farr.h>
#include <stdbool.h>

/**
 * Fixed length array based binary search tree
 *
 * @ingroup fabs_tree
 */
struct fabs_tree {
	/** Number of nodes currently sitting into the tree */
	unsigned int fabs_count;
	/** Array of nodes contained in this tree */
	struct farr  fabs_nodes;
};

/* Internal fabs_tree consistency checker */
#define fabs_tree_assert(_tree)                                       \
	assert(_tree);                                                \
	assert((_tree)->fabs_count <= farr_nr(&(_tree)->fabs_nodes)); \

#define FABS_TREE_ROOT_INDEX (0U)

/**
 * Return capacity of a fabs_tree in number of nodes
 *
 * @param tree fabs_tree to get capacity from
 *
 * @return maximum number of nodes
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_nr(const struct fabs_tree *tree)
{
	fabs_tree_assert(tree);

	return farr_nr(&tree->fabs_nodes);
}

/**
 * Return count of nodes hosted by a fabs_tree
 *
 * @param tree fabs_tree to test.
 *
 * @return count
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_count(const struct fabs_tree *tree)
{
	fabs_tree_assert(tree);

	return tree->fabs_count;
}

/**
 * Test wether a fabs_tree tree is empty or not
 *
 * @param tree fabs_tree to test.
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup fabs_tree
 */
static inline bool fabs_tree_empty(const struct fabs_tree *tree)
{
	return !fabs_tree_count(tree);
}

/**
 * Test wether a fabs_tree tree is full or not
 *
 * @param tree fabs_tree to test.
 *
 * @retval true  full
 * @retval false not full
 *
 * @ingroup fabs_tree
 */
static inline bool fabs_tree_full(const struct fabs_tree *tree)
{
	return (fabs_tree_count(tree) == fabs_tree_nr(tree));
}

/**
 * Return size of a single node hosted by fabs_tree passed as argument
 *
 * @param tree fabs_tree to get node size from
 *
 * @return size in bytes
 *
 * @ingroup fabs_tree
 */
static inline size_t fabs_tree_node_size(const struct fabs_tree *tree)
{
	fabs_tree_assert(tree);

	return farr_slot_size(&tree->fabs_nodes);
}

/**
 * Retrieve fabs_tree node specified by index
 *
 * @param tree  fabs_tree to retrieve node from
 * @param index index identifying the node to retrieve
 *
 * @return pointer to node
 *
 * @warning Behavior is undefined if @p index is out of bounds.
 *
 * @ingroup fabs_tree
 */
static inline char * fabs_tree_node(const struct fabs_tree *tree,
                                    unsigned int            index)
{
	fabs_tree_assert(tree);
	assert(index < farr_nr(&tree->fabs_nodes));

	return farr_slot(&tree->fabs_nodes, index);
}

/**
 * Retrieve index within specified fabs_tree of node passed as argument
 *
 * @param tree fabs_tree to retrieve node from
 * @param node node to retrieve index from
 *
 * @return index pointing to @p node location
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_node_index(const struct fabs_tree *tree,
                                                const char             *node)
{
	fabs_tree_assert(tree);

	return farr_slot_index(&tree->fabs_nodes, node);
}

/**
 * Retrieve root node of specified fabs_tree
 *
 * @param tree fabs_tree to retrieve root from
 *
 * @return pointer to root node
 *
 * @warning Behavior is undefined if @p tree points to an empty tree.
 *
 * @ingroup fabs_tree
 */
static inline char * fabs_tree_root(const struct fabs_tree *tree)
{
	assert(!fabs_tree_empty(tree));

	return farr_slot(&tree->fabs_nodes, FABS_TREE_ROOT_INDEX);
}

/**
 * Retrieve index of last (deepest) node of specified fabs_tree
 *
 * @param tree fabs_tree to retrieve last node index from
 *
 * @return index of last node
 *
 * @warning Behavior is undefined if @p tree points to an empty tree.
 *
 * @ingroup fabs_tree
 */
static inline unsigned int
fabs_tree_last_index(const struct fabs_tree *tree)
{
	assert(!fabs_tree_empty(tree));

	return fabs_tree_count(tree) - 1;
}

/**
 * Retrieve last (deepest) node of specified fabs_tree
 *
 * @param tree fabs_tree to retrieve last node from
 *
 * @return pointer to last node or %NULL if @p tree is empty
 *
 * @warning Behavior is undefined if @p tree points to an empty tree.
 *
 * @ingroup fabs_tree
 */
static inline char * fabs_tree_last(const struct fabs_tree *tree)
{
	return farr_slot(&tree->fabs_nodes, fabs_tree_last_index(tree));
}

/**
 * Retrieve index of next node available for insertion from the specified
 * fabs_tree
 *
 * @param tree fabs_tree to retrieve next free node from
 *
 * @return index of next available node
 *
 * @warning Behavior is undefined if @p tree points to a full tree.
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_bottom_index(const struct fabs_tree *tree)
{
	assert(!fabs_tree_full(tree));

	return fabs_tree_count(tree);
}

/**
 * Retrieve next node available for insertion from the specified fabs_tree
 *
 * @param tree fabs_tree to retrieve next free node from
 *
 * @return pointer to next free node or %NULL if @p tree is full
 *
 * @ingroup fabs_tree
 */
static inline char *
fabs_tree_bottom(const struct fabs_tree *tree)
{
	return farr_slot(&tree->fabs_nodes, fabs_tree_bottom_index(tree));
}

/**
 * Retrieve left child index of node specified by index
 *
 * @param index node index to retrieve left child index from
 *
 * @return index of node's left child
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_left_child_index(unsigned int index)
{
	return (2 * index) + 1;
}

/**
 * Retrieve right child index of node specified by index
 *
 * @param index node index to retrieve right child index from
 *
 * @return index of node's right child
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_right_child_index(unsigned int index)
{
	return (2 * index) + 2;
}

/**
 * Retrieve parent index of node specified by index
 *
 * @param index node index to retrieve parent index from
 *
 * @return index of node's parent
 *
 * @warning Behavior is undefined if @p index points to root location.
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_parent_index(unsigned int index)
{
	assert(index);

	return (index - 1) / 2;
}

/**
 * Retrieve depth of node specified by index
 *
 * @param index index of node to retrieve depth from
 *
 * @return depth starting from 0
 *
 * @ingroup fabs_tree
 */
static inline unsigned int fabs_tree_index_depth(unsigned int index)
{
	return lower_pow2(index + 1);
}

/**
 * Retrieve ancestor index of node specified by index
 *
 * @param index        index of node to ancestor's index from
 * @param depth_offset designate depth of ancestor node to consider
 *
 * Will return index of ancestor located @p depth_offset higher in the
 * fabs_tree hierarchy. Specifically, if a zero @p depth_offset is passed as
 * argument, fabs_tree_ancestor_index() returns the value of @p index.
 *
 * As an example, a @p depth_offset with value 1 returns the parent's index of
 * node identified by @p index ; a @p depth_offset with value 2 returns the
 * grand-parent's index of node identified by @p index ; and so on...
 *
 * @warning Behavior is undefined if @p depth_offset is larger than @p index
 * depth.
 *
 * @return ancestor's index
 *
 * @ingroup fabs_tree
 */
static inline unsigned int
fabs_tree_ancestor_index(unsigned int index, unsigned int depth_offset)
{
	assert(depth_offset <= fabs_tree_index_depth(index));

	return (index - (1U << depth_offset) + 1) >> depth_offset;
}

/**
 * Remove all nodes from specified fabs_tree
 *
 * @param tree fabs_tree to clear
 *
 * @ingroup fabs_tree
 */
static inline void fabs_tree_clear(struct fabs_tree *tree)
{
	fabs_tree_assert(tree);

	tree->fabs_count = 0;
}

/**
 * Account for node insertion into specified fabs_tree
 *
 * @param tree fabs_tree
 *
 * @warning Behavior is undefined if @p tree is full.
 *
 * @ingroup fabs_tree
 */
static inline void fabs_tree_credit(struct fabs_tree *tree)
{
	assert(!fabs_tree_full(tree));

	tree->fabs_count++;
}

/**
 * Account for node removal from specified fabs_tree
 *
 * @param tree fabs_tree
 *
 * @warning Behavior is undefined if @p tree is empty.
 *
 * @ingroup fabs_tree
 */
static inline void fabs_tree_debit(struct fabs_tree *tree)
{
	assert(!fabs_tree_empty(tree));

	tree->fabs_count--;
}

/**
 * Initialize a fabs_tree
 *
 * @param tree      fabs_tree to initialize
 * @param nodes     underlying memory area containing nodes
 * @param node_size size in bytes of a single node sitting into @p tree
 * @param node_nr   maximum number of nodes @p tree may contain
 *
 * @p nodes must point to a memory area large enough to contain at least @p nr
 * nodes
 *
 * @warning Behavior is undefined when called with a zero @p nr.
 *
 * @ingroup fabs_tree
 */
static inline void fabs_tree_init(struct fabs_tree *tree,
                                  char             *nodes,
                                  size_t            node_size,
                                  unsigned int      node_nr)
{
	assert(tree);
	assert(nodes);
	assert(node_size);
	assert(node_nr);

	tree->fabs_count = 0;
	farr_init(&tree->fabs_nodes, nodes, node_size, node_nr);
}

/**
 * Release resources allocated by a fabs_tree
 *
 * @param tree fabs_tree to release resources for
 *
 * @ingroup fabs_tree
 */
static inline void fabs_tree_fini(struct fabs_tree *tree __unused)
{
	fabs_tree_assert(tree);

	farr_fini(&tree->fabs_nodes);
}

#endif
