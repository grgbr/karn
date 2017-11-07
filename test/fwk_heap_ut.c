/**
 * @file      fwk_heap_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      03 Nov 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array based weak heap unit tests implementation
 *
 * @defgroup fwkhut Fixed length array based weak heap unit tests
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

#include "fwk_heap.h"
#include <cute/cute.h>
#include <stdlib.h>
#include <string.h>

static struct fwk_heap fwkhut_heap;

static int fwkhut_nodes[20];

static void fwkhut_copy(char *restrict dest, const char *restrict src)
{
	*(int *)dest = *(int *)src;
}

static int fwkhut_compare_min(const char *first, const char *second)
{
	return *(int *)first - *(int *)second;
}

static int fwkhut_qsort_compare_min(const void *first, const void *second)
{
	return fwkhut_compare_min((const char *)first, (const char *)second);
}

static CUTE_PNP_SUITE(fwkhut, NULL);

static void
fwkhut_setup_empty(void)
{
	memset(fwkhut_nodes, 0, sizeof(fwkhut_nodes));
	cute_ensure(!fwk_heap_init(&fwkhut_heap, (char *)fwkhut_nodes,
	                           sizeof(fwkhut_nodes[0]),
	                           array_nr(fwkhut_nodes),
	                           fwkhut_compare_min, fwkhut_copy));
}

static CUTE_PNP_FIXTURED_SUITE(fwkhut_empty, &fwkhut,
                               fwkhut_setup_empty, NULL);

/**
 * Check an empty fwk_heap is really exposed as empty and not full
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_check_emptiness, &fwkhut_empty)
{
	cute_ensure(fwk_heap_empty(&fwkhut_heap) == true);
	cute_ensure(fwk_heap_full(&fwkhut_heap) == false);
}

/**
 * Insert a single node into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_single, &fwkhut_empty)
{
	unsigned int node = 10;

	fwk_heap_insert(&fwkhut_heap, (char *)&node);
	cute_ensure(fwk_heap_empty(&fwkhut_heap) == false);
	cute_ensure(fwk_heap_full(&fwkhut_heap) == false);

	cute_ensure(*(int *)fwk_heap_peek(&fwkhut_heap) == 10U);
}

/**
 * Extract a node from a fwk_heap storing a single node
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_single, &fwkhut_empty)
{
	unsigned int inode = 11;
	unsigned int enode = 0;

	fwk_heap_insert(&fwkhut_heap, (char *)&inode);

	cute_ensure(fwk_heap_empty(&fwkhut_heap) == false);
	cute_ensure(fwk_heap_full(&fwkhut_heap) == false);

	fwk_heap_extract(&fwkhut_heap, (char *)&enode);
	cute_ensure(fwk_heap_empty(&fwkhut_heap) == true);
	cute_ensure(fwk_heap_full(&fwkhut_heap) == false);

	cute_ensure(enode == 11U);
}

static CUTE_PNP_FIXTURED_SUITE(fwkhut_insert, &fwkhut,
                               fwkhut_setup_empty, NULL);

static bool fwkhut_isleft_child(const struct fwk_heap *heap, unsigned int index)
{
	return (!!(index & 1)) == fbmp_test(&heap->fwk_rbits, index / 2);
}

static unsigned int fwkhut_dancestor_index(const struct fwk_heap *heap,
                                           unsigned int           index)
{
	/* Move up untill index points to a right child. */
	while (fwkhut_isleft_child(heap, index))
		index /= 2;

	/* Then return its parent. */
	return index / 2;
}

static void fwkhut_check_nodes(const struct fwk_heap *heap,
                                unsigned int            nr)
{
	unsigned int n;

	for (n = 1; n < nr; n++) {
		const int *node = (int *)farr_slot(&heap->fwk_nodes, n);

		if (fwkhut_isleft_child(heap, n)) {
			unsigned int didx;

			didx = fwkhut_dancestor_index(heap, n);
			cute_ensure(*((int *)
			              farr_slot(&heap->fwk_nodes, didx)) <=
			            *node);

		}
		else
			cute_ensure(*((int *)farr_slot(&heap->fwk_nodes,
			                               n / 2)) <= *node);
	}
}

static void fwkhut_check_insert(struct fwk_heap *heap,
                                 const int        *nodes,
                                 int               nr)
{
	int n;

	for (n = 0; n < nr; n++)
		fwk_heap_insert(heap, (char *)&nodes[n]);

	fwkhut_check_nodes(heap, nr);
}

/**
 * Insert 2 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder2, &fwkhut_insert)
{
	int nodes[] = { 1, 2 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder3, &fwkhut_insert)
{
	int nodes[] = { 1, 2, 3 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder7, &fwkhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder8, &fwkhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder9, &fwkhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_inorder20, &fwkhut_insert)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 2 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder2, &fwkhut_insert)
{
	int nodes[] = { 8, 7 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder3, &fwkhut_insert)
{
	int nodes[] = { 8, 7, 6 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder7, &fwkhut_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder8, &fwkhut_insert)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder9, &fwkhut_insert)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 nodes sorted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_revorder20, &fwkhut_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 3 unsorted nodes into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_mixorder3, &fwkhut_insert)
{
	int nodes[] = { 8, 6, 7 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 7 unsorted nodes with duplicates into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_mixorder7, &fwkhut_insert)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 8 unsorted nodes with duplicates into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_mixorder8, &fwkhut_insert)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 9 unsorted nodes with duplicates into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_mixorder9, &fwkhut_insert)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Insert 20 unsorted nodes with duplicates into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_insert_mixorder20, &fwkhut_insert)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fwkhut_check_insert(&fwkhut_heap, nodes, array_nr(nodes));
}

static CUTE_PNP_FIXTURED_SUITE(fwkhut_extract, &fwkhut,
                               fwkhut_setup_empty, NULL);

static void fwkhut_check_extract(struct fwk_heap *heap,
                                  const int        *nodes,
                                  int               nr)
{
	int n;
	int check[nr];

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), fwkhut_qsort_compare_min);

	for (n = 0; n < nr; n++)
		fwk_heap_insert(heap, (char *)&nodes[n]);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		fwkhut_check_nodes(heap, nr - n);
		cute_ensure(*(int *)fwk_heap_peek(heap) == check[n]);

		fwk_heap_extract(heap, (char *)&curr);
		cute_ensure(curr == check[n]);
	}
}

/**
 * Extract 2 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder2, &fwkhut_extract)
{
	int nodes[] = { 1, 2 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder3, &fwkhut_extract)
{
	int nodes[] = { 1, 2, 3 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder7, &fwkhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder8, &fwkhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder9, &fwkhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_inorder20, &fwkhut_extract)
{
	int nodes[] = { 1, 2, 3, 4, 5, 6, 7, 8,
	                9, 10, 11, 12, 13, 14, 15, 16,
	                17, 18, 19, 20 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 2 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder2, &fwkhut_extract)
{
	int nodes[] = { 8, 7 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder3, &fwkhut_extract)
{
	int nodes[] = { 8, 7, 6 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder7, &fwkhut_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder8, &fwkhut_extract)
{
	int nodes[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder9, &fwkhut_extract)
{
	int nodes[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes inserted in reverse order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_revorder20, &fwkhut_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,
	                10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 3 nodes inserted in unsorted order into an empty fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_mixorder3, &fwkhut_extract)
{
	int nodes[] = { 8, 6, 7 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 7 nodes with duplicates inserted in unsorted order into an empty
 * fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_mixorder7, &fwkhut_extract)
{
	int nodes[] = { 2, 5, 7, 1, 6, 3, 2 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 8 nodes with duplicates inserted in unsorted order into an empty
 * fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_mixorder8, &fwkhut_extract)
{
	int nodes[] = { 3, 6, 7, 5, 4, 1, 2, 1 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 9 nodes with duplicates inserted in unsorted order into an empty
 * fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_mixorder9, &fwkhut_extract)
{
	int nodes[] = { 8, 8, 7, 5, 1, 3, 7, 4, 5 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * fwk_heap
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_extract_mixorder20, &fwkhut_extract)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fwkhut_check_extract(&fwkhut_heap, nodes, array_nr(nodes));
}

static struct fwk_heap *fwkhut_created;

static void
fwkhut_create_empty(void)
{
	fwkhut_created = fwk_heap_create(sizeof(int), 20, fwkhut_compare_min,
	                                 fwkhut_copy);
	cute_ensure(fwkhut_created != NULL);
}

static void
fwkhut_destroy_empty(void)
{
	fwk_heap_destroy(fwkhut_created);
}

static CUTE_PNP_FIXTURED_SUITE(fwkhut_create, &fwkhut,
                               fwkhut_create_empty, fwkhut_destroy_empty);

/**
 * Extract 20 nodes with duplicates inserted in unsorted order into an empty
 * fwk_heap created dynamically
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_created_mixorder20, &fwkhut_create)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fwkhut_check_extract(fwkhut_created, nodes, array_nr(nodes));
}

static CUTE_PNP_SUITE(fwkhut_build, &fwkhut);

static void fwkhut_check_build(int *nodes, int nr)
{
	struct fwk_heap heap;
	int              check[nr];
	int              n;

	memcpy(check, nodes, nr * sizeof(*nodes));
	qsort(check, nr, sizeof(*nodes), fwkhut_qsort_compare_min);

	cute_ensure(!fwk_heap_init(&heap, (char *)nodes, sizeof(*nodes), nr,
	                           fwkhut_compare_min, fwkhut_copy));
	fwk_heap_build(&heap, nr);

	for (n = 0; n < nr; n++) {
		int curr = -1;

		fwkhut_check_nodes(&heap, nr - n);
		cute_ensure(*(int *)fwk_heap_peek(&heap) == check[n]);

		fwk_heap_extract(&heap, (char *)&curr);
		cute_ensure(curr == check[n]);
	}
}

/**
 * Build weak heap from external array composed of a sinle node.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_single, &fwkhut_build)
{
	int nodes[] = { 1 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 2 nodes sorted in
 * order.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_inorder2, &fwkhut_build)
{
	int nodes[] = { 1, 2 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 2 nodes sorted in
 * reverse order.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_revorder2, &fwkhut_build)
{
	int nodes[] = { 2, 1 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 3 nodes sorted in
 * order.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_inorder3, &fwkhut_build)
{
	int nodes[] = { 1, 2, 3 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 3 nodes sorted in reverse
 * order.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_revorder3, &fwkhut_build)
{
	int nodes[] = { 3, 2, 1 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 3 nodes in
 * unsorted order with largest node last.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder3_large, &fwkhut_build)
{
	int nodes[] = { 2, 1, 3 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 3 nodes in
 * unsorted order with smallest node last.
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder3_small, &fwkhut_build)
{
	int nodes[] = { 2, 3, 1 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 4 nodes in unsorted order
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder4, &fwkhut_build)
{
	int nodes[] = { 2, 3, 1, 4 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 5 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder5, &fwkhut_build)
{
	int nodes[] = { 2, 3, 1, 4, 2 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 7 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder7, &fwkhut_build)
{
	int nodes[] = { 2, 4, 1, 3, 3, 2 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

/**
 * Build weak heap from external array composed of 20 nodes with duplicates
 * in unsorted order
 *
 * @ingroup fwkhut
 */
CUTE_PNP_TEST(fwkhut_build_mixorder20, &fwkhut_build)
{
	int nodes[] = { 20, 19, 18, 17, 16, 16, 8, 4, 7, 5,
	                1, 3, 2, 4, 10, 11, 12, 13, 19 };

	fwkhut_check_build(nodes, array_nr(nodes));
}

#if defined(CONFIG_FWK_HEAP_SORT)

static CUTE_PNP_SUITE(fwkhut_sort, &fwkhut);

static int fwkhut_sort_compare_min(const char *first, const char *second)
{
	return 0 - fwkhut_compare_min(first, second);
}

static void fwkhut_check_entries(int          *entries,
                                  const int    *checks,
                                  unsigned int  nr)
{
	unsigned int e;

	fwk_heap_sort((char *)entries, sizeof(entries[0]), nr,
	               fwkhut_sort_compare_min, fwkhut_copy);

	for (e = 0; e < nr; e++)
		cute_ensure(entries[e] == checks[e]);
}

CUTE_PNP_TEST(fwkhut_sort_single, &fwkhut_sort)
{
	int entries[] = {
		0
	};
	const int check_entries[] = {
		0
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_inorder2, &fwkhut_sort)
{
	int entries[] = {
		0, 1
	};
	const int check_entries[] = {
		0, 1
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_revorder2, &fwkhut_sort)
{
	int entries[] = {
		1, 0
	};
	const int check_entries[] = {
		0, 1
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_duplicates, &fwkhut_sort)
{
	int entries[] = {
		1, 1
	};
	const int check_entries[] = {
		1, 1
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_presorted, &fwkhut_sort)
{
	int entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_reverse_sorted, &fwkhut_sort)
{
	int entries[] = {
		13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_unsorted, &fwkhut_sort)
{
	int entries[] = {
		2, 12, 13, 0, 1, 3, 10, 9, 8, 11, 4, 6, 5, 7
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(fwkhut_sort_unsorted_duplicates, &fwkhut_sort)
{
	int entries[] = {
		2, 12, 12, 0, 1, 3, 10, 9, 3, 11, 4, 6, 5, 2
	};
	const int check_entries[] = {
		0, 1, 2, 2, 3, 3, 4, 5, 6, 9, 10, 11, 12, 12
	};

	fwkhut_check_entries(entries, check_entries, array_nr(check_entries));
}

#endif /* defined(CONFIG_FWK_HEAP_SORT) */
