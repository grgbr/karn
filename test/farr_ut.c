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

#include "farr.h"
#include <cute/cute.h>

static int farrut_compare_min(const char *first, const char *second)
{
	return *(int *)first - *(int *)second;
}

static void farrut_copy(char *restrict dest, const char *restrict src)
{
	*(int *)dest = *(int *)src;
}

static CUTE_PNP_SUITE(farrut, NULL);

#if defined(CONFIG_FARR_BUBBLE_SORT)

static CUTE_PNP_SUITE(farrut_bubble_sort, &farrut);

static void farrut_check_entries(int          *entries,
                                 const int    *checks,
                                 unsigned int  nr)
{
	unsigned int e;

	farr_bubble_sort((char *)entries, sizeof(entries[0]), nr,
	                 farrut_compare_min, farrut_copy);

	for (e = 0; e < nr; e++)
		cute_ensure(entries[e] == checks[e]);
}

CUTE_PNP_TEST(farrut_sort_single, &farrut_bubble_sort)
{
	int entries[] = {
		0
	};
	const int check_entries[] = {
		0
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_inorder2, &farrut_bubble_sort)
{
	int entries[] = {
		0, 1
	};
	const int check_entries[] = {
		0, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_revorder2, &farrut_bubble_sort)
{
	int entries[] = {
		1, 0
	};
	const int check_entries[] = {
		0, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_duplicates, &farrut_bubble_sort)
{
	int entries[] = {
		1, 1
	};
	const int check_entries[] = {
		1, 1
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_presorted, &farrut_bubble_sort)
{
	int entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_reverse_sorted, &farrut_bubble_sort)
{
	int entries[] = {
		13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_unsorted, &farrut_bubble_sort)
{
	int entries[] = {
		2, 12, 13, 0, 1, 3, 10, 9, 8, 11, 4, 6, 5, 7
	};
	const int check_entries[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

CUTE_PNP_TEST(farrut_sort_unsorted_duplicates, &farrut_bubble_sort)
{
	int entries[] = {
		2, 12, 12, 0, 1, 3, 10, 9, 3, 11, 4, 6, 5, 2
	};
	const int check_entries[] = {
		0, 1, 2, 2, 3, 3, 4, 5, 6, 9, 10, 11, 12, 12
	};

	farrut_check_entries(entries, check_entries, array_nr(check_entries));
}

#endif /* defined(CONFIG_FARR_BUBBLE_SORT) */
