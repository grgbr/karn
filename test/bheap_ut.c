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
#include <stdlib.h>
#include <string.h>

static struct bheap_fixed bheaput_heap;

static int bheaput_nodes[20];

static int bheaput_compare_min(const char *first, const char *second)
{
	return *(int *)first - *(int *)second;
}

static int bheaput_qsort_compare_min(const void *first, const void *second)
{
	return bheaput_compare_min((const char *)first, (const char *)second);
}

static CUTE_PNP_SUITE(bheaput, NULL);

static void
bheaput_setup_empty(void)
{
	memset(bheaput_nodes, 0, sizeof(bheaput_nodes));
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

	cute_ensure(*(int *)bheap_peek_fixed(&bheaput_heap) == 10U);
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

static CUTE_PNP_FIXTURED_SUITE(bheaput_insert, &bheaput,
                               bheaput_setup_empty, NULL);

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

static void bheaput_check_insert(struct bheap_fixed *heap,
                                 const int          *nodes,
                                 int                 nr)
{
	int n;

	for (n = 0; n < nr; n++)
		bheap_insert_fixed(heap, (char *)&nodes[n], bheaput_compare_min);

	bheaput_check_nodes(heap, nr);
}

/**
 * Insert 2 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder2, &bheaput_insert)
{
	int nodes[] = { 1, 2 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder3, &bheaput_insert)
{
	int nodes[] = { 1, 2, 3 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder7, &bheaput_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder8, &bheaput_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder9, &bheaput_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_inorder20, &bheaput_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 2 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder2, &bheaput_insert)
{
	int nodes[] = { 8, 7 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder3, &bheaput_insert)
{
	int nodes[] = { 8, 7, 6 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder7, &bheaput_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder8, &bheaput_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder9, &bheaput_insert)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_revorder20, &bheaput_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 unsorted nodes into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_mixorder3, &bheaput_insert)
{
	int nodes[] = { 8, 6, 7 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 unsorted nodes with duplicates into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_mixorder7, &bheaput_insert)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 unsorted nodes with duplicates into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_mixorder8, &bheaput_insert)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 unsorted nodes with duplicates into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_mixorder9, &bheaput_insert)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 unsorted nodes with duplicates into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_insert_mixorder20, &bheaput_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	bheaput_check_insert(&bheaput_heap, nodes, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(bheaput_extract, &bheaput,
                               bheaput_setup_empty, NULL);

static void bheaput_check_extract(struct bheap_fixed *heap,
                                  const int          *nodes,
                                  int                 nr)
{
	int n;
	int check[nr];

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), bheaput_qsort_compare_min);

	for (n = 0; n < nr; n++)
		bheap_insert_fixed(heap, (char *)&nodes[n], bheaput_compare_min);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		bheaput_check_nodes(heap, nr - n);
		cute_ensure(*(int *)bheap_peek_fixed(heap) == check[n]);

		bheap_extract_fixed(heap, (char *)&curr, bheaput_compare_min);
		cute_ensure(curr == check[n]);
	}
}

/**
 * Extract 2 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder2, &bheaput_extract)
{
	int nodes[] = { 1, 2 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder3, &bheaput_extract)
{
	int nodes[] = { 1, 2, 3 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder7, &bheaput_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder8, &bheaput_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder9, &bheaput_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_inorder20, &bheaput_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 2 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder2, &bheaput_extract)
{
	int nodes[] = { 8, 7 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder3, &bheaput_extract)
{
	int nodes[] = { 8, 7, 6 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder7, &bheaput_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder8, &bheaput_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder9, &bheaput_extract)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in reverse order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_revorder20, &bheaput_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in unsorted order into an empty bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_mixorder3, &bheaput_extract)
{
	int nodes[] = { 8, 6, 7 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes with duplicates inserted in unsorted order into an empty
 * bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_mixorder7, &bheaput_extract)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes with duplicates inserted in unsorted order into an empty
 * bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_mixorder8, &bheaput_extract)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes with duplicates inserted in unsorted order into an empty
 * bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_mixorder9, &bheaput_extract)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * bheap_fixed
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_extract_mixorder20, &bheaput_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	bheaput_check_extract(&bheaput_heap, nodes, array_nr(nodes));
}

static struct bheap_fixed *bheaput_created;

static void
bheaput_create_empty(void)
{
	bheaput_created = bheap_create_fixed(sizeof(int), 20);
	cute_ensure(bheaput_created != NULL);
}

static void
bheaput_destroy_empty(void)
{
	bheap_destroy_fixed(bheaput_created);
}

static CUTE_PNP_FIXTURED_SUITE(bheaput_create, &bheaput,
                               bheaput_create_empty, bheaput_destroy_empty);

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * bheap_fixed created dynamically
 *
 * @ingroup bheaput_fixed
 */
CUTE_PNP_TEST(bheaput_created_mixorder20, &bheaput_create)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	bheaput_check_extract(bheaput_created, nodes, array_nr(nodes));
}

static CUTE_PNP_SUITE(bheaput_build, &bheaput);

static void bheaput_check_build(int *nodes, int nr)
{
	struct bheap_fixed heap;
	int                check[nr];
	int                n;

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), bheaput_qsort_compare_min);

	bheap_init_fixed(&heap, (char *)nodes, sizeof(*nodes), nr);
	bheap_build_fixed(&heap, nr, bheaput_compare_min);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		bheaput_check_nodes(&heap, nr - n);
		cute_ensure(*(int *)bheap_peek_fixed(&heap) == check[n]);

		bheap_extract_fixed(&heap, (char *)&curr, bheaput_compare_min);
		cute_ensure(curr == check[n]);
	}
}

CUTE_PNP_TEST(bheaput_build_single, &bheaput_build)
{
	int nodes[] = { 1 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_inorder2, &bheaput_build)
{
	int nodes[] = { 1, 2 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_revorder2, &bheaput_build)
{
	int nodes[] = { 2, 1 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_inorder3, &bheaput_build)
{
	int nodes[] = { 1, 2, 3 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_revorder3, &bheaput_build)
{
	int nodes[] = { 3, 2, 1 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder3_large_right, &bheaput_build)
{
	int nodes[] = { 2, 1, 3 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder3_large_left, &bheaput_build)
{
	int nodes[] = { 2, 3, 1 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder4, &bheaput_build)
{
	int nodes[] = { 2, 3, 1, 4 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder5, &bheaput_build)
{
	int nodes[] = { 2, 3, 1, 4, 2 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder7, &bheaput_build)
{
	int nodes[] = { 2, 4, 1, 3, 3, 2 };

	bheaput_check_build(nodes, array_nr(nodes));
}

CUTE_PNP_TEST(bheaput_build_mixorder20, &bheaput_build)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	bheaput_check_build(nodes, array_nr(nodes));
}
