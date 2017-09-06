/**
 * @file      bheap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary heap interface
 *
 * @defgroup bheap_fixed Fixed length array based binary heap
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

#ifndef _BHEAP_H
#define _BHEAP_H

#include <bstree.h>

/**
 * Fixed length array based binary heap
 *
 * @ingroup bheap_fixed
 */
struct bheap_fixed {
	/** Size of a single node / item in bytes. */
	size_t              node_size;
	/** underlying binary search tree */
	struct bstree_fixed bheap_tree;
};

#define bheap_assert_fixed(_heap)  \
	assert(_heap);             \
	assert((_heap)->node_size)

/**
 * Indicate wether a fixed length array based binary heap is empty or not
 *
 * @param heap heap to test
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup bheap_fixed
 */
static inline bool bheap_fixed_empty(struct bheap_fixed *heap)
{
	bheap_assert_fixed(heap);

	return bstree_fixed_empty(&heap->bheap_tree);
}

/**
 * Indicate wether a fixed length array based binary heap is full or not
 *
 * @param heap heap to test
 *
 * @retval true  full
 * @retval false not full
 *
 * @ingroup bheap_fixed
 */
static inline bool bheap_fixed_full(struct bheap_fixed *heap)
{
	bheap_assert_fixed(heap);

	return bstree_fixed_full(&heap->bheap_tree);
}

/**
 * Retrieve first node satisfying the heap property
 *
 * @param heap heap to retrieve node from
 *
 * Heap propery is preserved through array_compare_fn function pointer passed as
 * argument at insertion and extraction time.
 * Depending on user's compare implementation, bheap_peek_fixed() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup bheap_fixed
 */
static inline char * bheap_peek_fixed(const struct bheap_fixed *heap)
{
	bheap_assert_fixed(heap);
	assert(!bstree_fixed_empty(&heap->bheap_tree));

	return bstree_fixed_root(&heap->bheap_tree, heap->node_size);
}

/**
 * Insert data into a fixed length array based binary heap
 *
 * @param heap    heap to insert into
 * @param node    data to insert
 * @param compare comparison function used to locate the right array slot to
 *                insert data into
 *
 * @p node is inserted by copy.
 *
 * @ingroup bheap_fixed
 */
extern void bheap_insert_fixed(struct bheap_fixed *heap,
                               const char         *node,
                               array_compare_fn   *compare);

/**
 * Extract data from a fixed length array based binary heap
 *
 * @param heap    heap to extract from
 * @param node    data location to extract into
 * @param compare comparison function used to preserve heap properties
 *
 * @p node is inserted by copy.
 * Heap propery is preserved through array_compare_fn function pointer passed as
 * argument at insertion and extraction time.
 * Depending on user's compare implementation, bheap_peek_fixed() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup bheap_fixed
 */
extern void bheap_extract_fixed(struct bheap_fixed *heap,
                                char               *node,
                                array_compare_fn   *compare);

/**
 * Clear content of specified bheap_fixed
 *
 * @param heap heap to clear
 *
 * @ingroup bheap_fixed
 */
static inline void bheap_clear_fixed(struct bheap_fixed *heap)
{
	bheap_assert_fixed(heap);

	bstree_clear_fixed(&heap->bheap_tree);
}

/**
 * Build / heapify a bheap_fixed initialized with unsorted data
 *
 * @param heap    heap to heapify
 * @param count   count of nodes to heapify
 * @param compare comparison function used to locate the right array slot to
 *                insert data into
 *
 * Build @p heap bheap_fixed from an the array passed as argument to
 * bheap_init_fixed() according to Floyd algorithm in O(n) time complexity.
 *
 * @warning Behavior is undefined if @p count is zero.
 *
 * @ingroup bheap_fixed
 */
extern void bheap_build_fixed(struct bheap_fixed *heap,
                              unsigned int        count,
                              array_compare_fn   *compare);
/**
 * Initialize a bheap_fixed
 *
 * @param heap      heap to initialize
 * @param nodes     underlying memory area containing nodes
 * @param node_size size in bytes of a single node sitting into @p heap
 * @param nr        maximum number of nodes @p heap may contain
 *
 * @p nodes must point to a memory area large enough to contain at least @p nr
 * nodes
 *
 * @warning Behavior is undefined when called with a zero @p nr or a zero
 * @p node_size.
 *
 * @ingroup bheap_fixed
 */
extern void bheap_init_fixed(struct bheap_fixed *heap,
                             char               *nodes,
                             size_t              node_size,
                             unsigned int        nr);

/**
 * Release resources allocated for a bheap_fixed
 *
 * @param heap heap to release resources for
 *
 * @ingroup bheap_fixed
 */
extern void bheap_fini_fixed(struct bheap_fixed *heap __unused);

/**
 * Create a bheap_fixed
 *
 * @param node_size size in bytes of a single node sitting into @p heap
 * @param nr        maximum number of nodes @p heap may contain
 *
 * Wrapper allocating and initializing a bheap_fixed.
 * 
 * @warning Behavior is undefined when called with a zero @p nr or a zero
 * @p node_size.
 * 
 * @return pointer to new created binary heap
 *
 * @ingroup bheap_fixed
 */
extern struct bheap_fixed * bheap_create_fixed(size_t       node_size,
                                               unsigned int nr);

/**
 * Release resources allocated by bheap_create_fixed() for fixed length array
 * based binary heap
 *
 * @param heap heap to release resources for
 *
 * @ingroup bheap_fixed
 */
extern void bheap_destroy_fixed(struct bheap_fixed *heap);

#endif
