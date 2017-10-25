/**
 * @file      fbnr_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary heap interface
 *
 * @defgroup fbnr_heap Fixed length array based binary heap
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

#ifndef _FBNR_HEAP_H
#define _FBNR_HEAP_H

#include <fabs_tree.h>

/**
 * Fixed length array based binary heap
 *
 * @ingroup fbnr_heap
 */
struct fbnr_heap {
	/** Node comparator */
	farr_compare_fn  *fbnr_compare;
	/** Node copier */
	farr_copy_fn     *fbnr_copy;
	/** underlying binary search tree */
	struct fabs_tree  fbnr_tree;
};

#define fbnr_heap_assert(_heap)        \
	assert(_heap);                 \
	assert((_heap)->fbnr_compare); \
	assert((_heap)->fbnr_copy)

/**
 * Indicate wether a fixed length array based binary heap is empty or not
 *
 * @param heap heap to test
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup fbnr_heap
 */
static inline bool fbnr_heap_empty(const struct fbnr_heap *heap)
{
	fbnr_heap_assert(heap);

	return fabs_tree_empty(&heap->fbnr_tree);
}

/**
 * Indicate wether a fixed length array based binary heap is full or not
 *
 * @param heap heap to test
 *
 * @retval true  full
 * @retval false not full
 *
 * @ingroup fbnr_heap
 */
static inline bool fbnr_heap_full(const struct fbnr_heap *heap)
{
	fbnr_heap_assert(heap);

	return fabs_tree_full(&heap->fbnr_tree);
}

/**
 * Retrieve first node satisfying the heap property
 *
 * @param heap heap to retrieve node from
 *
 * Heap propery is preserved through farr_compare_fn function pointer passed as
 * argument at init time.
 * Depending on user's compare implementation, fbnr_heap_peek() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup fbnr_heap
 */
static inline char * fbnr_heap_peek(const struct fbnr_heap *heap)
{
	fbnr_heap_assert(heap);

	return fabs_tree_root(&heap->fbnr_tree);
}

/**
 * Insert data into a fixed length array based binary heap
 *
 * @param heap heap to insert into
 * @param node data to insert
 *
 * @p node is inserted by copy.
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_insert(struct fbnr_heap *heap, const char *node);

/**
 * Extract data from a fixed length array based binary heap
 *
 * @param heap    heap to extract from
 * @param node    data location to extract into
 *
 * @p node is inserted by copy.
 * Heap propery is preserved through farr_compare_fn function pointer passed as
 * argument at init time.
 * Depending on user's compare implementation, fbnr_heap_peek() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_extract(struct fbnr_heap *heap, char *node);

/**
 * Clear content of specified fbnr_heap
 *
 * @param heap heap to clear
 *
 * @ingroup fbnr_heap
 */
static inline void fbnr_heap_clear(struct fbnr_heap *heap)
{
	fbnr_heap_assert(heap);

	fabs_tree_clear(&heap->fbnr_tree);
}

/**
 * Build / heapify a fbnr_heap initialized with unsorted data
 *
 * @param heap  heap to heapify
 * @param count count of nodes to heapify
 *
 * Build @p heap fbnr_heap from an the array passed as argument to
 * fbnr_heap_init() according to Floyd algorithm in O(n) time complexity.
 *
 * @warning Behavior is undefined if @p count is zero.
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_build(struct fbnr_heap *heap, unsigned int count);

/**
 * Initialize a fbnr_heap
 *
 * @param heap      heap to initialize
 * @param nodes     underlying memory area containing nodes
 * @param node_size size in bytes of a single node sitting into @p heap
 * @param node_nr   maximum number of nodes @p heap may contain
 * @param compare   comparison function used to locate the right array slot to
 *                  insert data into
 * @param copy      copy function used to swap nodes / array slots.
 *
 * @p nodes must point to a memory area large enough to contain at least
 * @p node_nr nodes.
 *
 * @warning Behavior is undefined when called with a zero @p node_nr or a zero
 * @p node_size.
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_init(struct fbnr_heap *heap,
                           char             *nodes,
                           size_t            node_size,
                           unsigned int      node_nr,
                           farr_compare_fn  *compare,
                           farr_copy_fn     *copy);

/**
 * Release resources allocated for a fbnr_heap
 *
 * @param heap heap to release resources for
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_fini(struct fbnr_heap *heap __unused);

/**
 * Create a fbnr_heap
 *
 * @param node_size size in bytes of a single node sitting into @p heap
 * @param node_nr   maximum number of nodes @p heap may contain
 * @param compare   comparison function used to locate the right array slot to
 *                  insert data into
 * @param copy      copy function used to swap nodes / array slots.
 *
 * Wrapper allocating and initializing a fbnr_heap.
 *
 * @warning Behavior is undefined when called with a zero @p node_nr or a zero
 * @p node_size.
 *
 * @return pointer to new created binary heap
 *
 * @ingroup fbnr_heap
 */
extern struct fbnr_heap * fbnr_heap_create(size_t           node_size,
                                           unsigned int     node_nr,
                                           farr_compare_fn *compare,
                                           farr_copy_fn    *copy);

/**
 * Release resources allocated by fbnr_heap_create() for fixed length array
 * based binary heap
 *
 * @param heap heap to release resources for
 *
 * @ingroup fbnr_heap
 */
extern void fbnr_heap_destroy(struct fbnr_heap *heap);

#if defined(CONFIG_FBNR_HEAP_SORT)

extern void fbnr_heap_sort(char            *entries,
                           size_t           entry_size,
                           size_t           entry_nr,
                           farr_compare_fn *compare,
                           farr_copy_fn    *copy);

#endif /* defined(CONFIG_FBNR_HEAP_SORT) */

#endif
