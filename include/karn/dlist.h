/**
 * @file      dlist.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list interface
 *
 * @defgroup dlist Doubly linked list
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

#ifndef _KARN_DLIST_H
#define _KARN_DLIST_H

#include <karn/common.h>
#include <stdbool.h>

/**
 * Doubly linked list node
 *
 * Describes a single entry linked into a dlist.
 *
 * @ingroup dlist
 */
struct dlist_node {
	/** Node following this node into a dlist. */
	struct dlist_node *dlist_next;
	/** Node preceding this node into a dlist. */
	struct dlist_node *dlist_prev;
};

/**
 * dlist_node constant initializer.
 *
 * @param _node dlist_node variable to initialize.
 *
 * @ingroup dlist
 */
#define DLIST_INIT(_node)               \
	{                               \
		.dlist_next = &(_node), \
		.dlist_prev = &(_node)  \
	}

/**
 * Initialize a dlist_node
 *
 * @param node dlist_node to initialize.
 *
 * @ingroup dlist
 */
static inline void dlist_init(struct dlist_node *node)
{
	karn_assert(node);

	node->dlist_next = node;
	node->dlist_prev = node;
}

/**
 * Test wether a dlist_node is empty or not.
 *
 * @param node dlist_node to test.
 *
 * @retval true  empty
 * @retval false not empty
 *
 * @ingroup dlist
 */
static inline bool dlist_empty(const struct dlist_node *node)
{
	karn_assert(node);

	return node->dlist_next == node;
}

/**
 * Get node following specified node.
 *
 * @param node Node which to get the follower from.
 *
 * @return Pointer to following node.
 *
 * @ingroup dlist
 */
static inline struct dlist_node * dlist_next(const struct dlist_node *node)
{
	karn_assert(node);

	return node->dlist_next;
}

/**
 * Get node preceding specified node.
 *
 * @param node Node which to get the predecessor from.
 *
 * @return Pointer to preceding node.
 *
 * @ingroup dlist
 */
static inline struct dlist_node * dlist_prev(const struct dlist_node *node)
{
	karn_assert(node);

	return node->dlist_prev;
}

/**
 * Insert a dlist_node in between specified adjacent nodes.
 *
 * @param prev Node to append @p node after.
 * @param node Node to inject.
 * @param next Node to insert @p node before.
 *
 * @ingroup dlist
 */
static inline void dlist_inject(struct dlist_node *prev,
                                struct dlist_node *restrict node,
                                struct dlist_node *next)
{
	karn_assert(node);
	karn_assert(prev);
	karn_assert(next);
	karn_assert(node != prev);
	karn_assert(node != next);

	next->dlist_prev = node;

	node->dlist_next = next;
	node->dlist_prev = prev;

	prev->dlist_next = node;
}

/**
 * Insert a dlist_node before specified node.
 *
 * @param node Node to insert.
 * @param at   Node to insert @p node before.
 *
 * @ingroup dlist
 */
static inline void dlist_insert(struct dlist_node *restrict at,
                                struct dlist_node *restrict node)
{
	dlist_inject(at->dlist_prev, node, at);
}

/**
 * Append a dlist_node after specified node.
 *
 * @param node Node to append.
 * @param at   Node to append @p node after.
 *
 * @ingroup dlist
 */
static inline void dlist_append(struct dlist_node *restrict at,
                                struct dlist_node *restrict node)
{
	dlist_inject(at, node, at->dlist_next);
}

/**
 * Enqueue a dlist_node at the head of specified list.
 *
 * @param list Dummy head node designating the list.
 * @param node Node to enqueue.
 *
 * @ingroup dlist
 */
static inline void dlist_nqueue_front(struct dlist_node *restrict list,
                                      struct dlist_node *restrict node)
{
	dlist_append(list, node);
}

/**
 * Enqueue a dlist_node at the tail of specified list.
 *
 * @param list Dummy head node designating the list.
 * @param node Node to enqueue.
 *
 * @ingroup dlist
 */
static inline void dlist_nqueue_back(struct dlist_node *restrict list,
                                     struct dlist_node *restrict node)
{
	dlist_insert(list, node);
}

/**
 * Remove a dlist_node from a doubly linked list.
 *
 * @param node Node to remove.
 *
 * @ingroup dlist
 */
static inline void dlist_remove(const struct dlist_node *node)
{
	struct dlist_node *next = node->dlist_next;
	struct dlist_node *prev = node->dlist_prev;

	prev->dlist_next = next;
	next->dlist_prev = prev;
}

/**
 * Remove then reinitialize a dlist_node from a doubly linked list.
 *
 * @param node Node to remove.
 *
 * @ingroup dlist
 */
static inline void dlist_remove_init(struct dlist_node *node)
{
	dlist_remove(node);
	dlist_init(node);
}

/**
 * Dequeue a dlist_node from the head of specified list.
 *
 * @param list Dummy head node designating the list.
 *
 * @return Pointer to dequeued node.
 *
 * @ingroup dlist
 */
static inline struct dlist_node * dlist_dqueue_front(struct dlist_node *list)
{
	struct dlist_node *node = dlist_next(list);

	dlist_remove(node);

	return node;
}

/**
 * Dequeue a dlist_node from the tail of specified list.
 *
 * @param list Dummy head node designating the list.
 *
 * @return Pointer to dequeued node.
 *
 * @ingroup dlist
 */
static inline struct dlist_node * dlist_dqueue_back(struct dlist_node *list)
{
	struct dlist_node *node = dlist_prev(list);

	dlist_remove(node);

	return node;
}

/**
 * Replace old entry by new one.
 *
 * @param old  Node to replace.
 * @param node New node to insert.
 *
 * @warning Behavior is unpredictable if @p old node is empty.
 *
 * @ingroup dlist
 */
static inline void dlist_replace(struct dlist_node *restrict old,
                                 struct dlist_node *restrict node)
{
	karn_assert(!dlist_empty(old));
	karn_assert(node);

	dlist_inject(old->dlist_prev, node, old->dlist_next);
}

/**
 * Move entry from one location to another.
 *
 * @param at   Node after which to append @p node.
 * @param node Node to move.
 *
 * @warning Behavior is unpredictable if @p old node is empty.
 *
 * @ingroup dlist
 */
static inline void dlist_move_after(struct dlist_node *restrict at,
                                    struct dlist_node *restrict node)
{
	karn_assert(at);

	dlist_remove(node);
	dlist_inject(at, node, at->dlist_next);
}

/**
 * Extract / remove a portion of nodes from a dlist.
 *
 * @param first First node of portion to remove.
 * @param last  Last node of portion to remove.
 *
 * @ingroup dlist
 */
static inline void dlist_withdraw(const struct dlist_node *first,
                                  const struct dlist_node *last)
{
	karn_assert(first);
	karn_assert(last);

	struct dlist_node *next = last->dlist_next;
	struct dlist_node *prev = first->dlist_prev;

	prev->dlist_next = next;
	next->dlist_prev = prev;
}

/**
 * Insert a portion of nodes into a dlist.
 *
 * @param at    Node after which portion is inserted.
 * @param first First node of portion to insert.
 * @param last  Last node of portion to insert.
 *
 * @ingroup dlist
 */
static inline void dlist_embed(struct dlist_node *restrict at,
                               struct dlist_node *first,
                               struct dlist_node *last)
{
	karn_assert(at);
	karn_assert(first);
	karn_assert(last);

	struct dlist_node *restrict next = at->dlist_next;

	first->dlist_prev = at;
	at->dlist_next = first;

	last->dlist_next = next;
	next->dlist_prev = last;
}

/**
 * Extract source list portion and move it to result list at specified location.
 *
 * @param at    Node to insert nodes after.
 * @param first First node of portion to move.
 * @param last  Last node of portion to move.
 *
 * @ingroup dlist
 */
extern void dlist_splice(struct dlist_node *restrict at,
                         struct dlist_node *first,
                         struct dlist_node *last);

/**
 * Iterate over a dlist_node based list .
 *
 * @param _list Dummy head node designating the list to iterate over.
 * @param _node Pointer to current node.
 *
 * @ingroup dlist
 */
#define dlist_foreach_node(_list, _node) \
	for (_node = dlist_next(_list);  \
	     _node != (_list);           \
	     _node = dlist_next(_node))

/**
 * Return type casted pointer to entry containing specified node.
 *
 * @param _node   dlist_node to retrieve container from.
 * @param _type   Type of container
 * @param _member Member field of container structure pointing to _node.
 *
 * @return Pointer to type casted entry.
 *
 * @ingroup dlist
 */
#define dlist_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

/**
 * Return type casted pointer to entry following specified entry.
 *
 * @param _entry  Entry containing dlist node.
 * @param _member Member field of container structure pointing to dlist node.
 *
 * @return Pointer to following entry.
 *
 * @ingroup dlist
 */
#define dlist_next_entry(_entry, _member) \
	dlist_entry(dlist_next(&(_entry)->_member), typeof(*(_entry)), _member)

/**
 * Return type casted pointer to entry preceding specified entry.
 *
 * @param _entry  Entry containing dlist node.
 * @param _member Member field of container structure pointing to dlist node.
 *
 * @return Pointer to preceding entry.
 *
 * @ingroup dlist
 */
#define dlist_prev_entry(_entry, _member) \
	dlist_entry(dlist_prev(&(_entry)->_member), typeof(*(_entry)), _member)

/**
 * Iterate over dlist node container entries.
 *
 * @param _list   dlist to iterate over.
 * @param _entry  Pointer to entry containing @p _list's current node.
 * @param _member Member field of container structure pointing to dlist node.
 *
 * @ingroup dlist
 */
#define dlist_foreach_entry(_list, _entry, _member)            \
	for (_entry = dlist_entry((_list)->dlist_next,         \
	                          typeof(*(_entry)), _member); \
	     &(_entry)->_member != (_list);                    \
	     _entry = dlist_next_entry(_entry, _member))

#endif /* _KARN_DLIST_H */
