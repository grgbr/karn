/**
 * @file      fwk_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      06 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based weak heap interface
 *
 * @defgroup fwk_heap Fixed length array based weak heap
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

#ifndef _KARN_FWK_HEAP_H
#define _KARN_FWK_HEAP_H

#include <karn/fbmp.h>
#include <karn/farr.h>

/**
 * Fixed length array based weak heap
 *
 * @ingroup fwk_heap
 */
struct fwk_heap {
	/** Node comparator */
	farr_compare_fn *fwk_compare;
	/** Node copier */
	farr_copy_fn    *fwk_copy;
	/** Current number of hosted nodes */
	unsigned int     fwk_count;
	/**
	 * Reverse bits bitmap used to identify wether a node is a left or right
	 * child
	 */
	uintptr_t       *fwk_rbits;
	/* Undelying array of data nodes */
	struct farr      fwk_nodes;
};

#define fwk_heap_assert(_heap) \
	karn_assert(_heap); \
	karn_assert((_heap)->fwk_compare); \
	karn_assert((_heap)->fwk_copy); \
	karn_assert((_heap)->fwk_count <= farr_nr(&(_heap)->fwk_nodes))

#define FWK_HEAP_ROOT_INDEX (0U)

/**
 * Return capacity of a fwk_heap in number of nodes
 *
 * @param heap fwk_heap to get capacity from
 *
 * @return maximum number of nodes
 *
 * @ingroup fwk_heap
 */
static inline unsigned int fwk_heap_nr(const struct fwk_heap *heap)
{
	fwk_heap_assert(heap);

	return farr_nr(&heap->fwk_nodes);
}

/**
 * Return count of nodes hosted by a fwk_heap
 *
 * @param heap fwk_heap to get capacity from
 *
 * @return count
 *
 * @ingroup fwk_heap
 */
static inline unsigned int fwk_heap_count(const struct fwk_heap *heap)
{
	fwk_heap_assert(heap);

	return heap->fwk_count;
}

/**
 * Indicate wether a fixed length array based weak heap is empty or not
 *
 * @param heap heap to test
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup fwk_heap
 */
static inline unsigned int fwk_heap_empty(const struct fwk_heap *heap)
{
	return !fwk_heap_count(heap);
}

/**
 * Indicate wether a fixed length array based weak heap is full or not
 *
 * @param heap heap to test
 *
 * @retval true  full
 * @retval false not full
 *
 * @ingroup fwk_heap
 */
static inline unsigned int fwk_heap_full(const struct fwk_heap *heap)
{
	return fwk_heap_count(heap) == fwk_heap_nr(heap);
}

/**
 * Retrieve first node satisfying the weak heap property
 *
 * @param heap heap to retrieve node from
 *
 * Heap propery is preserved through farr_compare_fn function pointer passed as
 * argument at init time.
 * Depending on user's compare implementation, fwk_heap_peek() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup fwk_heap
 */
static inline char * fwk_heap_peek(const struct fwk_heap *heap)
{
	karn_assert(!fwk_heap_empty(heap));

	return farr_slot(&heap->fwk_nodes, FWK_HEAP_ROOT_INDEX);
}

/**
 * Insert data into a fixed length array based weak heap
 *
 * @param heap heap to insert into
 * @param node data to insert
 *
 * @p node is inserted by copy.
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_insert(struct fwk_heap *heap, const char *node);

/**
 * Extract data from a fixed length array based weak heap
 *
 * @param heap    heap to extract from
 * @param node    data location to extract into
 *
 * @p node is inserted by copy.
 * Heap propery is preserved through farr_compare_fn function pointer passed as
 * argument at init time.
 * Depending on user's compare implementation, fwk_heap_peek() will therefore
 * return smallest node for a min-heap, greatest one for a max-heap, or anything
 * else if not used / implemented in a consistent manner.
 *
 * @return pointer to first node
 *
 * @warning Behavior is undefined if @p heap is empty.
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_extract(struct fwk_heap *heap, char *node);

/**
 * Clear content of specified fwk_heap
 *
 * @param heap heap to clear
 * 
 * Reset heap to empty state.
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_clear(struct fwk_heap *heap);

/**
 * Build / heapify a fwk_heap initialized with unsorted data
 *
 * @param heap  heap to heapify
 * @param count count of nodes to heapify
 *
 * Build @p heap fwk_heap from an the array passed as argument to
 * fwk_heap_init() according to Floyd algorithm in O(n) time complexity.
 *
 * @warning Behavior is undefined if @p count is zero.
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_build(struct fwk_heap *heap, unsigned int count);

/**
 * Initialize a fwk_heap
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
 * @ingroup fwk_heap
 */
extern int fwk_heap_init(struct fwk_heap *heap,
                         char            *nodes,
                         size_t           node_size,
                         unsigned int     node_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy);

/**
 * Release resources allocated for a fwk_heap
 *
 * @param heap heap to release resources for
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_fini(struct fwk_heap *heap);

/**
 * Create a fwk_heap
 *
 * @param node_size size in bytes of a single node sitting into @p heap
 * @param node_nr   maximum number of nodes @p heap may contain
 * @param compare   comparison function used to locate the right array slot to
 *                  insert data into
 * @param copy      copy function used to swap nodes / array slots.
 *
 * Wrapper allocating and initializing a fwk_heap.
 *
 * @warning Behavior is undefined when called with a zero @p node_nr or a zero
 * @p node_size.
 *
 * @return pointer to new created weak heap
 *
 * @ingroup fwk_heap
 */
extern struct fwk_heap * fwk_heap_create(size_t           node_size,
                                         unsigned int     node_nr,
                                         farr_compare_fn *compare,
                                         farr_copy_fn    *copy);

/**
 * Release resources allocated by fwk_heap_create() for fixed length array
 * based weak heap
 *
 * @param heap heap to release resources for
 *
 * @ingroup fwk_heap
 */
extern void fwk_heap_destroy(struct fwk_heap *heap);

#if defined(CONFIG_KARN_FWK_HEAP_SORT)

/**
 * Sort array passed as argument according to weak heap sort scheme.
 *
 * @param entries    array of entries to sort
 * @param entry_size size in bytes of a single array entry
 * @param entry_nr   number of array entries
 * @param compare    comparison function used to locate the right array slot to
 *                   insert data into
 * @param copy       copy function used to swap nodes / array slots.
 *
 * @ingroup fwk_heap
 */
extern int fwk_heap_sort(char            *entries,
                         size_t           entry_size,
                         size_t           entry_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy);

#endif /* defined(CONFIG_KARN_FWK_HEAP_SORT) */

#endif /* _KARN_FWK_HEAP_H */
