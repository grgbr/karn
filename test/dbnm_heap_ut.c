/**
 * @file      dbnm_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Doubly linked list based binomial heap unit tests implementation
 *
 * @defgroup dbnmhut Doubly linked list based binomial heap unit tests
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

#include "dbnm_heap.h"
#include "array.h"
#include <cute/cute.h>

struct dbnmhut_node {
	struct dbnm_heap_node heap;
	int                   key;
};

#define DBNMHUT_INIT_NODE(_key)                                                \
	{                                                                      \
		.heap = {                                                      \
			.dbnm_sibling = {                                      \
				.dlist_next = (struct dlist_node *)0xdeadbeef, \
				.dlist_prev = (struct dlist_node *)0xbeefdead, \
			},                                                     \
			.dbnm_parent  = (struct dbnm_heap_node *)0xdeadbeef,   \
			.dbnm_child   = (struct dlist_node *)0xdeadbeef,       \
			.dbnm_order   = 0xdeadbeef                             \
		},                                                             \
		.key = _key                                                    \
	}

static struct dbnm_heap dbnmhut_heap;

static int dbnmhut_compare_min(const struct dbnm_heap_node *first,
                               const struct dbnm_heap_node *second)
{
	return ((struct dbnmhut_node *)first)->key -
	       ((struct dbnmhut_node *)second)->key;
}

static CUTE_PNP_SUITE(dbnmhut, NULL);

static void
dbnmhut_setup_empty(void)
{
	dbnm_heap_init(&dbnmhut_heap);
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_empty, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(dbnmhut_check_emptiness, &dbnmhut_empty)
{
	cute_ensure(dbnm_heap_empty(&dbnmhut_heap) == true);
}

CUTE_PNP_TEST(dbnmhut_insert_single, &dbnmhut_empty)
{
	struct dbnmhut_node node = DBNMHUT_INIT_NODE(2);

	dbnm_heap_insert(&dbnmhut_heap, &node.heap, dbnmhut_compare_min);

	cute_ensure(dbnm_heap_count(&dbnmhut_heap) == 1U);
	cute_ensure(dlist_next(&dbnmhut_heap.dbnm_roots) ==
	            &node.heap.dbnm_sibling);
	cute_ensure(node.heap.dbnm_child == NULL);
	cute_ensure(node.heap.dbnm_parent == NULL);
	cute_ensure(dlist_empty(&node.heap.dbnm_sibling) == false);
	cute_ensure(node.heap.dbnm_order == 0);
}

CUTE_PNP_TEST(dbnmhut_peek_single, &dbnmhut_empty)
{
	struct dbnmhut_node node = DBNMHUT_INIT_NODE(2);

	dbnm_heap_insert(&dbnmhut_heap, &node.heap, dbnmhut_compare_min);

	cute_ensure(dbnm_heap_count(&dbnmhut_heap) == 1U);
	cute_ensure(dbnm_heap_peek(&dbnmhut_heap, dbnmhut_compare_min) ==
	            &node.heap);
	cute_ensure(dbnm_heap_count(&dbnmhut_heap) == 1U);
}

CUTE_PNP_TEST(dbnmhut_extract_single, &dbnmhut_empty)
{
	struct dbnmhut_node node = DBNMHUT_INIT_NODE(2);

	dbnm_heap_insert(&dbnmhut_heap, &node.heap, dbnmhut_compare_min);

	cute_ensure(dbnm_heap_count(&dbnmhut_heap) == 1U);
	cute_ensure(dbnm_heap_extract(&dbnmhut_heap, dbnmhut_compare_min) ==
	            &node.heap);
	cute_ensure(dlist_empty(&dbnmhut_heap.dbnm_roots));
	cute_ensure(dbnm_heap_count(&dbnmhut_heap) == 0U);
}

static void dbnmhut_check_roots(const struct dbnm_heap* heap,
                                unsigned int            count)
{
	const struct dbnm_heap_node *node;
	int                          order = -1;

	dlist_foreach_entry(&heap->dbnm_roots, node, dbnm_sibling) {
		while (!(count & 1)) {
			count >>= 1;
			order++;
		}
		order++;
		count >>= 1;

		cute_ensure(node->dbnm_parent == NULL);
		cute_ensure(node->dbnm_order == (unsigned int)order);
	}

	cute_ensure(count == 0);
}

static void dbnmhut_check_heap(struct dbnm_heap     *heap,
                               struct dbnmhut_node  *nodes,
                               struct dbnmhut_node **checks,
                               unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		dbnm_heap_insert(heap, &nodes[n].heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(heap) == (n + 1));
	}

	dbnmhut_check_roots(heap, count);

	for (n = 0; n < count; n++) {
		const struct dbnm_heap_node *node = NULL;
		const struct dbnmhut_node   *check = checks[n];

		node = dbnm_heap_peek(heap, dbnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = dbnm_heap_extract(heap, dbnmhut_compare_min);
		cute_ensure(dbnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);
	}
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_inorder, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

static struct dbnmhut_node dbnmhut_inorder_nodes[] = {
	DBNMHUT_INIT_NODE(0),
	DBNMHUT_INIT_NODE(1),
	DBNMHUT_INIT_NODE(2),
	DBNMHUT_INIT_NODE(3),
	DBNMHUT_INIT_NODE(4),
	DBNMHUT_INIT_NODE(5),
	DBNMHUT_INIT_NODE(6),
	DBNMHUT_INIT_NODE(7),
	DBNMHUT_INIT_NODE(8),
	DBNMHUT_INIT_NODE(9),
	DBNMHUT_INIT_NODE(10),
	DBNMHUT_INIT_NODE(11),
	DBNMHUT_INIT_NODE(12),
	DBNMHUT_INIT_NODE(13),
	DBNMHUT_INIT_NODE(14),
	DBNMHUT_INIT_NODE(15),
	DBNMHUT_INIT_NODE(16)
};

static struct dbnmhut_node *dbnmhut_inorder_checks[] = {
	&dbnmhut_inorder_nodes[0],
	&dbnmhut_inorder_nodes[1],
	&dbnmhut_inorder_nodes[2],
	&dbnmhut_inorder_nodes[3],
	&dbnmhut_inorder_nodes[4],
	&dbnmhut_inorder_nodes[5],
	&dbnmhut_inorder_nodes[6],
	&dbnmhut_inorder_nodes[7],
	&dbnmhut_inorder_nodes[8],
	&dbnmhut_inorder_nodes[9],
	&dbnmhut_inorder_nodes[10],
	&dbnmhut_inorder_nodes[11],
	&dbnmhut_inorder_nodes[12],
	&dbnmhut_inorder_nodes[13],
	&dbnmhut_inorder_nodes[14],
	&dbnmhut_inorder_nodes[15],
	&dbnmhut_inorder_nodes[16]
};

CUTE_PNP_TEST(dbnmhut_inorder2, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 2);
}

CUTE_PNP_TEST(dbnmhut_inorder3, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 3);
}

CUTE_PNP_TEST(dbnmhut_inorder4, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 4);
}

CUTE_PNP_TEST(dbnmhut_inorder5, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 5);
}

CUTE_PNP_TEST(dbnmhut_inorder6, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 6);
}

CUTE_PNP_TEST(dbnmhut_inorder7, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 7);
}

CUTE_PNP_TEST(dbnmhut_inorder8, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 8);
}

CUTE_PNP_TEST(dbnmhut_inorder9, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 9);
}

CUTE_PNP_TEST(dbnmhut_inorder10, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 10);
}

CUTE_PNP_TEST(dbnmhut_inorder11, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 11);
}

CUTE_PNP_TEST(dbnmhut_inorder12, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 12);
}

CUTE_PNP_TEST(dbnmhut_inorder13, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 13);
}

CUTE_PNP_TEST(dbnmhut_inorder14, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 14);
}

CUTE_PNP_TEST(dbnmhut_inorder15, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 15);
}

CUTE_PNP_TEST(dbnmhut_inorder16, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 16);
}

CUTE_PNP_TEST(dbnmhut_inorder17, &dbnmhut_inorder)
{
	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_inorder_nodes,
	                   dbnmhut_inorder_checks, 17);
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_revorder, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

static struct dbnmhut_node dbnmhut_revorder_nodes[] = {
	DBNMHUT_INIT_NODE(16),
	DBNMHUT_INIT_NODE(15),
	DBNMHUT_INIT_NODE(14),
	DBNMHUT_INIT_NODE(13),
	DBNMHUT_INIT_NODE(12),
	DBNMHUT_INIT_NODE(11),
	DBNMHUT_INIT_NODE(10),
	DBNMHUT_INIT_NODE(9),
	DBNMHUT_INIT_NODE(8),
	DBNMHUT_INIT_NODE(7),
	DBNMHUT_INIT_NODE(6),
	DBNMHUT_INIT_NODE(5),
	DBNMHUT_INIT_NODE(4),
	DBNMHUT_INIT_NODE(3),
	DBNMHUT_INIT_NODE(2),
	DBNMHUT_INIT_NODE(1),
	DBNMHUT_INIT_NODE(0)
};

static struct dbnmhut_node *dbnmhut_revorder_checks[] = {
	&dbnmhut_revorder_nodes[16],
	&dbnmhut_revorder_nodes[15],
	&dbnmhut_revorder_nodes[14],
	&dbnmhut_revorder_nodes[13],
	&dbnmhut_revorder_nodes[12],
	&dbnmhut_revorder_nodes[11],
	&dbnmhut_revorder_nodes[10],
	&dbnmhut_revorder_nodes[9],
	&dbnmhut_revorder_nodes[8],
	&dbnmhut_revorder_nodes[7],
	&dbnmhut_revorder_nodes[6],
	&dbnmhut_revorder_nodes[5],
	&dbnmhut_revorder_nodes[4],
	&dbnmhut_revorder_nodes[3],
	&dbnmhut_revorder_nodes[2],
	&dbnmhut_revorder_nodes[1],
	&dbnmhut_revorder_nodes[0]
};

CUTE_PNP_TEST(dbnmhut_revorder2, &dbnmhut_revorder)
{
	unsigned int         count = 2;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder3, &dbnmhut_revorder)
{
	unsigned int         count = 3;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder4, &dbnmhut_revorder)
{
	unsigned int         count = 4;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder5, &dbnmhut_revorder)
{
	unsigned int         count = 5;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder6, &dbnmhut_revorder)
{
	unsigned int         count = 6;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder7, &dbnmhut_revorder)
{
	unsigned int         count = 7;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder8, &dbnmhut_revorder)
{
	unsigned int         count = 8;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder9, &dbnmhut_revorder)
{
	unsigned int         count = 9;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder10, &dbnmhut_revorder)
{
	unsigned int         count = 10;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder11, &dbnmhut_revorder)
{
	unsigned int         count = 11;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder12, &dbnmhut_revorder)
{
	unsigned int         count = 12;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder13, &dbnmhut_revorder)
{
	unsigned int         count = 13;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder14, &dbnmhut_revorder)
{
	unsigned int         count = 14;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder15, &dbnmhut_revorder)
{
	unsigned int         count = 15;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder16, &dbnmhut_revorder)
{
	unsigned int         count = 16;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

CUTE_PNP_TEST(dbnmhut_revorder17, &dbnmhut_revorder)
{
	unsigned int         count = 17;
	struct dbnmhut_node **checks =
		&dbnmhut_revorder_checks[array_nr(dbnmhut_revorder_checks) -
		                         count];

	dbnmhut_check_heap(&dbnmhut_heap, dbnmhut_revorder_nodes, checks,
	                   count);
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_unsorted, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(dbnmhut_unsorted_increasing, &dbnmhut_unsorted)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(0),  /* 0 */
		DBNMHUT_INIT_NODE(4),
		DBNMHUT_INIT_NODE(5),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),  /* 5 */
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(10),
		DBNMHUT_INIT_NODE(11),
		DBNMHUT_INIT_NODE(12),
		DBNMHUT_INIT_NODE(7),  /* 10 */
		DBNMHUT_INIT_NODE(8),
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(16),
		DBNMHUT_INIT_NODE(13),
		DBNMHUT_INIT_NODE(14), /* 15 */
		DBNMHUT_INIT_NODE(15)
	};

	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_unsorted_decreasing, &dbnmhut_unsorted)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(6),  /* 0 */
		DBNMHUT_INIT_NODE(5),
		DBNMHUT_INIT_NODE(4),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(2),  /* 5 */
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(8),
		DBNMHUT_INIT_NODE(7),
		DBNMHUT_INIT_NODE(16), /* 10 */
		DBNMHUT_INIT_NODE(12),
		DBNMHUT_INIT_NODE(11),
		DBNMHUT_INIT_NODE(10),
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(14), /* 15 */
		DBNMHUT_INIT_NODE(13)
	};

	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_unsorted_diverge, &dbnmhut_unsorted)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(4),  /* 0 */
		DBNMHUT_INIT_NODE(5),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(1),  /* 5 */
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(10),
		DBNMHUT_INIT_NODE(11),
		DBNMHUT_INIT_NODE(12),
		DBNMHUT_INIT_NODE(9),  /* 10 */
		DBNMHUT_INIT_NODE(8),
		DBNMHUT_INIT_NODE(7),
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(14),
		DBNMHUT_INIT_NODE(16), /* 15 */
		DBNMHUT_INIT_NODE(13)
	};

	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_unsorted_converge, &dbnmhut_unsorted)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(16), /* 0 */
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(14),
		DBNMHUT_INIT_NODE(13),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(1),  /* 5 */
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(12),
		DBNMHUT_INIT_NODE(11),
		DBNMHUT_INIT_NODE(10), /* 10 */
		DBNMHUT_INIT_NODE(4),
		DBNMHUT_INIT_NODE(5),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(7),  /* 15 */
		DBNMHUT_INIT_NODE(8)
	};

	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_duplicates, &dbnmhut,
                               dbnmhut_setup_empty, NULL);

CUTE_PNP_TEST(dbnmhut_duplicates2, &dbnmhut_duplicates)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(0)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_duplicates3, &dbnmhut_duplicates)
{
	struct dbnmhut_node  nodes[] = {
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(0)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_duplicates_leading, &dbnmhut_duplicates)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(2)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_duplicates_trailing, &dbnmhut_duplicates)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(0)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_duplicates_interleave, &dbnmhut_duplicates)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(0)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_duplicates_mix, &dbnmhut_duplicates)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(2),  /* 0 */
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(8),  /* 5 */
		DBNMHUT_INIT_NODE(7),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(5),
		DBNMHUT_INIT_NODE(4),
		DBNMHUT_INIT_NODE(4),  /* 10 */
		DBNMHUT_INIT_NODE(10),
		DBNMHUT_INIT_NODE(11),
		DBNMHUT_INIT_NODE(13),
		DBNMHUT_INIT_NODE(8),
		DBNMHUT_INIT_NODE(12), /* 15 */
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(9)
	};

	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap(&dbnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_SUITE(dbnmhut_merge, &dbnmhut);

static void dbnmhut_check_heap_merge(struct dbnmhut_node  *first,
                                     unsigned int          first_count,
                                     struct dbnmhut_node  *second,
                                     unsigned int          second_count,
                                     struct dbnmhut_node **checks)
{
	struct dbnm_heap fst;
	struct dbnm_heap snd;
	unsigned int     n;

	dbnm_heap_init(&fst);
	for (n = 0; n < first_count; n++) {
		dbnm_heap_insert(&fst, &first[n].heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(&fst) == (n + 1));
	}
	dbnmhut_check_roots(&fst, first_count);

	dbnm_heap_init(&snd);
	for (n = 0; n < second_count; n++) {
		dbnm_heap_insert(&snd, &second[n].heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(&snd) == (n + 1));
	}
	dbnmhut_check_roots(&snd, second_count);

	dbnm_heap_merge(&fst, &snd, dbnmhut_compare_min);

	dbnmhut_check_roots(&fst, first_count + second_count);

	for (n = 0; n < (first_count + second_count); n++) {
		const struct dbnm_heap_node *node;
		const struct dbnmhut_node   *check = checks[n];

		node = dbnm_heap_extract(&fst, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(&fst) ==
		            first_count + second_count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(dbnmhut_merge_inorder11, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(1)
	};
	struct dbnmhut_node *checks[] = {
		&fst[0],
		&snd[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_revorder11, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(1)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node *checks[] = {
		&snd[0],
		&fst[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_inorder12, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
	};
	struct dbnmhut_node *checks[] = {
		&fst[0],
		&snd[0],
		&snd[1]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_revorder12, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(2)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node *checks[] = {
		&snd[1],
		&snd[0],
		&fst[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_unsorted12, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(1)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&snd[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_unsorted22, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(0)
	};
	struct dbnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&fst[1],
		&snd[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_unsorted31, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(0),
		DBNMHUT_INIT_NODE(1)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(2),
	};
	struct dbnmhut_node *checks[] = {
		&fst[1],
		&fst[2],
		&snd[0],
		&fst[0]
	};

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

CUTE_PNP_TEST(dbnmhut_merge_mit, &dbnmhut_merge)
{
	struct dbnmhut_node  fst[] = {
		DBNMHUT_INIT_NODE(41),
		DBNMHUT_INIT_NODE(28),
		DBNMHUT_INIT_NODE(33),
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(7),
		DBNMHUT_INIT_NODE(25),
		DBNMHUT_INIT_NODE(12)
	};
	struct dbnmhut_node  snd[] = {
		DBNMHUT_INIT_NODE(17),
		DBNMHUT_INIT_NODE(10),
		DBNMHUT_INIT_NODE(44),
		DBNMHUT_INIT_NODE(50),
		DBNMHUT_INIT_NODE(31),
		DBNMHUT_INIT_NODE(48),
		DBNMHUT_INIT_NODE(29),
		DBNMHUT_INIT_NODE(8),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(24),
		DBNMHUT_INIT_NODE(22),
		DBNMHUT_INIT_NODE(23),
		DBNMHUT_INIT_NODE(55),
		DBNMHUT_INIT_NODE(32),
		DBNMHUT_INIT_NODE(45),
		DBNMHUT_INIT_NODE(30),
		DBNMHUT_INIT_NODE(37),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(18)
	};
	struct dbnmhut_node *checks[] = {
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

	dbnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                         checks);
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_update, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

static void dbnmhut_check_update(struct dbnm_heap     *heap,
                                 unsigned int          new_index,
                                 int                   new_key,
                                 struct dbnmhut_node  *nodes,
                                 struct dbnmhut_node **checks,
                                 unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		dbnm_heap_insert(heap, &nodes[n].heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(heap) == (n + 1));
	}

	dbnmhut_check_roots(heap, count);

	nodes[new_index].key = new_key;
	dbnm_heap_update(&nodes[new_index].heap, dbnmhut_compare_min);

	for (n = 0; n < count; n++) {
		const struct dbnm_heap_node *node = NULL;
		const struct dbnmhut_node   *check = checks[n];

		node = dbnm_heap_peek(heap, dbnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = dbnm_heap_extract(heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(dbnmhut_update1, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(0)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[0]
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 2, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_up2, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	dbnmhut_check_update(&dbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_down2, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 3, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_still2, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1]
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_left_up3, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_right_up3, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	dbnmhut_check_update(&dbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_down3, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[0]
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 4, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_deep_left_up4, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(4)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[3],
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};

	dbnmhut_check_update(&dbnmhut_heap, 3, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_shallow_left_up4, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(4)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1],
		&nodes[3]
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_right_up4, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(4)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3]
	};

	dbnmhut_check_update(&dbnmhut_heap, 1, 0, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_shallow_left_down4, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(4)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1],
		&nodes[3],
		&nodes[2]
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 5, nodes, checks,
	                     array_nr(nodes));
}

CUTE_PNP_TEST(dbnmhut_update_root_left_down4, &dbnmhut_update)
{
	struct dbnmhut_node nodes[] = {
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2),
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(4)
	};

	struct dbnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[0]
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 5, nodes, checks,
	                     array_nr(nodes));
}

static struct dbnmhut_node dbnmhut_update_nodes[] = {
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(23),
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(21),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(18),
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(12)
};

CUTE_PNP_TEST(dbnmhut_update_30, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_2322, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 1, 22, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up230, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[1],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
	};

	dbnmhut_check_update(&dbnmhut_heap, 1, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_1514, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 14, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up150, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_65, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 4, 5, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up60, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 4, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_2120, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 3, 20, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up210, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 3, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up215, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 3, 5, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_1817, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 5, 17, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up180, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 5, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up185, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 5, 5, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_98, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 6, 8, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up90, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 6, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up95, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 6, 5, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_1211, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 7, 11, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up120, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 7, 0, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up125, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 7, 5, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_up128, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 7, 8, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_34, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 4, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down37, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 7, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down310, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 10, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down314, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 0, 14, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down1522, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 2, 22, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down619, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 4, 19, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

CUTE_PNP_TEST(dbnmhut_update_down610, &dbnmhut_update)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_update_nodes[0],
		&dbnmhut_update_nodes[6],
		&dbnmhut_update_nodes[4],
		&dbnmhut_update_nodes[7],
		&dbnmhut_update_nodes[2],
		&dbnmhut_update_nodes[5],
		&dbnmhut_update_nodes[3],
		&dbnmhut_update_nodes[1],
	};

	dbnmhut_check_update(&dbnmhut_heap, 4, 10, dbnmhut_update_nodes, checks,
	                     array_nr(checks));
}

static CUTE_PNP_FIXTURED_SUITE(dbnmhut_remove, &dbnmhut, dbnmhut_setup_empty,
                               NULL);

static void dbnmhut_check_remove(struct dbnm_heap     *heap,
                                 unsigned int          index,
                                 struct dbnmhut_node  *nodes,
                                 struct dbnmhut_node **checks,
                                 unsigned int          count)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		dbnm_heap_insert(heap, &nodes[n].heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(heap) == (n + 1));
	}

	dbnmhut_check_roots(heap, count);

	dbnm_heap_remove(heap, &nodes[index].heap, dbnmhut_compare_min);

	for (n = 0; n < (count - 1); n++) {
		const struct dbnm_heap_node *node = NULL;
		const struct dbnmhut_node   *check = checks[n];

		node = dbnm_heap_peek(heap, dbnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);

		node = NULL;
		node = dbnm_heap_extract(heap, dbnmhut_compare_min);

		cute_ensure(dbnm_heap_count(heap) == count - n - 2);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct dbnmhut_node *)node)->key == check->key);
	}
}

static struct dbnmhut_node dbnmhut_remove_nodes[] = {
		DBNMHUT_INIT_NODE(3),
		DBNMHUT_INIT_NODE(23),
		DBNMHUT_INIT_NODE(15),
		DBNMHUT_INIT_NODE(21),
		DBNMHUT_INIT_NODE(6),
		DBNMHUT_INIT_NODE(18),
		DBNMHUT_INIT_NODE(9),
		DBNMHUT_INIT_NODE(12),
		DBNMHUT_INIT_NODE(27),
		DBNMHUT_INIT_NODE(1),
		DBNMHUT_INIT_NODE(2)
};

CUTE_PNP_TEST(dbnmhut_remove1, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 9, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove2, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 10, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove3, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 0, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove6, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 4, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove9, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 6, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove12, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 7, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove15, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 2, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove18, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 5, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove21, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[1],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 3, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove23, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[8]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 1, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}

CUTE_PNP_TEST(dbnmhut_remove27, &dbnmhut_remove)
{
	struct dbnmhut_node *checks[] = {
		&dbnmhut_remove_nodes[9],
		&dbnmhut_remove_nodes[10],
		&dbnmhut_remove_nodes[0],
		&dbnmhut_remove_nodes[4],
		&dbnmhut_remove_nodes[6],
		&dbnmhut_remove_nodes[7],
		&dbnmhut_remove_nodes[2],
		&dbnmhut_remove_nodes[5],
		&dbnmhut_remove_nodes[3],
		&dbnmhut_remove_nodes[1]
	};

	dbnmhut_check_remove(&dbnmhut_heap, 8, dbnmhut_remove_nodes, checks,
	                     array_nr(dbnmhut_remove_nodes));
}
