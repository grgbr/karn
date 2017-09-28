/**
 * @file      sbnm_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based binomial heap interface
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

#ifndef _KARN_SBNM_HEAP_H
#define _KARN_SBNM_HEAP_H

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

struct sbnm_heap_node {
	struct sbnm_heap_node *sbnm_eldest;
	struct sbnm_heap_node *sbnm_sibling;
	struct sbnm_heap_node *sbnm_parent;
	unsigned int           sbnm_order;
};

#define sbnm_heap_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

struct sbnm_heap {
	struct sbnm_heap_node *sbnm_trees;
	struct sbnm_heap_node *sbnm_next;
	unsigned int           sbnm_count;
};

#define sbnm_heap_assert(_heap)                            \
	assert(_heap);                                     \
	assert(!(_heap)->sbnm_trees ^ (_heap)->sbnm_count)

#define SBNM_HEAP_INIT(_heap) \
	{ .sbnm_trees = NULL, .sbnm_count = 0 }

typedef int (sbnm_heap_compare_fn)(const struct sbnm_heap_node *restrict first,
                                   const struct sbnm_heap_node *restrict second);

extern struct sbnm_heap_node * sbnm_heap_extract(struct sbnm_heap     *heap,
                                                 sbnm_heap_compare_fn *compare);

extern void sbnm_heap_insert(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key,
                             sbnm_heap_compare_fn  *compare);

extern void sbnm_heap_update(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key,
                             sbnm_heap_compare_fn  *compare);

extern void sbnm_heap_remove(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key,
                             sbnm_heap_compare_fn  *compare);

static inline unsigned int
sbnm_heap_count(const struct sbnm_heap* heap)
{
	sbnm_heap_assert(heap);

	return heap->sbnm_count;
}

static inline bool
sbnm_heap_empty(const struct sbnm_heap* heap)
{
	sbnm_heap_assert(heap);

	return heap->sbnm_count == 0;
}

static inline struct sbnm_heap_node *
sbnm_heap_peek(const struct sbnm_heap *heap)
{
	assert(heap);
	assert(heap->sbnm_trees);
	assert(heap->sbnm_count);
	assert(!heap->sbnm_next->sbnm_parent);

	return heap->sbnm_next;
}

extern void
sbnm_heap_merge_trees(struct sbnm_heap      *result,
                      struct sbnm_heap_node *tree,
                      sbnm_heap_compare_fn  *compare);

static inline void
sbnm_heap_merge(struct sbnm_heap     *result,
                struct sbnm_heap     *source,
                sbnm_heap_compare_fn *compare)
{
	assert(result);
	assert(result->sbnm_trees);
	assert(result->sbnm_count);
	assert(source);
	assert(source->sbnm_trees);
	assert(source->sbnm_count);

	sbnm_heap_merge_trees(result, source->sbnm_trees, compare);

	result->sbnm_count += source->sbnm_count;
}

static inline void
sbnm_heap_init(struct sbnm_heap* heap)
{
	assert(heap);

	heap->sbnm_trees = NULL;
	heap->sbnm_count = 0;
}

#endif
