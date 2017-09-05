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

#define BNMHUT_INIT_NODE(_key)                                                 \
	{                                                                      \
		.heap = {                                                      \
			.bnm_sibling = {                                       \
				.dlist_next = (struct dlist_node *)0xdeadbeef, \
				.dlist_prev = (struct dlist_node *)0xbeefdead, \
			},                                                     \
			.bnm_parent  = (struct bnm_heap_node *)0xdeadbeef,     \
			.bnm_child   = (struct dlist_node *)0xdeadbeef,        \
			.bnm_order   = 0xdeadbeef                              \
		},                                                             \
		.key = _key                                                    \
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
	cute_ensure(dlist_next(&bnmhut_heap.bnm_roots) ==
	            &node.heap.bnm_sibling);
	cute_ensure(node.heap.bnm_child == NULL);
	cute_ensure(node.heap.bnm_parent == NULL);
	cute_ensure(dlist_empty(&node.heap.bnm_sibling) == false);
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
	cute_ensure(dlist_empty(&bnmhut_heap.bnm_roots));
	cute_ensure(bnm_heap_count(&bnmhut_heap) == 0U);
}

static void bnmhut_check_roots(const struct bnm_heap* heap, unsigned int count)
{
	const struct bnm_heap_node *node;
	int                         order = -1;

	dlist_foreach_entry(&heap->bnm_roots, node, bnm_sibling) {
		while (!(count & 1)) {
			count >>= 1;
			order++;
		}
		order++;
		count >>= 1;

		cute_ensure(node->bnm_parent == NULL);
		cute_ensure(node->bnm_order == (unsigned int)order);
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
		const struct bnm_heap_node *node = NULL;
		const struct bnmhut_node   *check = checks[n];

		node = bnm_heap_peek(heap, bnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);

		node = NULL;
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

static CUTE_PNP_FIXTURED_SUITE(bnmhut_unsorted, &bnmhut, bnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(bnmhut_unsorted_increasing, &bnmhut_unsorted)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0),  /* 0 */
		BNMHUT_INIT_NODE(4),
		BNMHUT_INIT_NODE(5),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),  /* 5 */
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(10),
		BNMHUT_INIT_NODE(11),
		BNMHUT_INIT_NODE(12),
		BNMHUT_INIT_NODE(7),  /* 10 */
		BNMHUT_INIT_NODE(8),
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(16),
		BNMHUT_INIT_NODE(13),
		BNMHUT_INIT_NODE(14), /* 15 */
		BNMHUT_INIT_NODE(15)
	};

	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_unsorted_decreasing, &bnmhut_unsorted)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(6),  /* 0 */
		BNMHUT_INIT_NODE(5),
		BNMHUT_INIT_NODE(4),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(2),  /* 5 */
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(8),
		BNMHUT_INIT_NODE(7),
		BNMHUT_INIT_NODE(16), /* 10 */
		BNMHUT_INIT_NODE(12),
		BNMHUT_INIT_NODE(11),
		BNMHUT_INIT_NODE(10),
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(14), /* 15 */
		BNMHUT_INIT_NODE(13)
	};

	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_unsorted_diverge, &bnmhut_unsorted)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(4),  /* 0 */
		BNMHUT_INIT_NODE(5),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(1),  /* 5 */
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(10),
		BNMHUT_INIT_NODE(11),
		BNMHUT_INIT_NODE(12),
		BNMHUT_INIT_NODE(9),  /* 10 */
		BNMHUT_INIT_NODE(8),
		BNMHUT_INIT_NODE(7),
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(14),
		BNMHUT_INIT_NODE(16), /* 15 */
		BNMHUT_INIT_NODE(13)
	};

	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_unsorted_converge, &bnmhut_unsorted)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(16), /* 0 */
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(14),
		BNMHUT_INIT_NODE(13),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(1),  /* 5 */
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(12),
		BNMHUT_INIT_NODE(11),
		BNMHUT_INIT_NODE(10), /* 10 */
		BNMHUT_INIT_NODE(4),
		BNMHUT_INIT_NODE(5),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(7),  /* 15 */
		BNMHUT_INIT_NODE(8)
	};

	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_duplicates, &bnmhut, bnmhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(bnmhut_duplicates2, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(0)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_duplicates3, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(0)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_duplicates_leading, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(2)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_duplicates_trailing, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(0)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[2],
		&nodes[1],
		&nodes[0]
	};

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_duplicates_interleave, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(0)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_duplicates_mix, &bnmhut_duplicates)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(2),  /* 0 */
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(8),  /* 5 */
		BNMHUT_INIT_NODE(7),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(5),
		BNMHUT_INIT_NODE(4),
		BNMHUT_INIT_NODE(4),  /* 10 */
		BNMHUT_INIT_NODE(10),
		BNMHUT_INIT_NODE(11),
		BNMHUT_INIT_NODE(13),
		BNMHUT_INIT_NODE(8),
		BNMHUT_INIT_NODE(12), /* 15 */
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(9)
	};

	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap(&bnmhut_heap, nodes, checks, array_nr(nodes));
}

static CUTE_PNP_SUITE(bnmhut_merge, &bnmhut);

static void bnmhut_check_heap_merge(struct bnmhut_node  *first,
                                    unsigned int         first_count,
                                    struct bnmhut_node  *second,
                                    unsigned int         second_count,
                                    struct bnmhut_node **checks)
{
	struct bnm_heap fst;
	struct bnm_heap snd;
	unsigned int    n;

	bnm_heap_init(&fst);
	for (n = 0; n < first_count; n++) {
		bnm_heap_insert(&fst, &first[n].heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(&fst) == (n + 1));
	}
	bnmhut_check_roots(&fst, first_count);

	bnm_heap_init(&snd);
	for (n = 0; n < second_count; n++) {
		bnm_heap_insert(&snd, &second[n].heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(&snd) == (n + 1));
	}
	bnmhut_check_roots(&snd, second_count);

	bnm_heap_merge(&fst, &snd, bnmhut_compare_min);

	bnmhut_check_roots(&fst, first_count + second_count);

	for (n = 0; n < (first_count + second_count); n++) {
		const struct bnm_heap_node *node;
		const struct bnmhut_node   *check = checks[n];

		node = bnm_heap_extract(&fst, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(&fst) ==
		            first_count + second_count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(bnmhut_merge_inorder11, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(1)
	};
	static struct bnmhut_node *checks[] = {
		&fst[0],
		&snd[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_revorder11, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(1)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node *checks[] = {
		&snd[0],
		&fst[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_inorder12, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
	};
	static struct bnmhut_node *checks[] = {
		&fst[0],
		&snd[0],
		&snd[1]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_revorder12, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(2)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node *checks[] = {
		&snd[1],
		&snd[0],
		&fst[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_unsorted12, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(1)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&snd[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_unsorted22, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(0)
	};
	static struct bnmhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&fst[1],
		&snd[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_unsorted31, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(0),
		BNMHUT_INIT_NODE(1)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(2),
	};
	static struct bnmhut_node *checks[] = {
		&fst[1],
		&fst[2],
		&snd[0],
		&fst[0]
	};

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

CUTE_PNP_TEST(bnmhut_merge_mit, &bnmhut_merge)
{
	static struct bnmhut_node  fst[] = {
		BNMHUT_INIT_NODE(41),
		BNMHUT_INIT_NODE(28),
		BNMHUT_INIT_NODE(33),
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(7),
		BNMHUT_INIT_NODE(25),
		BNMHUT_INIT_NODE(12)
	};
	static struct bnmhut_node  snd[] = {
		BNMHUT_INIT_NODE(17),
		BNMHUT_INIT_NODE(10),
		BNMHUT_INIT_NODE(44),
		BNMHUT_INIT_NODE(50),
		BNMHUT_INIT_NODE(31),
		BNMHUT_INIT_NODE(48),
		BNMHUT_INIT_NODE(29),
		BNMHUT_INIT_NODE(8),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(24),
		BNMHUT_INIT_NODE(22),
		BNMHUT_INIT_NODE(23),
		BNMHUT_INIT_NODE(55),
		BNMHUT_INIT_NODE(32),
		BNMHUT_INIT_NODE(45),
		BNMHUT_INIT_NODE(30),
		BNMHUT_INIT_NODE(37),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(18)
	};
	static struct bnmhut_node *checks[] = {
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

	bnmhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd), checks);
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_update, &bnmhut, bnmhut_setup_empty,
                               NULL);

static void bnmhut_check_update(struct bnm_heap     *heap,
                                unsigned int         new_index,
                                int                  new_key,
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

	nodes[new_index].key = new_key;
	bnm_heap_update(&nodes[new_index].heap, bnmhut_compare_min);

	for (n = 0; n < count; n++) {
		const struct bnm_heap_node *node = NULL;
		const struct bnmhut_node   *check = checks[n];

		node = bnm_heap_peek(heap, bnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);

		node = NULL;
		node = bnm_heap_extract(heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);
	}
}

CUTE_PNP_TEST(bnmhut_update1, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(0)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[0]
	};

	bnmhut_check_update(&bnmhut_heap, 0, 2, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_up2, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	bnmhut_check_update(&bnmhut_heap, 1, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_down2, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	bnmhut_check_update(&bnmhut_heap, 0, 3, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_still2, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1]
	};

	bnmhut_check_update(&bnmhut_heap, 0, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_left_up3, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1]
	};

	bnmhut_check_update(&bnmhut_heap, 2, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_right_up3, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2]
	};

	bnmhut_check_update(&bnmhut_heap, 1, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_down3, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[0]
	};

	bnmhut_check_update(&bnmhut_heap, 0, 4, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_deep_left_up4, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(4)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[3],
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};

	bnmhut_check_update(&bnmhut_heap, 3, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_shallow_left_up4, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(4)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
		&nodes[1],
		&nodes[3]
	};

	bnmhut_check_update(&bnmhut_heap, 2, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_right_up4, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(4)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[0],
		&nodes[2],
		&nodes[3]
	};

	bnmhut_check_update(&bnmhut_heap, 1, 0, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_shallow_left_down4, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(4)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[0],
		&nodes[1],
		&nodes[3],
		&nodes[2]
	};

	bnmhut_check_update(&bnmhut_heap, 2, 5, nodes, checks, array_nr(nodes));
}

CUTE_PNP_TEST(bnmhut_update_root_left_down4, &bnmhut_update)
{
	static struct bnmhut_node nodes[] = {
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2),
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(4)
	};

	static struct bnmhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[3],
		&nodes[0]
	};

	bnmhut_check_update(&bnmhut_heap, 0, 5, nodes, checks, array_nr(nodes));
}

static struct bnmhut_node bnmhut_update_nodes[] = {
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(23),
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(21),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(18),
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(12)
};

CUTE_PNP_TEST(bnmhut_update_30, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 0, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_2322, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 1, 22, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up230, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[1],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
	};

	bnmhut_check_update(&bnmhut_heap, 1, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_1514, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 2, 14, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up150, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 2, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_65, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 4, 5, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up60, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 4, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_2120, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 3, 20, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up210, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 3, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up215, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 3, 5, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_1817, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 5, 17, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up180, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 5, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up185, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 5, 5, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_98, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 6, 8, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up90, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 6, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up95, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 6, 5, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_1211, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 7, 11, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up120, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 7, 0, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up125, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 7, 5, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_up128, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 7, 8, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_34, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 0, 4, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down37, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 0, 7, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down310, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 0, 10, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down314, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 0, 14, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down1522, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 2, 22, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down619, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 4, 19, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

CUTE_PNP_TEST(bnmhut_update_down610, &bnmhut_update)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_update_nodes[0],
		&bnmhut_update_nodes[6],
		&bnmhut_update_nodes[4],
		&bnmhut_update_nodes[7],
		&bnmhut_update_nodes[2],
		&bnmhut_update_nodes[5],
		&bnmhut_update_nodes[3],
		&bnmhut_update_nodes[1],
	};

	bnmhut_check_update(&bnmhut_heap, 4, 10, bnmhut_update_nodes, checks,
	                    array_nr(checks));
}

static CUTE_PNP_FIXTURED_SUITE(bnmhut_remove, &bnmhut, bnmhut_setup_empty,
                               NULL);

static void bnmhut_check_remove(struct bnm_heap     *heap,
                                unsigned int         index,
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

	bnm_heap_remove(heap, &nodes[index].heap, bnmhut_compare_min);

	for (n = 0; n < (count - 1); n++) {
		const struct bnm_heap_node *node = NULL;
		const struct bnmhut_node   *check = checks[n];

		node = bnm_heap_peek(heap, bnmhut_compare_min);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);

		node = NULL;
		node = bnm_heap_extract(heap, bnmhut_compare_min);

		cute_ensure(bnm_heap_count(heap) == count - n - 2);
		cute_ensure(node == &check->heap);
		cute_ensure(((struct bnmhut_node *)node)->key == check->key);
	}
}

static struct bnmhut_node bnmhut_remove_nodes[] = {
		BNMHUT_INIT_NODE(3),
		BNMHUT_INIT_NODE(23),
		BNMHUT_INIT_NODE(15),
		BNMHUT_INIT_NODE(21),
		BNMHUT_INIT_NODE(6),
		BNMHUT_INIT_NODE(18),
		BNMHUT_INIT_NODE(9),
		BNMHUT_INIT_NODE(12),
		BNMHUT_INIT_NODE(27),
		BNMHUT_INIT_NODE(1),
		BNMHUT_INIT_NODE(2)
};

CUTE_PNP_TEST(bnmhut_remove1, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 9, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove2, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 10, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove3, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 0, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove6, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 4, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove9, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 6, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove12, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 7, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove15, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 2, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove18, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 5, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove21, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[1],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 3, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove23, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[8]
	};

	bnmhut_check_remove(&bnmhut_heap, 1, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}

CUTE_PNP_TEST(bnmhut_remove27, &bnmhut_remove)
{
	static struct bnmhut_node *checks[] = {
		&bnmhut_remove_nodes[9],
		&bnmhut_remove_nodes[10],
		&bnmhut_remove_nodes[0],
		&bnmhut_remove_nodes[4],
		&bnmhut_remove_nodes[6],
		&bnmhut_remove_nodes[7],
		&bnmhut_remove_nodes[2],
		&bnmhut_remove_nodes[5],
		&bnmhut_remove_nodes[3],
		&bnmhut_remove_nodes[1]
	};

	bnmhut_check_remove(&bnmhut_heap, 8, bnmhut_remove_nodes, checks,
	                    array_nr(bnmhut_remove_nodes));
}
