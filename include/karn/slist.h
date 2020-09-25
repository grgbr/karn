/**
 * @file      slist.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      27 Jun 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list interface
 *
 * @defgroup slist Singly linked list
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

#ifndef _KARN_SLIST_H
#define _KARN_SLIST_H

#include <karn/common.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * Singly linked list node
 *
 * Describes a single entry linked into a slist.
 *
 * @ingroup slist
 */
struct slist_node {
	/** Node following this node into a slist. */
	struct slist_node *slist_next;
};

/**
 * Singly linked list
 *
 * @ingroup slist
 */
struct slist {
	/** slist head allowing to locate the first linked node. */
	struct slist_node  slist_head;
	/** Last entry in this slist. */
	struct slist_node *slist_tail;
};

/**
 * slist constant initializer.
 *
 * @param _list slist variable to initialize.
 *
 * @ingroup slist
 */
#define SLIST_INIT(_list)                                     \
	{                                                     \
		.slist_head.slist_next = NULL,                \
		.slist_tail            = &(_list)->slist_head \
	}

/**
 * Initialize a slist
 *
 * @param list slist to initialize.
 *
 * @ingroup slist
 */
static inline void slist_init(struct slist *list)
{
	karn_assert(list);

	list->slist_head.slist_next = NULL;
	list->slist_tail = &list->slist_head;
}

/**
 * Test wether a slist is empty or not.
 *
 * @param list slist to test.
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup slist
 */
static inline bool slist_empty(const struct slist *list)
{
	karn_assert(list);
	karn_assert(list->slist_tail);

	return list->slist_head.slist_next == NULL;
}

/**
 * Return slist head.
 *
 * @param list slist to get head from.
 *
 * @return Pointer to list's head node.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_head(struct slist *list)
{
	karn_assert(list);
	karn_assert(list->slist_tail);

	return &list->slist_head;
}

/**
 * Get node following specified node.
 *
 * @param node Node which to get the successor from.
 *
 * @return Pointer to following node.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_next(const struct slist_node *node)
{
	karn_assert(node);

	return node->slist_next;
}

/**
 * Get first node of specified slist.
 *
 * @param list slist to get the first node from.
 *
 * @retval Pointer to first list's node.
 * @retval NULL means list is empty.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_first(const struct slist *list)
{
	karn_assert(!slist_empty(list));

	return list->slist_head.slist_next;
}

/**
 * Get last node of specified slist.
 *
 * @param list slist to get the last node from.
 *
 * @retval Pointer to last list's node.
 * @retval Pointer to list's head node means list is empty.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @see slist_head()
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_last(const struct slist *list)
{
	karn_assert(!slist_empty(list));

	return list->slist_tail;
}

/**
 * Add a node into a slist.
 *
 * @param list     slist to add node to.
 * @param previous Node preceding the one to add.
 * @param node     Node to add.
 *
 * @ingroup slist
 */
static inline void slist_append(struct slist      *list,
                                struct slist_node *restrict previous,
                                struct slist_node *restrict node)
{
	karn_assert(list);
	karn_assert(!list->slist_head.slist_next || list->slist_tail);
	karn_assert(previous);
	karn_assert(node);

	if (!previous->slist_next)
		/* Update tail pointer if previous points to last node. */
		list->slist_tail = node;

	node->slist_next = previous->slist_next;
	previous->slist_next = node;
}

/**
 * Remove a node from a slist.
 *
 * @param list     slist to remove node from.
 * @param previous Node preceding the one to remove.
 * @param node     Node to remove.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline void slist_remove(struct slist            *list,
                                struct slist_node       *restrict previous,
                                const struct slist_node *restrict node)
{
	karn_assert(!slist_empty(list));
	karn_assert(previous);
	karn_assert(node);
	karn_assert(previous->slist_next == node);

	if (!node->slist_next)
		list->slist_tail = previous;

	previous->slist_next = node->slist_next;
}

/**
 * Move slist node from one location to another.
 *
 * @param list     slist to insert node into
 * @param at       list's node to insert node after
 * @param previous Node preceding the node to move
 * @param node     Node to move
 *
 * @ingroup slist
 */
extern void slist_move(struct slist      *list,
                       struct slist_node *restrict at,
                       struct slist_node *restrict previous,
                       struct slist_node *restrict node);

/**
 * Add a node to the end of a slist.
 *
 * @param list     slist to add node to.
 * @param node     Node to enqueue.
 *
 * @ingroup slist
 */
static inline void slist_nqueue(struct slist      *list,
                                struct slist_node *restrict node)
{
	karn_assert(list);
	karn_assert(!list->slist_head.slist_next || list->slist_tail);
	karn_assert(node);

	node->slist_next = NULL;

	list->slist_tail->slist_next = node;
	list->slist_tail = node;
}

/**
 * Remove a node of the begining of a slist and return it.
 *
 * @param list     slist to dequeue node from.
 *
 * @return Pointer to dequeued node.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline struct slist_node * slist_dqueue(struct slist *list)
{
	karn_assert(!slist_empty(list));
	karn_assert(list->slist_tail);

	struct slist_node *node = list->slist_head.slist_next;

	list->slist_head.slist_next = node->slist_next;

	if (!node->slist_next)
		list->slist_tail = &list->slist_head;

	return node;
}

/**
 * Extract / remove a portion of nodes from a slist.
 *
 * @param list  slist to remove nodes from.
 * @param first Node preceding the first node of the portion to remove.
 * @param last  Last node of portion to remove.
 *
 * @warning Behavior is undefined when called on an empty slist.
 *
 * @ingroup slist
 */
static inline void slist_withdraw(struct slist            *list,
                                  struct slist_node       *restrict first,
                                  const struct slist_node *restrict last)
{
	karn_assert(list);
	karn_assert(!slist_empty(list));
	karn_assert(first);
	karn_assert(last);

	first->slist_next = last->slist_next;

	if (!last->slist_next)
		list->slist_tail = first;
}

/**
 * Insert a portion of nodes into a slist.
 *
 * @param list  slist to insert nodes into.
 * @param at    Node after which portion is inserted.
 * @param first First node of portion to insert.
 * @param last  Last node of portion to insert.
 *
 * @ingroup slist
 */
static inline void slist_embed(struct slist      *list,
                               struct slist_node *restrict at,
                               struct slist_node *restrict first,
                               struct slist_node *restrict last)
{
	karn_assert(list);
	karn_assert(!list->slist_head.slist_next || list->slist_tail);
	karn_assert(at);
	karn_assert(first);
	karn_assert(last);

	last->slist_next = at->slist_next;
	if (!last->slist_next)
		list->slist_tail = last;

	at->slist_next = first;

}

/**
 * Extract source list portion and move it to result list at specified location.
 *
 * @param result slist to insert nodes into.
 * @param at     result's node to insert nodes after.
 * @param source slist to extract nodes from.
 * @param first  Node preceding the nodes portion to move.
 * @param last   Last portions's node to move.
 *
 * @warning Behavior is undefined when called on an empty @p source slist.
 *
 * @ingroup slist
 */
extern void
slist_splice(struct slist      *restrict result,
             struct slist_node *restrict at,
             struct slist      *restrict source,
             struct slist_node *restrict first,
             struct slist_node *restrict last);

/**
 * Iterate over slist nodes.
 *
 * @param _list  slist to iterate over.
 * @param _node  Pointer to current node.
 *
 * @ingroup slist
 */
#define slist_foreach_node(_list, _node)             \
	for (_node = (_list)->slist_head.slist_next; \
	     _node;                                  \
	     _node = _node->slist_next)

/**
 * Return type casted pointer to entry containing specified node.
 *
 * @param _node   slist node to retrieve container from.
 * @param _type   Type of container
 * @param _member Member field of container structure pointing to _node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

/**
 * Return type casted pointer to entry containing first node of specified slist.
 *
 * @param _list   slist to get the first node from.
 * @param _type   Type of container.
 * @param _member Member field of container structure pointing to first _list's
 *                node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_first_entry(_list, _type, _member) \
	slist_entry(slist_first(_list), _type, _member)

/**
 * Return type casted pointer to entry containing last node of specified slist.
 *
 * @param _list   slist to get the last node from.
 * @param _type   Type of container.
 * @param _member Member field of container structure pointing to last _list's
 *                node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup slist
 */
#define slist_last_entry(_list, _type, _member) \
	slist_entry(slist_last(_list), _type, _member)


/**
 * Return type casted pointer to entry following specified entry.
 *
 * @param _entry  Entry containing slist node.
 * @param _member Member field of container structure pointing to slist node.
 *
 * @return Pointer to following entry.
 *
 * @ingroup slist
 */
#define slist_next_entry(_entry, _member) \
	slist_entry(slist_next(&(_entry)->_member), typeof(*(_entry)), _member)

/**
 * Iterate over slist node container entries.
 *
 * @param _list   slist to iterate over.
 * @param _entry  Pointer to entry containing @p _list's current node.
 * @param _member Member field of container structure pointing to slist node.
 *
 * @ingroup slist
 */
#define slist_foreach_entry(_list, _entry, _member)               \
	for (_entry = slist_entry((_list)->slist_head.slist_next, \
	                          typeof(*(_entry)), _member);    \
	     &(_entry)->_member;                                  \
	     _entry = slist_next_entry(_entry, _member))

/******************************************************************************
 * Performance monitoring
 ******************************************************************************/

#if defined(CONFIG_KARN_SLIST_PERF_EVENTS)

/* slist performance counters */
struct slist_perf_events {
	/*
	 * Number of comparisons performed since last call to
	 * slist_clear_perf_events().
	 */
	unsigned long long compare;
	/*
	 * Number of entry swaps performed since last call to
	 * slist_clear_perf_events().
	 */
	unsigned long long swap;
};

/*
 * Retrieve slist performance counters.
 *
 * Return: pointer to structure holding accounted performance events since last
 *         call to slist_fetch_perf_events().
 *
 * See: slist_clear_perf_events()
 */
extern const struct slist_perf_events * slist_fetch_perf_events(void);

/*
 * Clear slist perfomance counters, i.e., set accounted performance counters to
 * 0.
 */
extern void slist_clear_perf_events(void);

#else /* !defined(CONFIG_KARN_SLIST_PERF_EVENTS) */

struct slist_perf_events { };

static inline void slist_clear_perf_events(void)
{
}

static inline const struct slist_perf_events * slist_fetch_perf_events(void)
{
	return NULL;
}

#endif /* defined(CONFIG_KARN_SLIST_PERF_EVENTS) */

/******************************************************************************
 * Slist sorting
 ******************************************************************************/

/**
 * @typedef slist_compare_fn
 *
 * slist item comparison function prototype.
 *
 * @param first  node to compare with @p second
 * @param second node to compare with @p first
 *
 * @retval <0 @p first precedes @p second
 * @retval 0  @p first and @p second are equal
 * @retval >0 @p first follows @p second
 *
 * @ingroup slist
 */
typedef int (slist_compare_fn)(const struct slist_node *restrict first,
                               const struct slist_node *restrict second);

#if defined(CONFIG_KARN_SLIST_INSERTION_SORT)
/**
 * Sort specified list according to the insertion sort scheme.
 *
 * @param list    slist to sort.
 * @param compare Comparison function used to compute order between 2 nodes.
 *
 * @ingroup slist
 */
extern void slist_insertion_sort(struct slist *list, slist_compare_fn *compare);

/**
 * Sort a specified number of nodes from a slist according to the insertion
 * sort scheme and move sorted nodes into a result slist.
 *
 * @param result  Result slist where sorted nodes are moved.
 * @param source  Source slist to sort.
 * @param count   Maximum number of source nodes to sort.
 * @param compare Comparison function used to compute order between 2 nodes.
 *
 * @ingroup slist
 */
extern void slist_counted_insertion_sort(struct slist     *restrict result,
                                         struct slist     *restrict source,
                                         unsigned int      count,
                                         slist_compare_fn *compare);
#endif /* defined(CONFIG_KARN_SLIST_INSERTION_SORT) */

#if defined(CONFIG_KARN_SLIST_SELECTION_SORT)
/**
 * Sort specified list according to the insertion sort scheme.
 *
 * @param list    slist to sort.
 * @param compare Comparison function used to compute order between 2 nodes.
 *
 * @ingroup slist
 */
extern void slist_selection_sort(struct slist *list, slist_compare_fn *compare);
#endif /* defined(CONFIG_KARN_SLIST_SELECTION_SORT) */

#if defined(CONFIG_KARN_SLIST_BUBBLE_SORT)
/**
 * Sort specified list according to the bubble sort scheme.
 *
 * @param list    slist to sort.
 * @param compare Comparison function used to compute order between 2 nodes.
 *
 * @ingroup slist
 */
extern void slist_bubble_sort(struct slist *list, slist_compare_fn *compare);
#endif /* defined(CONFIG_KARN_SLIST_BUBBLE_SORT) */

#if defined(CONFIG_KARN_SLIST_MERGE_SORT)
/**
 * Sort 2 presorted slists into a single one.
 *
 * @param result   slist within which both slists will be sorted into.
 * @param source   Source slist to sort within result.
 * @param compare Comparison function used to compute order between 2 nodes.
 *
 * @warning Behavior is undefined when called on an empty @p source slist.
 *
 * @ingroup slist
 */
extern void slist_merge_presort(struct slist     *result,
                                struct slist     *source,
                                slist_compare_fn *compare);

/**
 * Sort specified list according to an hybrid sort scheme mixing insertion and
 * merge sort algorithms.
 *
 * @param list     slist to sort.
 * @param run_len  Primary sorting run length in number of nodes.
 * @param nodes_nr Number of nodes linked into list.
 * @param compare  Comparison function used to compute order between 2 nodes.
 *
 * Description
 * ----------
 *
 * Use insertion scheme to sort sublists shorter than @p run_len nodes then
 * switch to merge sort strategy for longer ones.
 *
 * Merge sorting implementation is based on an original idea attributed to
 * Jon McCarthy and described
 * [here](http://richardhartersworld.com/cri/2007/schoen.html).
 *
 * The whole point is : avoid excessive scanning of the list during sublist
 * splitting phases by using an additional bounded auxiliary space to store
 * sublist heads.
 * The whole process is performed in an iterative manner.
 *
 * __Time complexity__
 * worst       | average     | best
 * ------------|-------------|-----
 * O(n.log(n)) | O(n.log(n)) | O(n)
 *
 * __Auxiliary space__: allocated on stack
 * worst                | average   | best
 * ---------------------|-----------|--------
 * 27 slists (54 words) | O(log(n)) | 0 words
 *
 * __Stability__: yes
 *
 * @warning Behavior is undefined when called on an empty slist.
 * @warning Behavior is undefined when called with @p run_len < 1.
 * @warning Behavior is undefined when called with @p nodes_nr < 1.
 *
 * @ingroup slist
 */
extern void slist_hybrid_merge_sort(struct slist     *list,
                                    unsigned int      run_len,
                                    unsigned int      nodes_nr,
                                    slist_compare_fn *compare);

/**
 * Sort specified list according to an hybrid merge sort scheme.
 *
 * @param list     slist to sort.
 * @param nodes_nr Number of nodes linked into list.
 * @param compare  Comparison function used to compute order between 2 nodes.
 *
 * Basically a wrapper around slist_hybrid_merge_sort() implementing a heuristic
 * to automatically compute a proper value for the initial run length.
 *
 * @warning Behavior is undefined when called on an empty slist.
 * @warning Behavior is undefined when called with @p nodes_nr < 1.
 *
 * @see slist_hybrid_merge_sort()
 *
 * @ingroup slist
 */
extern void slist_merge_sort(struct slist     *list,
                             unsigned int      nodes_nr,
                             slist_compare_fn *compare);
#endif /* defined(CONFIG_KARN_SLIST_MERGE_SORT) */

#endif /* _KARN_SLIST_H */
