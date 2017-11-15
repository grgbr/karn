/**
 * @file      spair_heap.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      15 Nov 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based pairing heap interface
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

#ifndef _KARN_SPAIR_HEAP_H
#define _KARN_SPAIR_HEAP_H

#include <utils.h>
#include <stdint.h>
#include <stdbool.h>

struct spair_heap_node {
	struct spair_heap_node *spair_youngest;
	struct spair_heap_node *spair_sibling;
} __align(sizeof(uintptr_t));

struct spair_heap {
	struct spair_heap_node *spair_root;
	unsigned int            spair_count;
};

#define spair_heap_assert(_heap)                            \
	assert(_heap);                                      \
	assert(!(_heap)->spair_root ^ (_heap)->spair_count)

#define SPAIR_HEAP_INIT(_heap) \
	{ .spair_root = NULL, .spair_count = 0 }

typedef int (spair_heap_compare_fn)(const struct spair_heap_node *restrict first,
                                    const struct spair_heap_node *restrict second);

static inline unsigned int spair_heap_count(const struct spair_heap* heap)
{
	spair_heap_assert(heap);

	return heap->spair_count;
}

static inline bool spair_heap_empty(const struct spair_heap* heap)
{
	spair_heap_assert(heap);

	return heap->spair_count == 0;
}

static inline struct spair_heap_node *
spair_heap_peek(const struct spair_heap *heap)
{
	spair_heap_assert(heap);

	return heap->spair_root;
}

extern void spair_heap_insert(struct spair_heap      *heap,
                              struct spair_heap_node *node,
                              spair_heap_compare_fn  *compare);

extern struct spair_heap_node *
spair_heap_extract(struct spair_heap *heap, spair_heap_compare_fn *compare);

extern void spair_heap_remove(struct spair_heap      *heap,
                              struct spair_heap_node *node,
                              spair_heap_compare_fn  *compare);

extern void spair_heap_merge(struct spair_heap     *result,
                             struct spair_heap     *heap,
                             spair_heap_compare_fn *compare);

extern void spair_heap_update(struct spair_heap      *heap,
                              struct spair_heap_node *key,
                              spair_heap_compare_fn  *compare);

extern void spair_heap_init(struct spair_heap *heap);

static inline void spair_heap_fini(struct spair_heap *heap __unused)
{
	spair_heap_assert(heap);
}

#endif
