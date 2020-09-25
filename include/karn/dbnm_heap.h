/**
 * @file      dbnm_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list based binomial heap interface
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

#ifndef _KARN_DBNM_HEAP_H
#define _KARN_DBNM_HEAP_H

#include <karn/dlist.h>

struct dbnm_heap_node {
	struct dlist_node      dbnm_sibling;
	struct dbnm_heap_node *dbnm_parent;
	struct dlist_node     *dbnm_child;
	unsigned int           dbnm_order;
};

#define dbnm_heap_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

struct dbnm_heap {
	struct dlist_node dbnm_roots;
	unsigned int      dbnm_count;
};

#define dbnm_heap_assert(_heap) \
	karn_assert(_heap); \
	karn_assert(dlist_empty(&(_heap)->dbnm_roots) ^ (_heap)->dbnm_count)

#define DBNM_HEAP_INIT(_heap)                                 \
	{                                                     \
		.dbnm_roots = DLIST_INIT((_heap).dbnm_roots), \
		.dbnm_count = 0                               \
	}

typedef int (dbnm_heap_compare_fn)(const struct dbnm_heap_node *restrict first,
                                   const struct dbnm_heap_node *restrict second);

extern struct dbnm_heap_node * dbnm_heap_peek(const struct dbnm_heap *heap,
                                              dbnm_heap_compare_fn   *compare);

extern struct dbnm_heap_node * dbnm_heap_extract(struct dbnm_heap     *heap,
                                                 dbnm_heap_compare_fn *compare);

extern void dbnm_heap_insert(struct dbnm_heap      *heap,
                             struct dbnm_heap_node *key,
                             dbnm_heap_compare_fn  *compare);

extern void dbnm_heap_remove(struct dbnm_heap      *heap,
                             struct dbnm_heap_node *key,
                             dbnm_heap_compare_fn  *compare);

extern void dbnm_heap_update(struct dbnm_heap_node *key,
                             dbnm_heap_compare_fn  *compare);

static inline unsigned int
dbnm_heap_count(const struct dbnm_heap* heap)
{
	dbnm_heap_assert(heap);

	return heap->dbnm_count;
}

static inline bool
dbnm_heap_empty(const struct dbnm_heap* heap)
{
	dbnm_heap_assert(heap);

	return heap->dbnm_count == 0;
}

extern void dbnm_heap_merge_trees(struct dlist_node    *first,
                                  struct dlist_node    *second,
                                  dbnm_heap_compare_fn *compare);

static inline void
dbnm_heap_merge(struct dbnm_heap     *result,
                struct dbnm_heap     *source,
                dbnm_heap_compare_fn *compare)
{
	karn_assert(result);
	karn_assert(source);

	dbnm_heap_merge_trees(&result->dbnm_roots, &source->dbnm_roots,
	                      compare);

	result->dbnm_count += source->dbnm_count;
}

static inline void
dbnm_heap_init(struct dbnm_heap* heap)
{
	karn_assert(heap);

	dlist_init(&heap->dbnm_roots);
	heap->dbnm_count = 0;
}

#endif /* _KARN_DBNM_HEAP_H */
