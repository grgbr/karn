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

#include <karn/lcrs.h>

struct sbnm_heap_node {
	struct lcrs_node sbnm_lcrs;
	unsigned int     sbnm_rank;
};

#define sbnm_heap_entry(_node, _type, _member) \
	containerof(_node, _type, _member)

static inline struct sbnm_heap_node *
sbnm_heap_node_from_lcrs(const struct lcrs_node *node)
{
	karn_assert(node);

	return lcrs_entry(node, struct sbnm_heap_node, sbnm_lcrs);
}

typedef int (sbnm_heap_compare_fn)(const struct sbnm_heap_node *first,
                                   const struct sbnm_heap_node *second);

struct sbnm_heap {
	unsigned int          sbnm_count;
	struct lcrs_node     *sbnm_roots;
	sbnm_heap_compare_fn *sbnm_compare;
};

#define sbnm_heap_assert(_heap) \
	karn_assert(_heap); \
	karn_assert(lcrs_istail((_heap)->sbnm_roots) ^ (_heap)->sbnm_count); \
	karn_assert((_heap)->sbnm_compare)

#define SBNM_HEAP_INIT(_heap, _compare) \
	{ \
		.sbnm_count   = 0, \
		.sbnm_roots   = LCRS_TAIL, \
		.sbnm_compare = _compare \
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

extern void sbnm_heap_merge(struct sbnm_heap *result, struct sbnm_heap *source);

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
	karn_assert(heap);
	karn_assert(compare);

	heap->sbnm_count = 0;
	heap->sbnm_roots = lcrs_mktail(NULL);
	heap->sbnm_compare = compare;
}

static inline void sbnm_heap_fini(struct sbnm_heap *heap __unused)
{
	sbnm_heap_assert(heap);
}

#endif /* _KARN_SBNM_HEAP_H */
