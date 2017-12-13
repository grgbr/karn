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

#include <lcrs.h>

struct sbnm_heap_node {
	struct lcrs_node sbnm_lcrs;
	unsigned int     sbnm_rank;
};

#define sbnm_heap_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

static inline struct sbnm_heap_node *
sbnm_heap_node_from_lcrs(const struct lcrs_node *node)
{
	assert(node);

	return lcrs_entry(node, struct sbnm_heap_node, sbnm_lcrs);
}

typedef int (sbnm_heap_compare_fn)(const struct sbnm_heap_node *restrict first,
                                   const struct sbnm_heap_node *restrict second);

struct sbnm_heap {
	unsigned int          sbnm_count;
	struct lcrs_node      sbnm_dummy;
	sbnm_heap_compare_fn *sbnm_compare;
};

#define sbnm_heap_assert(_heap)                        \
	assert(_heap);                                 \
	assert(!lcrs_has_child(&(_heap)->sbnm_dummy) ^ \
	       (_heap)->sbnm_count);                   \
	assert((_heap)->sbnm_compare)

#define SBNM_HEAP_INIT(_heap, _compare)                          \
	{                                                        \
		.sbnm_count   = 0,                               \
		.sbnm_dummy   = LCRS_INIT(&(_heap)->sbnm_dummy), \
		.sbnm_compare = _compare                         \
	}

extern struct sbnm_heap_node * sbnm_heap_peek(const struct sbnm_heap *heap);

extern void sbnm_heap_insert(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key);

extern struct sbnm_heap_node * sbnm_heap_extract(struct sbnm_heap *heap);

extern void sbnm_heap_remove(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key);

extern void sbnm_heap_promote(struct sbnm_heap      *heap,
                              struct sbnm_heap_node *key);

extern void sbnm_heap_demote(struct sbnm_heap      *heap,
                             struct sbnm_heap_node *key);

extern void sbnm_heap_merge(struct sbnm_heap *restrict result,
                            struct sbnm_heap *restrict source);

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

static inline void
sbnm_heap_init(struct sbnm_heap *heap, sbnm_heap_compare_fn *compare)
{
	assert(heap);
	assert(compare);

	heap->sbnm_count = 0;
	lcrs_init(&heap->sbnm_dummy);
	heap->sbnm_compare = compare;
}

static inline void sbnm_heap_fini(struct sbnm_heap *heap __unused)
{
	sbnm_heap_assert(heap);
}

#endif
