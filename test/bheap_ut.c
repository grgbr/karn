/**
 * @file      bheap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      07 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary heap unit tests implementation
 *
 * @defgroup bheaput_fixed Fixed length array based binary heap unit tests
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

#include "bheap.h"
#include <cute/cute.h>

static struct bheap_fixed bheaput_heap;

static unsigned int bheaput_nodes[8];

static int bheaput_compare_min(const char *first, const char *second)
{
	return *(unsigned int *)first - *(unsigned int *)second;
}

static CUTE_PNP_SUITE(bheaput, NULL);

static void
bheaput_setup_empty(void)
{
	bheap_init_fixed(&bheaput_heap, (char *)bheaput_nodes,
	                 sizeof(bheaput_nodes[0]), array_nr(bheaput_nodes));
}

static CUTE_PNP_FIXTURED_SUITE(bheaput_empty, &bheaput,
                               bheaput_setup_empty, NULL);

/**
 * Check an empty bheap_fixed is really exposed as empty and not full
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_check_emptiness, &bheaput_empty)
{
	cute_ensure(bheap_fixed_empty(&bheaput_heap) == true);
	cute_ensure(bheap_fixed_full(&bheaput_heap) == false);
}

/**
 * Insert a single node into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_single, &bheaput_empty)
{
	unsigned int node = 10;

	bheap_insert_fixed(&bheaput_heap, (char *)&node, bheaput_compare_min);
	cute_ensure(bheap_fixed_empty(&bheaput_heap) == false);
	cute_ensure(bheap_fixed_full(&bheaput_heap) == false);

	cute_ensure(*(unsigned int *)bheap_peek_fixed(&bheaput_heap) == 10U);
}

/**
 * Extract a node from a bheap_fixed storing a single node
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_single, &bheaput_empty)
{
	unsigned int inode = 11;
	unsigned int enode = 0;

	bheap_insert_fixed(&bheaput_heap, (char *)&inode, bheaput_compare_min);

	cute_ensure(bheap_fixed_empty(&bheaput_heap) == false);
	cute_ensure(bheap_fixed_full(&bheaput_heap) == false);

	bheap_extract_fixed(&bheaput_heap, (char *)&enode, bheaput_compare_min);
	cute_ensure(bheap_fixed_empty(&bheaput_heap) == true);
	cute_ensure(bheap_fixed_full(&bheaput_heap) == false);

	cute_ensure(enode == 11U);
}

static CUTE_PNP_SUITE(bheaput_multiple, &bheaput);

static void bheaput_check_nodes(const struct bheap_fixed *heap,
                                unsigned int              nr)
{
	unsigned int n;

	for (n = 1; n < nr; n++) {
		const int *node = (int *)
		                  array_fixed_item(&heap->bheap_tree.bst_nodes,
			                               sizeof(*node), n);
		cute_ensure(*((int *)
		              bstree_fixed_parent(&heap->bheap_tree, sizeof(*node),
		                                  (char *)node)) <= *node);
	}
}

static void bheap_check_inorder(const int *nodes, unsigned int nr)
{
	int i, j;

	for (i = 1; (unsigned int)i < nr; i++) {
		bheap_init_fixed(&bheaput_heap, (char *)bheaput_nodes,
		                 sizeof(bheaput_nodes[0]), array_nr(bheaput_nodes));

		for (j = 0; j < i; j++)
			bheap_insert_fixed(&bheaput_heap, (char *)&nodes[j],
			                   bheaput_compare_min);

		bheaput_check_nodes(&bheaput_heap, i);
	}
}

CUTE_PNP_TEST(bheaput_inorder, &bheaput_multiple)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	bheap_check_inorder(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_revorder, &bheaput_multiple)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	bheap_check_inorder(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_mixorder, &bheaput_multiple)
{
	int nodes[] = { 8, 6, 7, 5, 1, 3, 2, 4 };

	bheap_check_inorder(nodes, array_nr(nodes));
}
