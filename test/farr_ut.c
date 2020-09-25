/**
 * @file      farr_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      25 Oct 2017
 * @copyright GNU Public License v3
 *
 * Fixed length array unit tests implementation
 *
 * @defgroup farrut Fixed length array unit tests
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

#include <karn/farr.h>
#include <cute/cute.h>

static int farrut_compare_min(const char *first, const char *second)
{
	return *(int *)first - *(int *)second;
}

static void farrut_copy(char *restrict dest, const char *restrict src)
{
	*(int *)dest = *(int *)src;
}

typedef void (farrut_sort_fn)(char *,
                              size_t,
                              unsigned int,
                              farr_compare_fn *,
                              farr_copy_fn *);

static void farrut_check_entries(int            *entries,
                                 const int      *checks,
                                 unsigned int    nr,
                                 farrut_sort_fn *sort)
{
	unsigned int e;

	sort((char *)entries, sizeof(entries[0]), nr, farrut_compare_min,
	     farrut_copy);

	for (e = 0; e < nr; e++)
		cute_ensure(entries[e] == checks[e]);
}

static void farrut_sort_single(farrut_sort_fn *sort)
{
	int entries[] = {
		0
	};
	const int check_entries[] = {
		0
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_inorder2(farrut_sort_fn *sort)
{
	int entries[] = {
		0, 1
	};
	const int check_entries[] = {
		0, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_revorder2(farrut_sort_fn *sort)
{
	int entries[] = {
		1, 0
	};
	const int check_entries[] = {
		0, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_duplicates(farrut_sort_fn *sort)
{
	int entries[] = {
		1, 1
	};
	const int check_entries[] = {
		1, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_presorted(farrut_sort_fn *sort)
{
	int entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_reverse_sorted(farrut_sort_fn *sort)
{
	int entries[] = {
		13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_unsorted(farrut_sort_fn *sort)
{
	int entries[] = {
		2, 12, 13, 0, 1, 3, 10, 9, 8, 11, 4, 6, 5, 7
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static void farrut_sort_unsorted_duplicates(farrut_sort_fn *sort)
{
	int entries[] = {
		2, 12, 12, 0, 1, 3, 10, 9, 3, 11, 4, 6, 5, 2
	};
	const int check_entries[] = {
		0, 1, 2, 2, 3, 3, 4, 5, 6, 9, 10, 11, 12, 12
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries),
	                     sort);
}

static CUTE_PNP_SUITE(farrut, NULL);

#if defined(CONFIG_KARN_FARR_BUBBLE_SORT)

static CUTE_PNP_SUITE(farrut_bubble_sort, &farrut);

CUTE_PNP_TEST(farrut_bubble_sort_single, &farrut_bubble_sort)
{
	farrut_sort_single(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_inorder2, &farrut_bubble_sort)
{
	farrut_sort_inorder2(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_revorder2, &farrut_bubble_sort)
{
	farrut_sort_revorder2(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_duplicates, &farrut_bubble_sort)
{
	farrut_sort_duplicates(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_presorted, &farrut_bubble_sort)
{
	farrut_sort_presorted(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_reverse_sorted, &farrut_bubble_sort)
{
	farrut_sort_reverse_sorted(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_unsorted, &farrut_bubble_sort)
{
	farrut_sort_unsorted(farr_bubble_sort);
}

CUTE_PNP_TEST(farrut_bubble_sort_unsorted_duplicates, &farrut_bubble_sort)
{
	farrut_sort_unsorted_duplicates(farr_bubble_sort);
}

#endif /* defined(CONFIG_KARN_FARR_BUBBLE_SORT) */

#if defined(CONFIG_KARN_FARR_SELECTION_SORT)

static CUTE_PNP_SUITE(farrut_selection_sort, &farrut);

CUTE_PNP_TEST(farrut_selection_sort_single, &farrut_selection_sort)
{
	farrut_sort_single(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_inorder2, &farrut_selection_sort)
{
	farrut_sort_inorder2(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_revorder2, &farrut_selection_sort)
{
	farrut_sort_revorder2(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_duplicates, &farrut_selection_sort)
{
	farrut_sort_duplicates(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_presorted, &farrut_selection_sort)
{
	farrut_sort_presorted(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_reverse_sorted, &farrut_selection_sort)
{
	farrut_sort_reverse_sorted(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_unsorted, &farrut_selection_sort)
{
	farrut_sort_unsorted(farr_selection_sort);
}

CUTE_PNP_TEST(farrut_selection_sort_unsorted_duplicates, &farrut_selection_sort)
{
	farrut_sort_unsorted_duplicates(farr_selection_sort);
}

#endif /* defined(CONFIG_KARN_FARR_SELECTION_SORT) */

#if defined(CONFIG_KARN_FARR_INSERTION_SORT)

static CUTE_PNP_SUITE(farrut_insertion_sort, &farrut);

CUTE_PNP_TEST(farrut_insertion_sort_single, &farrut_insertion_sort)
{
	farrut_sort_single(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_inorder2, &farrut_insertion_sort)
{
	farrut_sort_inorder2(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_revorder2, &farrut_insertion_sort)
{
	farrut_sort_revorder2(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_duplicates, &farrut_insertion_sort)
{
	farrut_sort_duplicates(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_presorted, &farrut_insertion_sort)
{
	farrut_sort_presorted(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_reverse_sorted, &farrut_insertion_sort)
{
	farrut_sort_reverse_sorted(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_unsorted, &farrut_insertion_sort)
{
	farrut_sort_unsorted(farr_insertion_sort);
}

CUTE_PNP_TEST(farrut_insertion_sort_unsorted_duplicates, &farrut_insertion_sort)
{
	farrut_sort_unsorted_duplicates(farr_insertion_sort);
}

#endif /* defined(CONFIG_KARN_FARR_INSERTION_SORT) */

#if defined(CONFIG_KARN_FARR_QUICK_SORT)

static CUTE_PNP_SUITE(farrut_quick_sort, &farrut);

CUTE_PNP_TEST(farrut_quick_sort_single, &farrut_quick_sort)
{
	farrut_sort_single(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_inorder2, &farrut_quick_sort)
{
	farrut_sort_inorder2(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_revorder2, &farrut_quick_sort)
{
	farrut_sort_revorder2(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_duplicates, &farrut_quick_sort)
{
	farrut_sort_duplicates(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_presorted, &farrut_quick_sort)
{
	farrut_sort_presorted(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_reverse_sorted, &farrut_quick_sort)
{
	farrut_sort_reverse_sorted(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_unsorted, &farrut_quick_sort)
{
	farrut_sort_unsorted(farr_quick_sort);
}

CUTE_PNP_TEST(farrut_quick_sort_unsorted_duplicates, &farrut_quick_sort)
{
	farrut_sort_unsorted_duplicates(farr_quick_sort);
}

#endif /* defined(CONFIG_KARN_FARR_QUICK_SORT) */
