/**
 * @file      fbnr_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      07 Jul 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based binary heap unit tests implementation
 *
 * @defgroup fbnrhut Fixed length array based binary heap unit tests
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

#include "fbnr_heap.h"
#include <cute/cute.h>
#include <stdlib.h>
#include <string.h>

static struct fbnr_heap fbnrhut_heap;

static int fbnrhut_nodes[20];

static void fbnrhut_copy(char *restrict dest, const char *restrict src)
{
	*(int *)dest = *(int *)src;
}

static int fbnrhut_compare_min(const char *first, const char *second)
{
	return *(int *)first - *(int *)second;
}

static int fbnrhut_qsort_compare_min(const void *first, const void *second)
{
	return fbnrhut_compare_min((const char *)first, (const char *)second);
}

static CUTE_PNP_SUITE(fbnrhut, NULL);

static void
fbnrhut_setup_empty(void)
{
	memset(fbnrhut_nodes, 0, sizeof(fbnrhut_nodes));
	fbnr_heap_init(&fbnrhut_heap, (char *)fbnrhut_nodes,
	               sizeof(fbnrhut_nodes[0]), array_nr(fbnrhut_nodes),
	               fbnrhut_compare_min, fbnrhut_copy);
}

static CUTE_PNP_FIXTURED_SUITE(fbnrhut_empty, &fbnrhut,
                               fbnrhut_setup_empty, NULL);

/**
 * Check an empty fbnr_heap is really exposed as empty and not full
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_check_emptiness, &fbnrhut_empty)
{
	cute_ensure(fbnr_heap_empty(&fbnrhut_heap) == true);
	cute_ensure(fbnr_heap_full(&fbnrhut_heap) == false);
}

/**
 * Insert a single node into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_single, &fbnrhut_empty)
{
	unsigned int node = 10;

	fbnr_heap_insert(&fbnrhut_heap, (char *)&node);
	cute_ensure(fbnr_heap_empty(&fbnrhut_heap) == false);
	cute_ensure(fbnr_heap_full(&fbnrhut_heap) == false);

	cute_ensure(*(int *)fbnr_heap_peek(&fbnrhut_heap) == 10U);
}

/**
 * Extract a node from a fbnr_heap storing a single node
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_single, &fbnrhut_empty)
{
	unsigned int inode = 11;
	unsigned int enode = 0;

	fbnr_heap_insert(&fbnrhut_heap, (char *)&inode);

	cute_ensure(fbnr_heap_empty(&fbnrhut_heap) == false);
	cute_ensure(fbnr_heap_full(&fbnrhut_heap) == false);

	fbnr_heap_extract(&fbnrhut_heap, (char *)&enode);
	cute_ensure(fbnr_heap_empty(&fbnrhut_heap) == true);
	cute_ensure(fbnr_heap_full(&fbnrhut_heap) == false);

	cute_ensure(enode == 11U);
}

static CUTE_PNP_FIXTURED_SUITE(fbnrhut_insert, &fbnrhut,
                               fbnrhut_setup_empty, NULL);

static void fbnrhut_check_nodes(const struct fbnr_heap *heap,
                                unsigned int            nr)
{
	unsigned int n;

	for (n = 1; n < nr; n++) {
#if 0
		const int *node = (int *)
		                  array_fixed_item(&heap->fbnr_tree.bst_nodes,
		                                   n);
		cute_ensure(*((int *)
		              bstree_fixed_parent(&heap->fbnr_tree,
		                                  (char *)node)) <= *node);
#endif
		const int *node = (int *)fabs_tree_node(&heap->fbnr_tree, n);

		cute_ensure(*((int *)
		              fabs_tree_node(&heap->fbnr_tree,
		                             fabs_tree_parent_index(n))) <=
		            *node);
	}
}

static void fbnrhut_check_insert(struct fbnr_heap *heap,
                                 const int        *nodes,
                                 int               nr)
{
	int n;

	for (n = 0; n < nr; n++)
		fbnr_heap_insert(heap, (char *)&nodes[n]);

	fbnrhut_check_nodes(heap, nr);
}

/**
 * Insert 2 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder2, &fbnrhut_insert)
{
	int nodes[] = { 1, 2 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder3, &fbnrhut_insert)
{
	int nodes[] = { 1, 2, 3 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder7, &fbnrhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder8, &fbnrhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder9, &fbnrhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_inorder20, &fbnrhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 2 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder2, &fbnrhut_insert)
{
	int nodes[] = { 8, 7 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder3, &fbnrhut_insert)
{
	int nodes[] = { 8, 7, 6 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder7, &fbnrhut_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder8, &fbnrhut_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder9, &fbnrhut_insert)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_revorder20, &fbnrhut_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 unsorted nodes into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_mixorder3, &fbnrhut_insert)
{
	int nodes[] = { 8, 6, 7 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 unsorted nodes with duplicates into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_mixorder7, &fbnrhut_insert)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 unsorted nodes with duplicates into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_mixorder8, &fbnrhut_insert)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 unsorted nodes with duplicates into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_mixorder9, &fbnrhut_insert)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 unsorted nodes with duplicates into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_insert_mixorder20, &fbnrhut_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fbnrhut_check_insert(&fbnrhut_heap, nodes, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(fbnrhut_extract, &fbnrhut,
                               fbnrhut_setup_empty, NULL);

static void fbnrhut_check_extract(struct fbnr_heap *heap,
                                  const int        *nodes,
                                  int               nr)
{
	int n;
	int check[nr];

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), fbnrhut_qsort_compare_min);

	for (n = 0; n < nr; n++)
		fbnr_heap_insert(heap, (char *)&nodes[n]);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		fbnrhut_check_nodes(heap, nr - n);
		cute_ensure(*(int *)fbnr_heap_peek(heap) == check[n]);

		fbnr_heap_extract(heap, (char *)&curr);
		cute_ensure(curr == check[n]);
	}
}

/**
 * Extract 2 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder2, &fbnrhut_extract)
{
	int nodes[] = { 1, 2 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder3, &fbnrhut_extract)
{
	int nodes[] = { 1, 2, 3 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder7, &fbnrhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder8, &fbnrhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder9, &fbnrhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_inorder20, &fbnrhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 2 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder2, &fbnrhut_extract)
{
	int nodes[] = { 8, 7 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder3, &fbnrhut_extract)
{
	int nodes[] = { 8, 7, 6 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder7, &fbnrhut_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder8, &fbnrhut_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder9, &fbnrhut_extract)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in reverse order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_revorder20, &fbnrhut_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in unsorted order into an empty fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_mixorder3, &fbnrhut_extract)
{
	int nodes[] = { 8, 6, 7 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes with duplicates inserted in unsorted order into an empty
 * fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_mixorder7, &fbnrhut_extract)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes with duplicates inserted in unsorted order into an empty
 * fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_mixorder8, &fbnrhut_extract)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes with duplicates inserted in unsorted order into an empty
 * fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_mixorder9, &fbnrhut_extract)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * fbnr_heap
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_extract_mixorder20, &fbnrhut_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fbnrhut_check_extract(&fbnrhut_heap, nodes, array_nr(nodes));
}

static struct fbnr_heap *fbnrhut_created;

static void
fbnrhut_create_empty(void)
{
	fbnrhut_created = fbnr_heap_create(sizeof(int), 20, fbnrhut_compare_min,
	                                   fbnrhut_copy);
	cute_ensure(fbnrhut_created != NULL);
}

static void
fbnrhut_destroy_empty(void)
{
	fbnr_heap_destroy(fbnrhut_created);
}

static CUTE_PNP_FIXTURED_SUITE(fbnrhut_create, &fbnrhut,
                               fbnrhut_create_empty, fbnrhut_destroy_empty);

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * fbnr_heap created dynamically
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_created_mixorder20, &fbnrhut_create)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fbnrhut_check_extract(fbnrhut_created, nodes, array_nr(nodes));
}

static CUTE_PNP_SUITE(fbnrhut_build, &fbnrhut);

static void fbnrhut_check_build(int *nodes, int nr)
{
	struct fbnr_heap heap;
	int              check[nr];
	int              n;

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), fbnrhut_qsort_compare_min);

	fbnr_heap_init(&heap, (char *)nodes, sizeof(*nodes), nr,
	               fbnrhut_compare_min, fbnrhut_copy);
	fbnr_heap_build(&heap, nr);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		fbnrhut_check_nodes(&heap, nr - n);
		cute_ensure(*(int *)fbnr_heap_peek(&heap) == check[n]);

		fbnr_heap_extract(&heap, (char *)&curr);
		cute_ensure(curr == check[n]);
	}
}

/**
 * Build binary heap from external array composed of a sinle node.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_single, &fbnrhut_build)
{
	int nodes[] = { 1 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 2 nodes sorted in
 * order.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_inorder2, &fbnrhut_build)
{
	int nodes[] = { 1, 2 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 2 nodes sorted in
 * reverse order.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_revorder2, &fbnrhut_build)
{
	int nodes[] = { 2, 1 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 3 nodes sorted in
 * order.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_inorder3, &fbnrhut_build)
{
	int nodes[] = { 1, 2, 3 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 3 nodes sorted in reverse
 * order.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_revorder3, &fbnrhut_build)
{
	int nodes[] = { 3, 2, 1 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 3 nodes in
 * unsorted order with largest node last.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder3_large, &fbnrhut_build)
{
	int nodes[] = { 2, 1, 3 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 3 nodes in
 * unsorted order with smallest node last.
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder3_small, &fbnrhut_build)
{
	int nodes[] = { 2, 3, 1 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 4 nodes in unsorted order
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder4, &fbnrhut_build)
{
	int nodes[] = { 2, 3, 1, 4 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 5 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder5, &fbnrhut_build)
{
	int nodes[] = { 2, 3, 1, 4, 2 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 7 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder7, &fbnrhut_build)
{
	int nodes[] = { 2, 4, 1, 3, 3, 2 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build binary heap from external array composed of 20 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fbnrhut
 */
CUTE_PNP_TEST(fbnrhut_build_mixorder20, &fbnrhut_build)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fbnrhut_check_build(nodes, array_nr(nodes));
}

#if defined(CONFIG_FBNR_HEAP_SORT)

static CUTE_PNP_SUITE(fbnrhut_sort, &fbnrhut);

static int fbnrhut_sort_compare_min(const char *first, const char *second)
{
	return 0 - fbnrhut_compare_min(first, second);
}

static void fbnrhut_check_entries(int          *entries,
                                  const int    *checks,
                                  unsigned int  nr)
{
	unsigned int e;

	fbnr_heap_sort((char *)entries, sizeof(entries[0]), nr,
	               fbnrhut_sort_compare_min, fbnrhut_copy);

	for (e = 0; e < nr; e++)
		cute_ensure(entries[e] == checks[e]);
}

CUTE_PNP_TEST(fbnrhut_sort_single, &fbnrhut_sort)
{
	int entries[] = {
		0
	};
	const int check_entries[] = {
		0
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_inorder2, &fbnrhut_sort)
{
	int entries[] = {
		0, 1
	};
	const int check_entries[] = {
		0, 1
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_revorder2, &fbnrhut_sort)
{
	int entries[] = {
		1, 0
	};
	const int check_entries[] = {
		0, 1
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_duplicates, &fbnrhut_sort)
{
	int entries[] = {
		1, 1
	};
	const int check_entries[] = {
		1, 1
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_presorted, &fbnrhut_sort)
{
	int entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_reverse_sorted, &fbnrhut_sort)
{
	int entries[] = {
		13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_unsorted, &fbnrhut_sort)
{
	int entries[] = {
		2, 12, 13, 0, 1, 3, 10, 9, 8, 11, 4, 6, 5, 7
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fbnrhut_sort_unsorted_duplicates, &fbnrhut_sort)
{
	int entries[] = {
		2, 12, 12, 0, 1, 3, 10, 9, 3, 11, 4, 6, 5, 2
	};
	const int check_entries[] = {
		0, 1, 2, 2, 3, 3, 4, 5, 6, 9, 10, 11, 12, 12
	};

	fbnrhut_check_entries(entries, check_entries, array_nr(check_entries));
}

#endif /* defined(CONFIG_FBNR_HEAP_SORT) */
