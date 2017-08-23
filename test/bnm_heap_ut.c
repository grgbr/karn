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
#include "array.h"
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

CUTE_PNP_TEST(bnmhut_peek_single, &bnmhut_empty)
{
	struct bnmhut_node node = BNMHUT_INIT_NODE(2);

	bnm_heap_insert(&bnmhut_heap, &node.heap, bnmhut_compare_min);

	cute_ensure(bnm_heap_count(&bnmhut_heap) == 1U);
	cute_ensure(bnm_heap_peek(&bnmhut_heap, bnmhut_compare_min) ==
	            &node.heap);
	cute_ensure(bnm_heap_count(&bnmhut_heap) == 1U);
}

CUTE_PNP_TEST(bnmhut_extract_single, &bnmhut_empty)
{
	struct bnmhut_node node = BNMHUT_INIT_NODE(2);

	bnm_heap_insert(&bnmhut_heap, &node.heap, bnmhut_compare_min);

	cute_ensure(bnm_heap_count(&bnmhut_heap) == 1U);
	cute_ensure(bnm_heap_extract(&bnmhut_heap, bnmhut_compare_min) ==
	            &node.heap);
	cute_ensure(bnmhut_heap.bnm_trees == NULL);
	cute_ensure(bnm_heap_count(&bnmhut_heap) == 0U);
}

static void bnmhut_check_roots(const struct bnm_heap* heap, unsigned int count)
{
	const struct bnm_heap_node *node = heap->bnm_trees;
	int                         order = -1;

	while (node) {
		while (!(count & 1)) {
			count >>= 1;
			order++;
		}
		order++;
		count >>= 1;

		cute_ensure(node->bnm_parent == NULL);
		cute_ensure(node->bnm_order == (unsigned int)order);

		node = node->bnm_sibling;
	}

	cute_ensure(count == 0);
}

static void bnmhut_check_heap(struct bnm_heap     *heap,
                              struct bnmhut_node  *nodes,
                              struct bnmhut_node **checks,
                              unsigned int         count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		bnm_heap_insert(heap, &nodes[n].heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(heap) == (n + 1));
	}

	bnmhut_check_roots(heap, count);

	for (n = 0; n < count; n++) {
		const struct bnm_heap_node *node;
		const struct bnmhut_node   *check = checks[n];

		node = bnm_heap_extract(heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);
	}
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_inorder, &bnmhut, bnmhut_setup_empty,
                               NULL);

static struct bnmhut_node bnmhut_inorder_nodes[] = {
	BNMHUT_INIT_NODE(0),
	BNMHUT_INIT_NODE(1),
	BNMHUT_INIT_NODE(2),
	BNMHUT_INIT_NODE(3),
	BNMHUT_INIT_NODE(4),
	BNMHUT_INIT_NODE(5),
	BNMHUT_INIT_NODE(6),
	BNMHUT_INIT_NODE(7),
	BNMHUT_INIT_NODE(8),
	BNMHUT_INIT_NODE(9),
	BNMHUT_INIT_NODE(10),
	BNMHUT_INIT_NODE(11),
	BNMHUT_INIT_NODE(12),
	BNMHUT_INIT_NODE(13),
	BNMHUT_INIT_NODE(14),
	BNMHUT_INIT_NODE(15),
	BNMHUT_INIT_NODE(16)
};

static struct bnmhut_node *bnmhut_inorder_checks[] = {
	&bnmhut_inorder_nodes[0],
	&bnmhut_inorder_nodes[1],
	&bnmhut_inorder_nodes[2],
	&bnmhut_inorder_nodes[3],
	&bnmhut_inorder_nodes[4],
	&bnmhut_inorder_nodes[5],
	&bnmhut_inorder_nodes[6],
	&bnmhut_inorder_nodes[7],
	&bnmhut_inorder_nodes[8],
	&bnmhut_inorder_nodes[9],
	&bnmhut_inorder_nodes[10],
	&bnmhut_inorder_nodes[11],
	&bnmhut_inorder_nodes[12],
	&bnmhut_inorder_nodes[13],
	&bnmhut_inorder_nodes[14],
	&bnmhut_inorder_nodes[15],
	&bnmhut_inorder_nodes[16]
};

CUTE_PNP_TEST(bnmhut_inorder2, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 2);
}

CUTE_PNP_TEST(bnmhut_inorder3, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 3);
}

CUTE_PNP_TEST(bnmhut_inorder4, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 4);
}

CUTE_PNP_TEST(bnmhut_inorder5, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 5);
}

CUTE_PNP_TEST(bnmhut_inorder6, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 6);
}

CUTE_PNP_TEST(bnmhut_inorder7, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 7);
}

CUTE_PNP_TEST(bnmhut_inorder8, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 8);
}

CUTE_PNP_TEST(bnmhut_inorder9, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 9);
}

CUTE_PNP_TEST(bnmhut_inorder10, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 10);
}

CUTE_PNP_TEST(bnmhut_inorder11, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 11);
}

CUTE_PNP_TEST(bnmhut_inorder12, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 12);
}

CUTE_PNP_TEST(bnmhut_inorder13, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 13);
}

CUTE_PNP_TEST(bnmhut_inorder14, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 14);
}

CUTE_PNP_TEST(bnmhut_inorder15, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 15);
}

CUTE_PNP_TEST(bnmhut_inorder16, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 16);
}

CUTE_PNP_TEST(bnmhut_inorder17, &bnmhut_inorder)
{
	bnmhut_check_heap(&bnmhut_heap, bnmhut_inorder_nodes,
	                  bnmhut_inorder_checks, 17);
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_revorder, &bnmhut, bnmhut_setup_empty,
                               NULL);

static struct bnmhut_node bnmhut_revorder_nodes[] = {
	BNMHUT_INIT_NODE(16),
	BNMHUT_INIT_NODE(15),
	BNMHUT_INIT_NODE(14),
	BNMHUT_INIT_NODE(13),
	BNMHUT_INIT_NODE(12),
	BNMHUT_INIT_NODE(11),
	BNMHUT_INIT_NODE(10),
	BNMHUT_INIT_NODE(9),
	BNMHUT_INIT_NODE(8),
	BNMHUT_INIT_NODE(7),
	BNMHUT_INIT_NODE(6),
	BNMHUT_INIT_NODE(5),
	BNMHUT_INIT_NODE(4),
	BNMHUT_INIT_NODE(3),
	BNMHUT_INIT_NODE(2),
	BNMHUT_INIT_NODE(1),
	BNMHUT_INIT_NODE(0)
};

static struct bnmhut_node *bnmhut_revorder_checks[] = {
	&bnmhut_revorder_nodes[16],
	&bnmhut_revorder_nodes[15],
	&bnmhut_revorder_nodes[14],
	&bnmhut_revorder_nodes[13],
	&bnmhut_revorder_nodes[12],
	&bnmhut_revorder_nodes[11],
	&bnmhut_revorder_nodes[10],
	&bnmhut_revorder_nodes[9],
	&bnmhut_revorder_nodes[8],
	&bnmhut_revorder_nodes[7],
	&bnmhut_revorder_nodes[6],
	&bnmhut_revorder_nodes[5],
	&bnmhut_revorder_nodes[4],
	&bnmhut_revorder_nodes[3],
	&bnmhut_revorder_nodes[2],
	&bnmhut_revorder_nodes[1],
	&bnmhut_revorder_nodes[0]
};

CUTE_PNP_TEST(bnmhut_revorder2, &bnmhut_revorder)
{
	unsigned int         count = 2;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder3, &bnmhut_revorder)
{
	unsigned int         count = 3;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder4, &bnmhut_revorder)
{
	unsigned int         count = 4;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder5, &bnmhut_revorder)
{
	unsigned int         count = 5;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder6, &bnmhut_revorder)
{
	unsigned int         count = 6;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder7, &bnmhut_revorder)
{
	unsigned int         count = 7;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder8, &bnmhut_revorder)
{
	unsigned int         count = 8;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder9, &bnmhut_revorder)
{
	unsigned int         count = 9;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder10, &bnmhut_revorder)
{
	unsigned int         count = 10;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder11, &bnmhut_revorder)
{
	unsigned int         count = 11;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder12, &bnmhut_revorder)
{
	unsigned int         count = 12;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder13, &bnmhut_revorder)
{
	unsigned int         count = 13;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder14, &bnmhut_revorder)
{
	unsigned int         count = 14;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder15, &bnmhut_revorder)
{
	unsigned int         count = 15;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder16, &bnmhut_revorder)
{
	unsigned int         count = 16;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}

CUTE_PNP_TEST(bnmhut_revorder17, &bnmhut_revorder)
{
	unsigned int         count = 17;
	struct bnmhut_node **checks =
		&bnmhut_revorder_checks[array_nr(bnmhut_revorder_checks) -
		                        count];

	bnmhut_check_heap(&bnmhut_heap, bnmhut_revorder_nodes, checks, count);
}
