/**
 * @file      bnm_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Binomial heap interface
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

#ifndef _KARN_BNM_HEAP_H
#define _KARN_BNM_HEAP_H

#include "dlist.h"
#include <assert.h>

struct bnm_heap_node {
	struct dlist_node     bnm_sibling;
	struct bnm_heap_node *bnm_parent;
	struct dlist_node    *bnm_child;
	unsigned int          bnm_order;
};

#define bnm_heap_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

struct bnm_heap {
	struct dlist_node bnm_roots;
	unsigned int      bnm_count;
};

#define bnm_heap_assert(_heap)                                        \
	assert(_heap);                                                \
	assert(dlist_empty(&(_heap)->bnm_roots) ^ (_heap)->bnm_count)

#define BNM_HEAP_INIT(_heap) \
	{ .bnm_roots = DLIST_INIT((_heap).bnm_roots), .bnm_count = 0 }

typedef int (bnm_heap_compare_fn)(const struct bnm_heap_node *first,
                                  const struct bnm_heap_node *second);

extern void bnm_heap_insert(struct bnm_heap      *heap,
                            struct bnm_heap_node *key,
                            bnm_heap_compare_fn  *compare);

extern struct bnm_heap_node * bnm_heap_peek(const struct bnm_heap *heap,
                                            bnm_heap_compare_fn   *compare);

extern struct bnm_heap_node * bnm_heap_extract(struct bnm_heap     *heap,
                                               bnm_heap_compare_fn *compare);

extern void bnm_heap_merge_trees(struct dlist_node   *first,
                                 struct dlist_node   *second,
                                 bnm_heap_compare_fn *compare);

extern void bnm_heap_update(struct bnm_heap_node *key,
                            bnm_heap_compare_fn  *compare);

extern void bnm_heap_remove(struct bnm_heap      *heap,
                            struct bnm_heap_node *key,
                            bnm_heap_compare_fn  *compare);

static inline unsigned int
bnm_heap_count(const struct bnm_heap* heap)
{
	bnm_heap_assert(heap);

	return heap->bnm_count;
}

static inline bool
bnm_heap_empty(const struct bnm_heap* heap)
{
	bnm_heap_assert(heap);

	return heap->bnm_count == 0;
}

static inline void
bnm_heap_merge(struct bnm_heap     *result,
               struct bnm_heap     *source,
               bnm_heap_compare_fn *compare)
{
	bnm_heap_merge_trees(&result->bnm_roots, &source->bnm_roots, compare);

	result->bnm_count += source->bnm_count;
}

static inline void
bnm_heap_init(struct bnm_heap* heap)
{
	assert(heap);

	dlist_init(&heap->bnm_roots);
	heap->bnm_count = 0;
}

#endif
