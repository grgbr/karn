/**
 * @file      sbnm_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based binomial heap unit tests implementation
 *
 * @defgroup sbnmhut Singly linked list based binomial heap unit tests
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

#include "sbnm_heap.h"
#include "array.h"
#include <cute/cute.h>

struct sbnmhut_node {
	struct sbnm_heap_node heap;
	int                   key;
};

#define SBNMHUT_INIT_NODE(_key)                                              \
	{                                                                    \
		.heap = {                                                    \
			.sbnm_parent  = (struct sbnm_heap_node *)0xdeadbeef, \
			.sbnm_eldest  = (struct sbnm_heap_node *)0xdeadbeef, \
			.sbnm_sibling = (struct sbnm_heap_node *)0xdeadbeef, \
			.sbnm_order   = 0xdeadbeef                           \
		},                                                           \
		.key = _key                                                  \
	}

static struct sbnm_heap sbnmhut_heap;

static int sbnmhut_compare_min(const struct sbnm_heap_node *first,
                               const struct sbnm_heap_node *second)
{
	return ((struct sbnmhut_node *)first)->key -
	       ((struct sbnmhut_node *)second)->key;
}

static CUTE_PNP_SUITE(sbnmhut, NULL);

static void
sbnmhut_setup_empty(void)
{
	sbnm_heap_init(&sbnmhut_heap);
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_empty, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(sbnmhut_check_emptiness, &sbnmhut_empty)
{
	cute_ensure(sbnm_heap_empty(&sbnmhut_heap) == true);
}

CUTE_PNP_TEST(sbnmhut_insert_single, &sbnmhut_empty)
{
	struct sbnmhut_node node = SBNMHUT_INIT_NODE(2);

	sbnm_heap_insert(&sbnmhut_heap, &node.heap, sbnmhut_compare_min);

	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 1U);
	cute_ensure(sbnmhut_heap.sbnm_trees == &node.heap);
	cute_ensure(node.heap.sbnm_parent == NULL);
	cute_ensure(node.heap.sbnm_eldest == NULL);
	cute_ensure(node.heap.sbnm_sibling == NULL);
	cute_ensure(node.heap.sbnm_order == 0);
}

CUTE_PNP_TEST(sbnmhut_peek_single, &sbnmhut_empty)
{
	struct sbnmhut_node node = SBNMHUT_INIT_NODE(2);

	sbnm_heap_insert(&sbnmhut_heap, &node.heap, sbnmhut_compare_min);

	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 1U);
	cute_ensure(sbnm_heap_peek(&sbnmhut_heap) == &node.heap);
	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 1U);
}

CUTE_PNP_TEST(sbnmhut_extract_single, &sbnmhut_empty)
{
	struct sbnmhut_node node = SBNMHUT_INIT_NODE(2);

	sbnm_heap_insert(&sbnmhut_heap, &node.heap, sbnmhut_compare_min);

	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 1U);
	cute_ensure(sbnm_heap_extract(&sbnmhut_heap, sbnmhut_compare_min) ==
	            &node.heap);
	cute_ensure(sbnmhut_heap.sbnm_trees == NULL);
	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 0U);
}

static void sbnmhut_check_roots(const struct sbnm_heap* heap,
                                unsigned int            count)
{
	const struct sbnm_heap_node *node = heap->sbnm_trees;
	int                          order = -1;

	while (node) {
		while (!(count & 1)) {
			count >>= 1;
			order++;
		}
		order++;
		count >>= 1;

		cute_ensure(node->sbnm_parent == NULL);
		cute_ensure(node->sbnm_order == (unsigned int)order);

		node = node->sbnm_sibling;
	}

	cute_ensure(count == 0);
}

static void sbnmhut_check_heap(struct sbnm_heap     *heap,
                               struct sbnmhut_node  *nodes,
                               struct sbnmhut_node **checks,
                               unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		sbnm_heap_insert(heap, &nodes[n].heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(heap) == (n + 1));
	}

	sbnmhut_check_roots(heap, count);

	for (n = 0; n < count; n++) {
		const struct sbnm_heap_node *node = NULL;
		const struct sbnmhut_node   *check = checks[n];

		node = sbnm_heap_peek(heap);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = sbnm_heap_extract(heap, sbnmhut_compare_min);
		cute_ensure(sbnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);
	}
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_inorder, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

static struct sbnmhut_node sbnmhut_inorder_nodes[] = {
	SBNMHUT_INIT_NODE(0),
	SBNMHUT_INIT_NODE(1),
	SBNMHUT_INIT_NODE(2),
	SBNMHUT_INIT_NODE(3),
	SBNMHUT_INIT_NODE(4),
	SBNMHUT_INIT_NODE(5),
	SBNMHUT_INIT_NODE(6),
	SBNMHUT_INIT_NODE(7),
	SBNMHUT_INIT_NODE(8),
	SBNMHUT_INIT_NODE(9),
	SBNMHUT_INIT_NODE(10),
	SBNMHUT_INIT_NODE(11),
	SBNMHUT_INIT_NODE(12),
	SBNMHUT_INIT_NODE(13),
	SBNMHUT_INIT_NODE(14),
	SBNMHUT_INIT_NODE(15),
	SBNMHUT_INIT_NODE(16)
};

static struct sbnmhut_node *sbnmhut_inorder_checks[] = {
	&sbnmhut_inorder_nodes[0],
	&sbnmhut_inorder_nodes[1],
	&sbnmhut_inorder_nodes[2],
	&sbnmhut_inorder_nodes[3],
	&sbnmhut_inorder_nodes[4],
	&sbnmhut_inorder_nodes[5],
	&sbnmhut_inorder_nodes[6],
	&sbnmhut_inorder_nodes[7],
	&sbnmhut_inorder_nodes[8],
	&sbnmhut_inorder_nodes[9],
	&sbnmhut_inorder_nodes[10],
	&sbnmhut_inorder_nodes[11],
	&sbnmhut_inorder_nodes[12],
	&sbnmhut_inorder_nodes[13],
	&sbnmhut_inorder_nodes[14],
	&sbnmhut_inorder_nodes[15],
	&sbnmhut_inorder_nodes[16]
};

CUTE_PNP_TEST(sbnmhut_inorder2, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 2);
}

CUTE_PNP_TEST(sbnmhut_inorder3, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 3);
}

CUTE_PNP_TEST(sbnmhut_inorder4, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 4);
}

CUTE_PNP_TEST(sbnmhut_inorder5, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 5);
}

CUTE_PNP_TEST(sbnmhut_inorder6, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 6);
}

CUTE_PNP_TEST(sbnmhut_inorder7, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 7);
}

CUTE_PNP_TEST(sbnmhut_inorder8, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 8);
}

CUTE_PNP_TEST(sbnmhut_inorder9, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 9);
}

CUTE_PNP_TEST(sbnmhut_inorder10, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 10);
}

CUTE_PNP_TEST(sbnmhut_inorder11, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 11);
}

CUTE_PNP_TEST(sbnmhut_inorder12, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 12);
}

CUTE_PNP_TEST(sbnmhut_inorder13, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 13);
}

CUTE_PNP_TEST(sbnmhut_inorder14, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 14);
}

CUTE_PNP_TEST(sbnmhut_inorder15, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 15);
}

CUTE_PNP_TEST(sbnmhut_inorder16, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 16);
}

CUTE_PNP_TEST(sbnmhut_inorder17, &sbnmhut_inorder)
{
	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_inorder_nodes,
	                   sbnmhut_inorder_checks, 17);
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_revorder, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

static struct sbnmhut_node sbnmhut_revorder_nodes[] = {
	SBNMHUT_INIT_NODE(16),
	SBNMHUT_INIT_NODE(15),
	SBNMHUT_INIT_NODE(14),
	SBNMHUT_INIT_NODE(13),
	SBNMHUT_INIT_NODE(12),
	SBNMHUT_INIT_NODE(11),
	SBNMHUT_INIT_NODE(10),
	SBNMHUT_INIT_NODE(9),
	SBNMHUT_INIT_NODE(8),
	SBNMHUT_INIT_NODE(7),
	SBNMHUT_INIT_NODE(6),
	SBNMHUT_INIT_NODE(5),
	SBNMHUT_INIT_NODE(4),
	SBNMHUT_INIT_NODE(3),
	SBNMHUT_INIT_NODE(2),
	SBNMHUT_INIT_NODE(1),
	SBNMHUT_INIT_NODE(0)
};

static struct sbnmhut_node *sbnmhut_revorder_checks[] = {
	&sbnmhut_revorder_nodes[16],
	&sbnmhut_revorder_nodes[15],
	&sbnmhut_revorder_nodes[14],
	&sbnmhut_revorder_nodes[13],
	&sbnmhut_revorder_nodes[12],
	&sbnmhut_revorder_nodes[11],
	&sbnmhut_revorder_nodes[10],
	&sbnmhut_revorder_nodes[9],
	&sbnmhut_revorder_nodes[8],
	&sbnmhut_revorder_nodes[7],
	&sbnmhut_revorder_nodes[6],
	&sbnmhut_revorder_nodes[5],
	&sbnmhut_revorder_nodes[4],
	&sbnmhut_revorder_nodes[3],
	&sbnmhut_revorder_nodes[2],
	&sbnmhut_revorder_nodes[1],
	&sbnmhut_revorder_nodes[0]
};

CUTE_PNP_TEST(sbnmhut_revorder2, &sbnmhut_revorder)
{
	unsigned int          count = 2;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder3, &sbnmhut_revorder)
{
	unsigned int          count = 3;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder4, &sbnmhut_revorder)
{
	unsigned int          count = 4;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                        count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder5, &sbnmhut_revorder)
{
	unsigned int          count = 5;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder6, &sbnmhut_revorder)
{
	unsigned int          count = 6;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder7, &sbnmhut_revorder)
{
	unsigned int          count = 7;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder8, &sbnmhut_revorder)
{
	unsigned int          count = 8;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder9, &sbnmhut_revorder)
{
	unsigned int          count = 9;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder10, &sbnmhut_revorder)
{
	unsigned int          count = 10;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder11, &sbnmhut_revorder)
{
	unsigned int          count = 11;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder12, &sbnmhut_revorder)
{
	unsigned int          count = 12;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder13, &sbnmhut_revorder)
{
	unsigned int          count = 13;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder14, &sbnmhut_revorder)
{
	unsigned int          count = 14;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder15, &sbnmhut_revorder)
{
	unsigned int          count = 15;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder16, &sbnmhut_revorder)
{
	unsigned int          count = 16;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(sbnmhut_revorder17, &sbnmhut_revorder)
{
	unsigned int          count = 17;
	struct sbnmhut_node **checks =
		&sbnmhut_revorder_checks[array_nr(sbnmhut_revorder_checks) -
		                         count];

	sbnmhut_check_heap(&sbnmhut_heap, sbnmhut_revorder_nodes, checks,
	                   count);
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_unsorted, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(sbnmhut_unsorted_increasing, &sbnmhut_unsorted)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(0),  /* 0 */
		SBNMHUT_INIT_NODE(4),
		SBNMHUT_INIT_NODE(5),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),  /* 5 */
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(10),
		SBNMHUT_INIT_NODE(11),
		SBNMHUT_INIT_NODE(12),
		SBNMHUT_INIT_NODE(7),  /* 10 */
		SBNMHUT_INIT_NODE(8),
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(16),
		SBNMHUT_INIT_NODE(13),
		SBNMHUT_INIT_NODE(14), /* 15 */
		SBNMHUT_INIT_NODE(15)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[0],
		&nodes[4],
		&nodes[5],
		&nodes[6],
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[10],
		&nodes[11],
		&nodes[12],
		&nodes[7],
		&nodes[8],
		&nodes[9],
		&nodes[14],
		&nodes[15],
		&nodes[16],
		&nodes[13]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_unsorted_decreasing, &sbnmhut_unsorted)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(6),  /* 0 */
		SBNMHUT_INIT_NODE(5),
		SBNMHUT_INIT_NODE(4),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(2),  /* 5 */
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(8),
		SBNMHUT_INIT_NODE(7),
		SBNMHUT_INIT_NODE(16), /* 10 */
		SBNMHUT_INIT_NODE(12),
		SBNMHUT_INIT_NODE(11),
		SBNMHUT_INIT_NODE(10),
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(14), /* 15 */
		SBNMHUT_INIT_NODE(13)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[3],
		&nodes[6],
		&nodes[5],
		&nodes[4],
		&nodes[2],
		&nodes[1],
		&nodes[0],
		&nodes[9],
		&nodes[8],
		&nodes[7],
		&nodes[13],
		&nodes[12],
		&nodes[11],
		&nodes[16],
		&nodes[15],
		&nodes[14],
		&nodes[10]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_unsorted_diverge, &sbnmhut_unsorted)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(4),  /* 0 */
		SBNMHUT_INIT_NODE(5),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(1),  /* 5 */
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(10),
		SBNMHUT_INIT_NODE(11),
		SBNMHUT_INIT_NODE(12),
		SBNMHUT_INIT_NODE(9),  /* 10 */
		SBNMHUT_INIT_NODE(8),
		SBNMHUT_INIT_NODE(7),
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(14),
		SBNMHUT_INIT_NODE(16), /* 15 */
		SBNMHUT_INIT_NODE(13)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[6],
		&nodes[5],
		&nodes[4],
		&nodes[3],
		&nodes[0],
		&nodes[1],
		&nodes[2],
		&nodes[12],
		&nodes[11],
		&nodes[10],
		&nodes[7],
		&nodes[8],
		&nodes[9],
		&nodes[16],
		&nodes[14],
		&nodes[13],
		&nodes[15]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_unsorted_converge, &sbnmhut_unsorted)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(16), /* 0 */
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(14),
		SBNMHUT_INIT_NODE(13),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(1),  /* 5 */
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(12),
		SBNMHUT_INIT_NODE(11),
		SBNMHUT_INIT_NODE(10), /* 10 */
		SBNMHUT_INIT_NODE(4),
		SBNMHUT_INIT_NODE(5),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(7),  /* 15 */
		SBNMHUT_INIT_NODE(8)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[4],
		&nodes[5],
		&nodes[6],
		&nodes[7],
		&nodes[11],
		&nodes[12],
		&nodes[13],
		&nodes[15],
		&nodes[16],
		&nodes[14],
		&nodes[10],
		&nodes[9],
		&nodes[8],
		&nodes[3],
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_duplicates, &sbnmhut,
                               sbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(sbnmhut_duplicates2, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(0)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_duplicates3, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(0)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_duplicates_leading, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(2)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_duplicates_trailing, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(0)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_duplicates_interleave, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(0)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_duplicates_mix, &sbnmhut_duplicates)
{
	struct sbnmhut_node  nodes[] = {
		SBNMHUT_INIT_NODE(2),  /* 0 */
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(8),  /* 5 */
		SBNMHUT_INIT_NODE(7),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(5),
		SBNMHUT_INIT_NODE(4),
		SBNMHUT_INIT_NODE(4),  /* 10 */
		SBNMHUT_INIT_NODE(10),
		SBNMHUT_INIT_NODE(11),
		SBNMHUT_INIT_NODE(13),
		SBNMHUT_INIT_NODE(8),
		SBNMHUT_INIT_NODE(12), /* 15 */
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(9)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[3],
		&nodes[1],
		&nodes[0],
		&nodes[4],
		&nodes[10],
		&nodes[9],
		&nodes[8],
		&nodes[7],
		&nodes[6],
		&nodes[5],
		&nodes[14],
		&nodes[17],
		&nodes[16],
		&nodes[11],
		&nodes[12],
		&nodes[15],
		&nodes[13]
	};

	sbnmhut_check_heap(&sbnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_SUITE(sbnmhut_merge, &sbnmhut);

static void sbnmhut_check_heap_merge(struct sbnmhut_node  *first,
                                     unsigned int          first_count,
                                     struct sbnmhut_node  *second,
                                     unsigned int          second_count,
                                     struct sbnmhut_node **checks)
{
	struct sbnm_heap fst;
	struct sbnm_heap snd;
	unsigned int    n;

	sbnm_heap_init(&fst);
	for (n = 0; n < first_count; n++) {
		sbnm_heap_insert(&fst, &first[n].heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(&fst) == (n + 1));
	}
	sbnmhut_check_roots(&fst, first_count);

	sbnm_heap_init(&snd);
	for (n = 0; n < second_count; n++) {
		sbnm_heap_insert(&snd, &second[n].heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(&snd) == (n + 1));
	}
	sbnmhut_check_roots(&snd, second_count);

	sbnm_heap_merge(&fst, &snd, sbnmhut_compare_min);

	sbnmhut_check_roots(&fst, first_count + second_count);

	for (n = 0; n < (first_count + second_count); n++) {
		const struct sbnm_heap_node *node;
		const struct sbnmhut_node   *check = checks[n];

		node = sbnm_heap_extract(&fst, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(&fst) ==
		            first_count + second_count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(sbnmhut_merge_inorder11, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(1)
	};
	struct sbnmhut_node *checks[] = {
		&fst[0],
		&snd[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_revorder11, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(1)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node *checks[] = {
		&snd[0],
		&fst[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_inorder12, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
	};
	struct sbnmhut_node *checks[] = {
		&fst[0],
		&snd[0],
		&snd[1]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_revorder12, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(2)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node *checks[] = {
		&snd[1],
		&snd[0],
		&fst[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_unsorted12, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(1)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&snd[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_unsorted22, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(0)
	};
	struct sbnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&fst[1],
		&snd[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_unsorted31, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(0),
		SBNMHUT_INIT_NODE(1)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(2),
	};
	struct sbnmhut_node *checks[] = {
		&fst[1],
		&fst[2],
		&snd[0],
		&fst[0]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(sbnmhut_merge_mit, &sbnmhut_merge)
{
	struct sbnmhut_node  fst[] = {
		SBNMHUT_INIT_NODE(41),
		SBNMHUT_INIT_NODE(28),
		SBNMHUT_INIT_NODE(33),
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(7),
		SBNMHUT_INIT_NODE(25),
		SBNMHUT_INIT_NODE(12)
	};
	struct sbnmhut_node  snd[] = {
		SBNMHUT_INIT_NODE(17),
		SBNMHUT_INIT_NODE(10),
		SBNMHUT_INIT_NODE(44),
		SBNMHUT_INIT_NODE(50),
		SBNMHUT_INIT_NODE(31),
		SBNMHUT_INIT_NODE(48),
		SBNMHUT_INIT_NODE(29),
		SBNMHUT_INIT_NODE(8),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(24),
		SBNMHUT_INIT_NODE(22),
		SBNMHUT_INIT_NODE(23),
		SBNMHUT_INIT_NODE(55),
		SBNMHUT_INIT_NODE(32),
		SBNMHUT_INIT_NODE(45),
		SBNMHUT_INIT_NODE(30),
		SBNMHUT_INIT_NODE(37),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(18)
	};
	struct sbnmhut_node *checks[] = {
		&snd[17],
		&snd[8],
		&fst[4],
		&snd[7],
		&snd[1],
		&fst[6],
		&fst[3],
		&snd[0],
		&snd[18],
		&snd[10],
		&snd[11],
		&snd[9],
		&fst[5],
		&fst[1],
		&snd[6],
		&snd[15],
		&snd[4],
		&snd[13],
		&fst[2],
		&snd[16],
		&fst[0],
		&snd[2],
		&snd[14],
		&snd[5],
		&snd[3],
		&snd[12]
	};

	sbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_update, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

static void sbnmhut_check_update(struct sbnm_heap     *heap,
                                 unsigned int          new_index,
                                 int                   new_key,
                                 struct sbnmhut_node  *nodes,
                                 struct sbnmhut_node **checks,
                                 unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		sbnm_heap_insert(heap, &nodes[n].heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(heap) == (n + 1));
	}

	sbnmhut_check_roots(heap, count);

	nodes[new_index].key = new_key;
	sbnm_heap_update(heap, &nodes[new_index].heap, sbnmhut_compare_min);

	for (n = 0; n < count; n++) {
		const struct sbnm_heap_node *node = NULL;
		const struct sbnmhut_node   *check = checks[n];

		node = sbnm_heap_peek(heap);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = sbnm_heap_extract(heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(sbnmhut_update1, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(0)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[0]
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 2, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_up2, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_update(&sbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_down2, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 3, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_still2, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1]
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_left_up3, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_right_up3, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	sbnmhut_check_update(&sbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_down3, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[0]
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 4, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_deep_left_up4, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(4)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[3],
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};

	sbnmhut_check_update(&sbnmhut_heap, 3, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_shallow_left_up4, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(4)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1],
		&nodes[3]
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_right_up4, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(4)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3]
	};

	sbnmhut_check_update(&sbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_shallow_left_down4, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(4)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1],
		&nodes[3],
		&nodes[2]
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 5, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(sbnmhut_update_root_left_down4, &sbnmhut_update)
{
	struct sbnmhut_node nodes[] = {
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2),
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(4)
	};

	struct sbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[0]
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 5, nodes, checks,
	                     array_nr(nodes));
}

static struct sbnmhut_node sbnmhut_update_nodes[] = {
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(23),
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(21),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(18),
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(12)
};

CUTE_PNP_TEST(sbnmhut_update_30, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_2322, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 1, 22, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up230, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[1],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
	};

	sbnmhut_check_update(&sbnmhut_heap, 1, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_1514, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 14, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up150, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_65, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 4, 5, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up60, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 4, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_2120, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 3, 20, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up210, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 3, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up215, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 3, 5, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_1817, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 5, 17, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up180, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 5, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up185, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 5, 5, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_98, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 6, 8, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up90, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 6, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up95, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 6, 5, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_1211, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 7, 11, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up120, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 7, 0, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up125, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 7, 5, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_up128, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 7, 8, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_34, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 4, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down37, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 7, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down310, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 10, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down314, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 0, 14, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down1522, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 2, 22, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down619, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 4, 19, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(sbnmhut_update_down610, &sbnmhut_update)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_update_nodes[0],
		&sbnmhut_update_nodes[6],
		&sbnmhut_update_nodes[4],
		&sbnmhut_update_nodes[7],
		&sbnmhut_update_nodes[2],
		&sbnmhut_update_nodes[5],
		&sbnmhut_update_nodes[3],
		&sbnmhut_update_nodes[1],
	};

	sbnmhut_check_update(&sbnmhut_heap, 4, 10, sbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

static CUTE_PNP_FIXTURED_SUITE(sbnmhut_remove, &sbnmhut, sbnmhut_setup_empty,
                               NULL);

static void sbnmhut_check_remove(struct sbnm_heap     *heap,
                                 unsigned int          index,
                                 struct sbnmhut_node  *nodes,
                                 struct sbnmhut_node **checks,
                                 unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		sbnm_heap_insert(heap, &nodes[n].heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(heap) == (n + 1));
	}

	sbnmhut_check_roots(heap, count);

	sbnm_heap_remove(heap, &nodes[index].heap, sbnmhut_compare_min);

	for (n = 0; n < (count - 1); n++) {
		const struct sbnm_heap_node *node = NULL;
		const struct sbnmhut_node   *check = checks[n];

		node = sbnm_heap_peek(heap);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = sbnm_heap_extract(heap, sbnmhut_compare_min);

		cute_ensure(sbnm_heap_count(heap) == count - n - 2);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct sbnmhut_node *)node)->key == check->key);
	}
}

static struct sbnmhut_node sbnmhut_remove_nodes[] = {
		SBNMHUT_INIT_NODE(3),
		SBNMHUT_INIT_NODE(23),
		SBNMHUT_INIT_NODE(15),
		SBNMHUT_INIT_NODE(21),
		SBNMHUT_INIT_NODE(6),
		SBNMHUT_INIT_NODE(18),
		SBNMHUT_INIT_NODE(9),
		SBNMHUT_INIT_NODE(12),
		SBNMHUT_INIT_NODE(27),
		SBNMHUT_INIT_NODE(1),
		SBNMHUT_INIT_NODE(2)
};

CUTE_PNP_TEST(sbnmhut_remove_alone, &sbnmhut_remove)
{
	struct sbnmhut_node node = SBNMHUT_INIT_NODE(2);

	sbnm_heap_insert(&sbnmhut_heap, &node.heap, sbnmhut_compare_min);

	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 1U);

	sbnm_heap_remove(&sbnmhut_heap, &node.heap, sbnmhut_compare_min);

	cute_ensure(sbnmhut_heap.sbnm_trees == NULL);
	cute_ensure(sbnm_heap_count(&sbnmhut_heap) == 0U);
}

CUTE_PNP_TEST(sbnmhut_remove1, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 9, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove2, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 10, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove3, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 0, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove6, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 4, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove9, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 6, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove12, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 7, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove15, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 2, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove18, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 5, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove21, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[1],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 3, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove23, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[8]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 1, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}

CUTE_PNP_TEST(sbnmhut_remove27, &sbnmhut_remove)
{
	struct sbnmhut_node *checks[] = {
		&sbnmhut_remove_nodes[9],
		&sbnmhut_remove_nodes[10],
		&sbnmhut_remove_nodes[0],
		&sbnmhut_remove_nodes[4],
		&sbnmhut_remove_nodes[6],
		&sbnmhut_remove_nodes[7],
		&sbnmhut_remove_nodes[2],
		&sbnmhut_remove_nodes[5],
		&sbnmhut_remove_nodes[3],
		&sbnmhut_remove_nodes[1]
	};

	sbnmhut_check_remove(&sbnmhut_heap, 8, sbnmhut_remove_nodes, checks,
	                     array_nr(sbnmhut_remove_nodes));
}
