/**
 * @file      bnm_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Binomial heap unit tests implementation
 *
 * @defgroup bnmhut Binomial heap unit tests
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

#include "bnm_heap.h"
#include <cute/cute.h>

struct bnmhut_node {
	struct bnm_heap_node heap;
	int                  key;
};

#define BNMHUT_INIT_NODE(_key)                                             \
	{                                                                  \
		.heap = {                                                  \
			.bnm_parent  = (struct bnm_heap_node *)0xdeadbeef, \
			.bnm_eldest  = (struct bnm_heap_node *)0xdeadbeef, \
			.bnm_sibling = (struct bnm_heap_node *)0xdeadbeef, \
			.bnm_order   = 0xdeadbeef                          \
		},                                                         \
		.key = _key                                                \
	}

static struct bnm_heap bnmhut_heap;

static int bnmhut_compare_min(const struct bnm_heap_node *first,
                              const struct bnm_heap_node *second)
{
	return ((struct bnmhut_node *)first)->key -
	       ((struct bnmhut_node *)second)->key;
}

static CUTE_PNP_SUITE(bnmhut, NULL);

static void
bnmhut_setup_empty(void)
{
	bnm_heap_init(&bnmhut_heap);
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_empty, &bnmhut, bnmhut_setup_empty, NULL);

CUTE_PNP_TEST(bnmhut_check_emptiness, &bnmhut_empty)
{
	cute_ensure(bnm_heap_empty(&bnmhut_heap) == true);
}

CUTE_PNP_TEST(bnmhut_insert_single, &bnmhut_empty)
{
	struct bnmhut_node node = BNMHUT_INIT_NODE(2);

	bnm_heap_insert(&bnmhut_heap, &node.heap, bnmhut_compare_min);

	cute_ensure(bnm_heap_count(&bnmhut_heap) == 1U);
	cute_ensure(bnmhut_heap.bnm_trees == &node.heap);
	cute_ensure(node.heap.bnm_parent == NULL);
	cute_ensure(node.heap.bnm_eldest == NULL);
	cute_ensure(node.heap.bnm_sibling == NULL);
	cute_ensure(node.heap.bnm_order == 0);
}
