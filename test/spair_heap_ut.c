/**
 * @file      spair_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      22 Aug 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list based binomial heap unit tests implementation
 *
 * @defgroup spairhut Singly linked list based binomial heap unit tests
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

#include "spair_heap.h"
#include <cute/cute.h>

struct spairhut_node {
	struct lcrs_node heap;
	int              key;
};

#define SPAIRHUT_INIT_NODE(_key)                                         \
	{                                                                \
		.heap = {                                                \
			.lcrs_sibling  = (struct lcrs_node *)0xdeadbeef, \
			.lcrs_youngest = (struct lcrs_node *)0xdeadbeef, \
		},                                                       \
		.key = _key                                              \
	}

static struct spair_heap spairhut_heap;

static int spairhut_compare_min(const struct lcrs_node *restrict first,
                                const struct lcrs_node *restrict second)
{
	cute_ensure(first);
	cute_ensure(!lcrs_istail(first));
	cute_ensure(second);
	cute_ensure(!lcrs_istail(second));

	return ((struct spairhut_node *)first)->key -
	       ((struct spairhut_node *)second)->key;
}

static CUTE_PNP_SUITE(spairhut, NULL);

static void spairhut_setup_empty(void)
{
	spair_heap_init(&spairhut_heap);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_empty, &spairhut, spairhut_setup_empty,
                               NULL);

CUTE_PNP_TEST(spairhut_check_emptiness, &spairhut_empty)
{
	cute_ensure(spair_heap_empty(&spairhut_heap) == true);
}

CUTE_PNP_TEST(spairhut_insert_single, &spairhut_empty)
{
	struct spairhut_node node = SPAIRHUT_INIT_NODE(2);

	spair_heap_insert(&spairhut_heap, &node.heap, spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) == 1U);
	cute_ensure(spairhut_heap.spair_root == &node.heap);
	cute_ensure(!lcrs_has_child(&node.heap));
	cute_ensure(node.heap.lcrs_sibling == lcrs_mktail(NULL));
}

CUTE_PNP_TEST(spairhut_peek_single, &spairhut_empty)
{
	struct spairhut_node node = SPAIRHUT_INIT_NODE(2);

	spair_heap_insert(&spairhut_heap, &node.heap, spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) == 1U);
	cute_ensure(spair_heap_peek(&spairhut_heap) == &node.heap);
	cute_ensure(spair_heap_count(&spairhut_heap) == 1U);
}

CUTE_PNP_TEST(spairhut_extract_single, &spairhut_empty)
{
	struct spairhut_node node = SPAIRHUT_INIT_NODE(2);

	spair_heap_insert(&spairhut_heap, &node.heap, spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) == 1U);
	cute_ensure(spair_heap_extract(&spairhut_heap, spairhut_compare_min) ==
	            &node.heap);
	cute_ensure(spairhut_heap.spair_root == NULL);
	cute_ensure(spair_heap_count(&spairhut_heap) == 0U);
}

CUTE_PNP_TEST(spairhut_remove_single, &spairhut_empty)
{
	struct spairhut_node node = SPAIRHUT_INIT_NODE(2);

	spair_heap_insert(&spairhut_heap, &node.heap, spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) == 1U);
	spair_heap_remove(&spairhut_heap, &node.heap, spairhut_compare_min);
	cute_ensure(spairhut_heap.spair_root == NULL);
	cute_ensure(spair_heap_count(&spairhut_heap) == 0U);
}

static void spairhut_check_root(const struct lcrs_node *parent,
                                lcrs_compare_fn        *compare)
{
	if (lcrs_has_child(parent)) {
		const struct lcrs_node *node = lcrs_youngest(parent);

		do {
			cute_ensure(compare(parent, node) <= 0);
			spairhut_check_root(node, compare);
			node = lcrs_next(node);
		} while (!lcrs_istail(node));
	}
}

static void spairhut_check_heap_nodes(struct spair_heap     *heap,
                                      struct spairhut_node **checks,
                                      unsigned int           count,
                                      lcrs_compare_fn       *compare)
{
	unsigned int n;

	spairhut_check_root(heap->spair_root, compare);

	for (n = 0; n < count; n++) {
		const struct lcrs_node     *node = NULL;
		const struct spairhut_node *check = checks[n];

		node = spair_heap_peek(heap);
		cute_ensure(node == &check->heap);
		cute_ensure(!compare(node, &check->heap));

		node = NULL;
		node = spair_heap_extract(heap, compare);
		cute_ensure(spair_heap_count(heap) == count - n - 1);
		cute_ensure(node == &check->heap);
		cute_ensure(!compare(node, &check->heap));
	}
}

static void spairhut_fill_heap(struct spair_heap    *heap,
                               struct spairhut_node *nodes,
                               unsigned int          count,
                               lcrs_compare_fn      *compare)
{
	unsigned int n;

	for (n = 0; n < count; n++) {
		spair_heap_insert(heap, &nodes[n].heap, compare);

		cute_ensure(spair_heap_count(heap) == (n + 1));
	}
}

static void spairhut_check_heap(struct spair_heap     *heap,
                                struct spairhut_node  *nodes,
                                struct spairhut_node **checks,
                                unsigned int           count,
                                lcrs_compare_fn      *compare)
{
	spairhut_fill_heap(heap, nodes, count, compare);
	spairhut_check_heap_nodes(heap, checks, count, compare);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_inorder, &spairhut,
                               spairhut_setup_empty, NULL);

static struct spairhut_node spairhut_inorder_nodes[] = {
	SPAIRHUT_INIT_NODE(0),
	SPAIRHUT_INIT_NODE(1),
	SPAIRHUT_INIT_NODE(2),
	SPAIRHUT_INIT_NODE(3),
	SPAIRHUT_INIT_NODE(4),
	SPAIRHUT_INIT_NODE(5),
	SPAIRHUT_INIT_NODE(6),
	SPAIRHUT_INIT_NODE(7),
	SPAIRHUT_INIT_NODE(8),
	SPAIRHUT_INIT_NODE(9),
	SPAIRHUT_INIT_NODE(10),
	SPAIRHUT_INIT_NODE(11),
	SPAIRHUT_INIT_NODE(12),
	SPAIRHUT_INIT_NODE(13),
	SPAIRHUT_INIT_NODE(14),
	SPAIRHUT_INIT_NODE(15),
	SPAIRHUT_INIT_NODE(16)
};

static struct spairhut_node *spairhut_inorder_checks[] = {
	&spairhut_inorder_nodes[0],
	&spairhut_inorder_nodes[1],
	&spairhut_inorder_nodes[2],
	&spairhut_inorder_nodes[3],
	&spairhut_inorder_nodes[4],
	&spairhut_inorder_nodes[5],
	&spairhut_inorder_nodes[6],
	&spairhut_inorder_nodes[7],
	&spairhut_inorder_nodes[8],
	&spairhut_inorder_nodes[9],
	&spairhut_inorder_nodes[10],
	&spairhut_inorder_nodes[11],
	&spairhut_inorder_nodes[12],
	&spairhut_inorder_nodes[13],
	&spairhut_inorder_nodes[14],
	&spairhut_inorder_nodes[15],
	&spairhut_inorder_nodes[16]
};

CUTE_PNP_TEST(spairhut_inorder2, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                    spairhut_inorder_checks, 2, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder3, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                    spairhut_inorder_checks, 3, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder4, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 4, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder5, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 5, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder6, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 6, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder7, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 7, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder8, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 8, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder9, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 9, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder10, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 10, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder11, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 11, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder12, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 12, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder13, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 13, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder14, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 14, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder15, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 15, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder16, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 16, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_inorder17, &spairhut_inorder)
{
	spairhut_check_heap(&spairhut_heap, spairhut_inorder_nodes,
	                   spairhut_inorder_checks, 17, spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_revorder, &spairhut,
                               spairhut_setup_empty, NULL);

static struct spairhut_node spairhut_revorder_nodes[] = {
	SPAIRHUT_INIT_NODE(16),
	SPAIRHUT_INIT_NODE(15),
	SPAIRHUT_INIT_NODE(14),
	SPAIRHUT_INIT_NODE(13),
	SPAIRHUT_INIT_NODE(12),
	SPAIRHUT_INIT_NODE(11),
	SPAIRHUT_INIT_NODE(10),
	SPAIRHUT_INIT_NODE(9),
	SPAIRHUT_INIT_NODE(8),
	SPAIRHUT_INIT_NODE(7),
	SPAIRHUT_INIT_NODE(6),
	SPAIRHUT_INIT_NODE(5),
	SPAIRHUT_INIT_NODE(4),
	SPAIRHUT_INIT_NODE(3),
	SPAIRHUT_INIT_NODE(2),
	SPAIRHUT_INIT_NODE(1),
	SPAIRHUT_INIT_NODE(0)
};

static struct spairhut_node *spairhut_revorder_checks[] = {
	&spairhut_revorder_nodes[16],
	&spairhut_revorder_nodes[15],
	&spairhut_revorder_nodes[14],
	&spairhut_revorder_nodes[13],
	&spairhut_revorder_nodes[12],
	&spairhut_revorder_nodes[11],
	&spairhut_revorder_nodes[10],
	&spairhut_revorder_nodes[9],
	&spairhut_revorder_nodes[8],
	&spairhut_revorder_nodes[7],
	&spairhut_revorder_nodes[6],
	&spairhut_revorder_nodes[5],
	&spairhut_revorder_nodes[4],
	&spairhut_revorder_nodes[3],
	&spairhut_revorder_nodes[2],
	&spairhut_revorder_nodes[1],
	&spairhut_revorder_nodes[0]
};

CUTE_PNP_TEST(spairhut_revorder2, &spairhut_revorder)
{
	unsigned int           count = 2;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder3, &spairhut_revorder)
{
	unsigned int           count = 3;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder4, &spairhut_revorder)
{
	unsigned int           count = 4;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder5, &spairhut_revorder)
{
	unsigned int           count = 5;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                   count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder6, &spairhut_revorder)
{
	unsigned int           count = 6;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder7, &spairhut_revorder)
{
	unsigned int           count = 7;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder8, &spairhut_revorder)
{
	unsigned int           count = 8;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder9, &spairhut_revorder)
{
	unsigned int           count = 9;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder10, &spairhut_revorder)
{
	unsigned int           count = 10;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder11, &spairhut_revorder)
{
	unsigned int           count = 11;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder12, &spairhut_revorder)
{
	unsigned int           count = 12;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder13, &spairhut_revorder)
{
	unsigned int           count = 13;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder14, &spairhut_revorder)
{
	unsigned int           count = 14;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder15, &spairhut_revorder)
{
	unsigned int           count = 15;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder16, &spairhut_revorder)
{
	unsigned int           count = 16;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_revorder17, &spairhut_revorder)
{
	unsigned int           count = 17;
	struct spairhut_node **checks =
		&spairhut_revorder_checks[array_nr(spairhut_revorder_checks) -
		                          count];

	spairhut_check_heap(&spairhut_heap, spairhut_revorder_nodes, checks,
	                    count, spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_unsorted, &spairhut,
                               spairhut_setup_empty, NULL);

CUTE_PNP_TEST(spairhut_unsorted_increasing, &spairhut_unsorted)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),  /* 0 */
		SPAIRHUT_INIT_NODE(4),
		SPAIRHUT_INIT_NODE(5),
		SPAIRHUT_INIT_NODE(6),
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(2),  /* 5 */
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(10),
		SPAIRHUT_INIT_NODE(11),
		SPAIRHUT_INIT_NODE(12),
		SPAIRHUT_INIT_NODE(7),  /* 10 */
		SPAIRHUT_INIT_NODE(8),
		SPAIRHUT_INIT_NODE(9),
		SPAIRHUT_INIT_NODE(16),
		SPAIRHUT_INIT_NODE(13),
		SPAIRHUT_INIT_NODE(14), /* 15 */
		SPAIRHUT_INIT_NODE(15)
	};

	struct spairhut_node *checks[] = {
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

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_unsorted_decreasing, &spairhut_unsorted)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(6),  /* 0 */
		SPAIRHUT_INIT_NODE(5),
		SPAIRHUT_INIT_NODE(4),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(2),  /* 5 */
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(9),
		SPAIRHUT_INIT_NODE(8),
		SPAIRHUT_INIT_NODE(7),
		SPAIRHUT_INIT_NODE(16), /* 10 */
		SPAIRHUT_INIT_NODE(12),
		SPAIRHUT_INIT_NODE(11),
		SPAIRHUT_INIT_NODE(10),
		SPAIRHUT_INIT_NODE(15),
		SPAIRHUT_INIT_NODE(14), /* 15 */
		SPAIRHUT_INIT_NODE(13)
	};

	struct spairhut_node *checks[] = {
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

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_unsorted_diverge, &spairhut_unsorted)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(4),  /* 0 */
		SPAIRHUT_INIT_NODE(5),
		SPAIRHUT_INIT_NODE(6),
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(1),  /* 5 */
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(10),
		SPAIRHUT_INIT_NODE(11),
		SPAIRHUT_INIT_NODE(12),
		SPAIRHUT_INIT_NODE(9),  /* 10 */
		SPAIRHUT_INIT_NODE(8),
		SPAIRHUT_INIT_NODE(7),
		SPAIRHUT_INIT_NODE(15),
		SPAIRHUT_INIT_NODE(14),
		SPAIRHUT_INIT_NODE(16), /* 15 */
		SPAIRHUT_INIT_NODE(13)
	};

	struct spairhut_node *checks[] = {
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

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_unsorted_converge, &spairhut_unsorted)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(16), /* 0 */
		SPAIRHUT_INIT_NODE(15),
		SPAIRHUT_INIT_NODE(14),
		SPAIRHUT_INIT_NODE(13),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(1),  /* 5 */
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(12),
		SPAIRHUT_INIT_NODE(11),
		SPAIRHUT_INIT_NODE(10), /* 10 */
		SPAIRHUT_INIT_NODE(4),
		SPAIRHUT_INIT_NODE(5),
		SPAIRHUT_INIT_NODE(6),
		SPAIRHUT_INIT_NODE(9),
		SPAIRHUT_INIT_NODE(7),  /* 15 */
		SPAIRHUT_INIT_NODE(8)
	};

	struct spairhut_node *checks[] = {
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

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_duplicates, &spairhut,
                               spairhut_setup_empty, NULL);

CUTE_PNP_TEST(spairhut_duplicates2, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(0)
	};

	struct spairhut_node *checks[] = {
		&nodes[0],
		&nodes[1]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_duplicates3, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(0)
	};

	struct spairhut_node *checks[] = {
		&nodes[0],
		&nodes[2],
		&nodes[1]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_duplicates_leading, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(2)
	};

	struct spairhut_node *checks[] = {
		&nodes[0],
		&nodes[1],
		&nodes[2]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_duplicates_trailing, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(0)
	};

	struct spairhut_node *checks[] = {
		&nodes[1],
		&nodes[2],
		&nodes[0]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_duplicates_interleave, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(0)
	};

	struct spairhut_node *checks[] = {
		&nodes[0],
		&nodes[2],
		&nodes[1]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_duplicates_mix, &spairhut_duplicates)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(2),  /* 0 */
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(8),  /* 5 */
		SPAIRHUT_INIT_NODE(7),
		SPAIRHUT_INIT_NODE(6),
		SPAIRHUT_INIT_NODE(5),
		SPAIRHUT_INIT_NODE(4),
		SPAIRHUT_INIT_NODE(4),  /* 10 */
		SPAIRHUT_INIT_NODE(10),
		SPAIRHUT_INIT_NODE(11),
		SPAIRHUT_INIT_NODE(13),
		SPAIRHUT_INIT_NODE(8),
		SPAIRHUT_INIT_NODE(12), /* 15 */
		SPAIRHUT_INIT_NODE(9),
		SPAIRHUT_INIT_NODE(9)
	};

	struct spairhut_node *checks[] = {
		&nodes[2],
		&nodes[3],
		&nodes[0],
		&nodes[1],
		&nodes[4],
		&nodes[10],
		&nodes[9],
		&nodes[8],
		&nodes[7],
		&nodes[6],
		&nodes[14],
		&nodes[5],
		&nodes[17],
		&nodes[16],
		&nodes[11],
		&nodes[12],
		&nodes[15],
		&nodes[13]
	};

	spairhut_check_heap(&spairhut_heap, nodes, checks, array_nr(nodes),
	                    spairhut_compare_min);
}

static CUTE_PNP_SUITE(spairhut_merge, &spairhut);

static void spairhut_check_heap_merge(struct spairhut_node  *first,
                                     unsigned int            first_count,
                                     struct spairhut_node   *second,
                                     unsigned int            second_count,
                                     struct spairhut_node  **checks,
                                     lcrs_compare_fn       *compare)
{
	struct spair_heap fst;
	struct spair_heap snd;
	unsigned int      n;

	spair_heap_init(&fst);
	for (n = 0; n < first_count; n++) {
		spair_heap_insert(&fst, &first[n].heap, compare);

		cute_ensure(spair_heap_count(&fst) == (n + 1));
	}
	spairhut_check_root(fst.spair_root, compare);

	spair_heap_init(&snd);
	for (n = 0; n < second_count; n++) {
		spair_heap_insert(&snd, &second[n].heap, compare);

		cute_ensure(spair_heap_count(&snd) == (n + 1));
	}
	spairhut_check_root(snd.spair_root, compare);

	spair_heap_merge(&fst, &snd, compare);

	spairhut_check_heap_nodes(&fst, checks, first_count + second_count,
	                          compare);
}

CUTE_PNP_TEST(spairhut_merge_inorder11, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node *checks[] = {
		&fst[0],
		&snd[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_revorder11, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node *checks[] = {
		&snd[0],
		&fst[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_inorder12, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(2)
	};
	struct spairhut_node *checks[] = {
		&fst[0],
		&snd[0],
		&snd[1]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_revorder12, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(2)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node *checks[] = {
		&snd[1],
		&snd[0],
		&fst[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_unsorted12, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&snd[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_unsorted22, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(2)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node *checks[] = {
		&snd[1],
		&fst[0],
		&fst[1],
		&snd[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_unsorted31, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(2),
	};
	struct spairhut_node *checks[] = {
		&fst[1],
		&fst[2],
		&snd[0],
		&fst[0]
	};

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_merge_mit, &spairhut_merge)
{
	struct spairhut_node  fst[] = {
		SPAIRHUT_INIT_NODE(41),
		SPAIRHUT_INIT_NODE(28),
		SPAIRHUT_INIT_NODE(33),
		SPAIRHUT_INIT_NODE(15),
		SPAIRHUT_INIT_NODE(7),
		SPAIRHUT_INIT_NODE(25),
		SPAIRHUT_INIT_NODE(12)
	};
	struct spairhut_node  snd[] = {
		SPAIRHUT_INIT_NODE(17),
		SPAIRHUT_INIT_NODE(10),
		SPAIRHUT_INIT_NODE(44),
		SPAIRHUT_INIT_NODE(50),
		SPAIRHUT_INIT_NODE(31),
		SPAIRHUT_INIT_NODE(48),
		SPAIRHUT_INIT_NODE(29),
		SPAIRHUT_INIT_NODE(8),
		SPAIRHUT_INIT_NODE(6),
		SPAIRHUT_INIT_NODE(24),
		SPAIRHUT_INIT_NODE(22),
		SPAIRHUT_INIT_NODE(23),
		SPAIRHUT_INIT_NODE(55),
		SPAIRHUT_INIT_NODE(32),
		SPAIRHUT_INIT_NODE(45),
		SPAIRHUT_INIT_NODE(30),
		SPAIRHUT_INIT_NODE(37),
		SPAIRHUT_INIT_NODE(3),
		SPAIRHUT_INIT_NODE(18)
	};
	struct spairhut_node *checks[] = {
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

	spairhut_check_heap_merge(fst, array_nr(fst), snd, array_nr(snd),
	                          checks, spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_remove, &spairhut, spairhut_setup_empty,
                               NULL);

static void spairhut_check_heap_remove(struct spairhut_node   *nodes,
                                       struct spairhut_node   *removed,
                                       struct spairhut_node  **checks,
                                       unsigned int            count,
                                       lcrs_compare_fn        *compare)
{
	spairhut_fill_heap(&spairhut_heap, nodes, count, compare);

	spair_heap_remove(&spairhut_heap, &removed->heap, compare);

	count--;
	cute_ensure(spair_heap_count(&spairhut_heap) == count);

	spairhut_check_heap_nodes(&spairhut_heap, checks, count, compare);
}

CUTE_PNP_TEST(spairhut_remove_top, &spairhut_remove)
{
	struct spairhut_node        nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(2)
	};
	struct spairhut_node *checks[] = {
		&nodes[1]
	};

	spairhut_check_heap_remove(nodes, &nodes[0], checks, array_nr(nodes),
	                           spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_remove_bottom, &spairhut_remove)
{
	struct spairhut_node  nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(2)
	};
	struct spairhut_node *checks[] = {
		&nodes[0]
	};

	spairhut_check_heap_remove(nodes, &nodes[1], checks, array_nr(nodes),
	                           spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_remove_middle, &spairhut_remove)
{
	struct spairhut_node        nodes[] = {
		SPAIRHUT_INIT_NODE(1),
		SPAIRHUT_INIT_NODE(2),
		SPAIRHUT_INIT_NODE(0)
	};
	struct spairhut_node *checks[] = {
		&nodes[2],
		&nodes[0],
	};

	spairhut_check_heap_remove(nodes, &nodes[1], checks, array_nr(nodes),
	                           spairhut_compare_min);
}

static struct spairhut_node spairhut_nodes[] = {
	SPAIRHUT_INIT_NODE(11),
	SPAIRHUT_INIT_NODE(12),
	SPAIRHUT_INIT_NODE(18),
	SPAIRHUT_INIT_NODE(10),
	SPAIRHUT_INIT_NODE(14),
	SPAIRHUT_INIT_NODE(15),
	SPAIRHUT_INIT_NODE(21),
	SPAIRHUT_INIT_NODE(17),
	SPAIRHUT_INIT_NODE(13),
	SPAIRHUT_INIT_NODE(16),
	SPAIRHUT_INIT_NODE(20),
	SPAIRHUT_INIT_NODE(19)
};

CUTE_PNP_TEST(spairhut_remove_inorder, &spairhut_remove)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[3],
		/* &spairhut_nodes[0], */
		/* &spairhut_nodes[1], */
		&spairhut_nodes[8],
		/* &spairhut_nodes[4], */
		&spairhut_nodes[5],
		&spairhut_nodes[9],
		/* &spairhut_nodes[7], */
		&spairhut_nodes[2],
		&spairhut_nodes[11],
		&spairhut_nodes[10],
		/* &spairhut_nodes[6] */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[0].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[1].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[4].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[7].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[6].heap,
	                  spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            (array_nr(spairhut_nodes) - 5));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes) - 5,
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_remove_revorder, &spairhut_remove)
{
	struct spairhut_node *checks[] = {
		/* &spairhut_nodes[3], */
		&spairhut_nodes[0],
		&spairhut_nodes[1],
		&spairhut_nodes[8],
		/* &spairhut_nodes[4], */
		&spairhut_nodes[5],
		&spairhut_nodes[9],
		/* &spairhut_nodes[7], */
		&spairhut_nodes[2],
		/* &spairhut_nodes[11], */
		/* &spairhut_nodes[10], */
		&spairhut_nodes[6]
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[10].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[11].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[7].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[4].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[3].heap,
	                  spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            (array_nr(spairhut_nodes) - 5));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes) - 5,
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_remove_altorder, &spairhut_remove)
{
	struct spairhut_node *checks[] = {
		/* &spairhut_nodes[3], */
		&spairhut_nodes[0],
		&spairhut_nodes[1],
		/* &spairhut_nodes[8], */
		&spairhut_nodes[4],
		/* &spairhut_nodes[5], */
		/* &spairhut_nodes[9], */
		&spairhut_nodes[7],
		&spairhut_nodes[2],
		/* &spairhut_nodes[11], */
		&spairhut_nodes[10],
		&spairhut_nodes[6]
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[5].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[9].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[8].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[11].heap,
	                  spairhut_compare_min);
	spair_heap_remove(&spairhut_heap,
	                  &spairhut_nodes[3].heap,
	                  spairhut_compare_min);

	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            (array_nr(spairhut_nodes) - 5));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes) - 5,
	                          spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_promote, &spairhut,
                               spairhut_setup_empty, NULL);

CUTE_PNP_TEST(spairhut_promote_inorder, &spairhut_promote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[4],  /* 8 */
		&spairhut_nodes[1],  /* 9 */
		&spairhut_nodes[3],  /* 10 */
		&spairhut_nodes[0],  /* 10 */
		&spairhut_nodes[7],  /* 12 */
		&spairhut_nodes[8],  /* 13 */
		&spairhut_nodes[5],  /* 15 */
		&spairhut_nodes[9],  /* 16 */
		&spairhut_nodes[6],  /* 17 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[11], /* 19 */
		&spairhut_nodes[10]  /* 20 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[0].key -= 1;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[0].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[1].key -= 3;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[1].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[4].key -= 6;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[4].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[7].key -= 5;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[7].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[6].key -= 4;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[6].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_promote_revorder, &spairhut_promote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[7],  /* 7 */
		&spairhut_nodes[3],  /* 8 */
		&spairhut_nodes[4],  /* 9 */
		&spairhut_nodes[0],  /* 11 */
		&spairhut_nodes[1],  /* 12 */
		&spairhut_nodes[8],  /* 13 */
		&spairhut_nodes[5],  /* 15 */
		&spairhut_nodes[11], /* 16 */
		&spairhut_nodes[9],  /* 16 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[10], /* 19 */
		&spairhut_nodes[6]   /* 21 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[10].key -= 1;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[10].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[11].key -= 3;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[11].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[7].key -= 10;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[7].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[4].key -= 5;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[4].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[3].key -= 2;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[3].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_promote_altorder, &spairhut_promote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[8],  /* 8 */
		&spairhut_nodes[9],  /* 9 */
		&spairhut_nodes[3],  /* 9 */
		&spairhut_nodes[0],  /* 11 */
		&spairhut_nodes[1],  /* 12 */
		&spairhut_nodes[5],  /* 13 */
		&spairhut_nodes[4],  /* 14 */
		&spairhut_nodes[11], /* 16 */
		&spairhut_nodes[7],  /* 17 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[10], /* 20 */
		&spairhut_nodes[6]   /* 21 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[5].key -= 2;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[5].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[9].key -= 7;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[9].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[8].key -= 5;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[8].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[11].key -= 3;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[11].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[3].key -= 1;
	spair_heap_promote(&spairhut_heap,
	                   &spairhut_nodes[3].heap,
	                   spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}

static CUTE_PNP_FIXTURED_SUITE(spairhut_demote, &spairhut,
                               spairhut_setup_empty, NULL);

CUTE_PNP_TEST(spairhut_demote_single, &spairhut_demote)
{
	static struct spairhut_node nodes[] = {
		SPAIRHUT_INIT_NODE(0)
	};

	struct spairhut_node *checks[] = {
		&nodes[0]
	};

	spairhut_fill_heap(&spairhut_heap, nodes, array_nr(nodes),
	                   spairhut_compare_min);

	nodes[0].key += 1;
	spair_heap_demote(&spairhut_heap, &nodes[0].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) == array_nr(nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks, array_nr(nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_demote_top, &spairhut_demote)
{
	static struct spairhut_node nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node *checks[] = {
		&nodes[1],
		&nodes[0]
	};

	spairhut_fill_heap(&spairhut_heap, nodes, array_nr(nodes),
	                   spairhut_compare_min);

	nodes[0].key += 2;
	spair_heap_demote(&spairhut_heap, &nodes[0].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) == array_nr(nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks, array_nr(nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_demote_bottom, &spairhut_demote)
{
	static struct spairhut_node nodes[] = {
		SPAIRHUT_INIT_NODE(0),
		SPAIRHUT_INIT_NODE(1)
	};
	struct spairhut_node *checks[] = {
		&nodes[0],
		&nodes[1]
	};

	spairhut_fill_heap(&spairhut_heap, nodes, array_nr(nodes),
	                   spairhut_compare_min);

	nodes[1].key += 2;
	spair_heap_demote(&spairhut_heap, &nodes[1].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) == array_nr(nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks, array_nr(nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_demote_inorder, &spairhut_demote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[3],  /* 10 */
		&spairhut_nodes[0],  /* 12 */
		&spairhut_nodes[8],  /* 13 */
		&spairhut_nodes[5],  /* 15 */
		&spairhut_nodes[1],  /* 15 */
		&spairhut_nodes[9],  /* 16 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[11], /* 19 */
		&spairhut_nodes[10], /* 20 */
		&spairhut_nodes[4],  /* 20 */
		&spairhut_nodes[7],  /* 22 */
		&spairhut_nodes[6]   /* 25 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[0].key += 1;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[0].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[1].key += 3;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[1].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[4].key += 6;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[4].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[7].key += 5;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[7].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[6].key += 4;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[6].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_demote_revorder, &spairhut_demote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[0],  /* 11 */
		&spairhut_nodes[1],  /* 12 */
		&spairhut_nodes[8],  /* 13 */
		&spairhut_nodes[5],  /* 15 */
		&spairhut_nodes[9],  /* 16 */
		&spairhut_nodes[4],  /* 17 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[3],  /* 19 */
		&spairhut_nodes[11], /* 20 */
		&spairhut_nodes[6],  /* 21 */
		&spairhut_nodes[10], /* 22 */
		&spairhut_nodes[7]   /* 24 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[10].key += 2;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[10].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[11].key += 1;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[11].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[7].key += 7;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[7].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[4].key += 3;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[4].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[3].key += 9;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[3].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}

CUTE_PNP_TEST(spairhut_demote_altorder, &spairhut_demote)
{
	struct spairhut_node *checks[] = {
		&spairhut_nodes[0],  /* 11 */
		&spairhut_nodes[1],  /* 12 */
		&spairhut_nodes[3],  /* 13 */
		&spairhut_nodes[4],  /* 14 */
		&spairhut_nodes[8],  /* 15 */
		&spairhut_nodes[5],  /* 16 */
		&spairhut_nodes[7],  /* 17 */
		&spairhut_nodes[2],  /* 18 */
		&spairhut_nodes[10], /* 20 */
		&spairhut_nodes[6],  /* 21 */
		&spairhut_nodes[11], /* 22 */
		&spairhut_nodes[9]   /* 24 */
	};

	spairhut_fill_heap(&spairhut_heap, spairhut_nodes,
	                   array_nr(spairhut_nodes),
	                   spairhut_compare_min);

	spairhut_nodes[5].key += 1;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[10].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[9].key += 8;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[11].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[8].key += 2;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[7].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[11].key += 3;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[4].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_nodes[3].key += 3;
	spair_heap_demote(&spairhut_heap,
	                  &spairhut_nodes[3].heap,
	                  spairhut_compare_min);
	cute_ensure(spair_heap_count(&spairhut_heap) ==
	            array_nr(spairhut_nodes));

	spairhut_check_heap_nodes(&spairhut_heap, checks,
	                          array_nr(spairhut_nodes),
	                          spairhut_compare_min);
}
