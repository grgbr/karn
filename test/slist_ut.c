/**
 * @file      slist_ut.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      08 Jul 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list unit tests implementation
 *
 * @defgroup slistut Singly linked list unit tests
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

#include "slist.h"
#include "array.h"
#include <cute/cute.h>

struct slistut_entry {
	struct slist_node node;
	unsigned int      value;
};

#define SLISTUT_INIT_ENTRY(_value) { .value = _value }

typedef void             (slistut_sort_fn)(struct slist     *restrict,
                                           slist_compare_fn *);
static struct slist       slistut_list;
static struct slist_node  slistut_nodes[4];
static unsigned int       slistut_nr;
static slistut_sort_fn   *slistut_sort;
static unsigned int       slistut_merge_run_len;

static void
slistut_init_entries(struct slistut_entry entries[], unsigned int count)
{
	unsigned int c;

	slist_init(&slistut_list);

	for (c = 0; c < count; c++)
		slist_nqueue(&slistut_list, &entries[c].node);

	slistut_nr = count;
}

static int
slistut_compare_entries(const struct slist_node *a, const struct slist_node *b)
{
	return slist_entry(a, struct slistut_entry, node)->value -
	       slist_entry(b, struct slistut_entry, node)->value;
}

static void
slistut_hybrid_merge_sort(struct slist *list, slist_compare_fn *compare)
{
	slist_hybrid_merge_sort(list, slistut_merge_run_len, slistut_nr,
	                        compare);
}

static void
slistut_check_nodes(const struct slist_node *const check_nodes[],
                    unsigned int                   count)
{
	struct slist_node *node;
	unsigned int       cnt = 0;

	slist_foreach_node(&slistut_list, node) {
		cute_ensure(cnt <= count);

		cute_ensure(node == check_nodes[cnt]);
		cnt++;
	}

	cute_ensure(slist_empty(&slistut_list) == false);
	cute_ensure(cnt == count);
	cute_ensure(slist_first(&slistut_list) == check_nodes[0]);
	cute_ensure(slist_last(&slistut_list) == check_nodes[count - 1]);
}

static void slistut_setup_bubble_sort(void)
{
	slistut_sort = slist_bubble_sort;
}

static void slistut_setup_insertion_sort(void)
{
	slistut_sort = slist_insertion_sort;
}

static void slistut_setup_selection_sort(void)
{
	slistut_sort = slist_selection_sort;
}

static void slistut_domerge_sort(struct slist *list, slist_compare_fn *compare)
{
	slist_merge_sort(list, slistut_nr, compare);
}

static void slistut_setup_merge_sort(void)
{
	slistut_sort = slistut_domerge_sort;
}

static void slistut_setup_runlen_merge_sort(void)
{
	slistut_sort = slistut_hybrid_merge_sort;
}

static void slistut_sort_single(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(0)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[0].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_double(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(2),
		SLISTUT_INIT_ENTRY(0)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[1].node,
		&entries[0].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_presorted(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(5),
		SLISTUT_INIT_ENTRY(6),
		SLISTUT_INIT_ENTRY(7),
		SLISTUT_INIT_ENTRY(8),
		SLISTUT_INIT_ENTRY(9)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[0].node,
		&entries[1].node,
		&entries[2].node,
		&entries[3].node,
		&entries[4].node,
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_unsorted(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(2),
		SLISTUT_INIT_ENTRY(4),
		SLISTUT_INIT_ENTRY(6),
		SLISTUT_INIT_ENTRY(5),
		SLISTUT_INIT_ENTRY(9),
		SLISTUT_INIT_ENTRY(7),
		SLISTUT_INIT_ENTRY(8),
		SLISTUT_INIT_ENTRY(1),
		SLISTUT_INIT_ENTRY(0)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[9].node,
		&entries[8].node,
		&entries[1].node,
		&entries[0].node,
		&entries[2].node,
		&entries[4].node,
		&entries[3].node,
		&entries[6].node,
		&entries[7].node,
		&entries[5].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_reverse_sorted(void)
{
	struct slistut_entry entries[] = {
		[0] = SLISTUT_INIT_ENTRY(9),
		[1] = SLISTUT_INIT_ENTRY(8),
		[2] = SLISTUT_INIT_ENTRY(7),
		[3] = SLISTUT_INIT_ENTRY(6),
		[4] = SLISTUT_INIT_ENTRY(5),
		[5] = SLISTUT_INIT_ENTRY(4),
		[6] = SLISTUT_INIT_ENTRY(3),
		[7] = SLISTUT_INIT_ENTRY(2),
		[8] = SLISTUT_INIT_ENTRY(1),
		[9] = SLISTUT_INIT_ENTRY(0)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[9].node,
		&entries[8].node,
		&entries[7].node,
		&entries[6].node,
		&entries[5].node,
		&entries[4].node,
		&entries[3].node,
		&entries[2].node,
		&entries[1].node,
		&entries[0].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_double_duplicates(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(3)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[0].node,
		&entries[1].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_duplicates(void)
{
	struct slistut_entry entries[] = {
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(3),
		SLISTUT_INIT_ENTRY(3)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[0].node,
		&entries[1].node,
		&entries[2].node,
		&entries[3].node,
		&entries[4].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_stable_unsorted(void)
{
	struct slistut_entry entries[] = {
		[0]  = SLISTUT_INIT_ENTRY(3),
		[1]  = SLISTUT_INIT_ENTRY(2),
		[2]  = SLISTUT_INIT_ENTRY(4),
		[3]  = SLISTUT_INIT_ENTRY(3),
		[4]  = SLISTUT_INIT_ENTRY(6),
		[5]  = SLISTUT_INIT_ENTRY(5),
		[6]  = SLISTUT_INIT_ENTRY(9),
		[7]  = SLISTUT_INIT_ENTRY(9),
		[8]  = SLISTUT_INIT_ENTRY(7),
		[9]  = SLISTUT_INIT_ENTRY(8),
		[10] = SLISTUT_INIT_ENTRY(1),
		[11] = SLISTUT_INIT_ENTRY(0),
		[12] = SLISTUT_INIT_ENTRY(3)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[11].node,
		&entries[10].node,
		&entries[1].node,
		&entries[0].node,
		&entries[3].node,
		&entries[12].node,
		&entries[2].node,
		&entries[5].node,
		&entries[4].node,
		&entries[8].node,
		&entries[9].node,
		&entries[6].node,
		&entries[7].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_worstins_sorted(void)
{
	struct slistut_entry entries[] = {
		[0] = SLISTUT_INIT_ENTRY(9),
		[1] = SLISTUT_INIT_ENTRY(0),
		[2] = SLISTUT_INIT_ENTRY(1),
		[3] = SLISTUT_INIT_ENTRY(2),
		[4] = SLISTUT_INIT_ENTRY(3),
		[5] = SLISTUT_INIT_ENTRY(4),
		[6] = SLISTUT_INIT_ENTRY(5),
		[7] = SLISTUT_INIT_ENTRY(6),
		[8] = SLISTUT_INIT_ENTRY(7),
		[9] = SLISTUT_INIT_ENTRY(8)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[1].node,
		&entries[2].node,
		&entries[3].node,
		&entries[4].node,
		&entries[5].node,
		&entries[6].node,
		&entries[7].node,
		&entries[8].node,
		&entries[9].node,
		&entries[0].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_dosort_large_mix(void)
{
	struct slistut_entry entries[] = {
		[0] = SLISTUT_INIT_ENTRY(9),
		[1] = SLISTUT_INIT_ENTRY(0),
		[2] = SLISTUT_INIT_ENTRY(1),
		[3] = SLISTUT_INIT_ENTRY(2),
		[4] = SLISTUT_INIT_ENTRY(3),
		[5] = SLISTUT_INIT_ENTRY(4),
		[6] = SLISTUT_INIT_ENTRY(5),
		[7] = SLISTUT_INIT_ENTRY(6),
		[8] = SLISTUT_INIT_ENTRY(7),
		[9] = SLISTUT_INIT_ENTRY(8),

		[10] = SLISTUT_INIT_ENTRY(29),
		[11] = SLISTUT_INIT_ENTRY(20),
		[12] = SLISTUT_INIT_ENTRY(21),
		[13] = SLISTUT_INIT_ENTRY(22),
		[14] = SLISTUT_INIT_ENTRY(23),
		[15] = SLISTUT_INIT_ENTRY(24),
		[16] = SLISTUT_INIT_ENTRY(25),
		[17] = SLISTUT_INIT_ENTRY(26),
		[18] = SLISTUT_INIT_ENTRY(27),
		[19] = SLISTUT_INIT_ENTRY(28),

		[20] = SLISTUT_INIT_ENTRY(11),
		[21] = SLISTUT_INIT_ENTRY(11),
		[22] = SLISTUT_INIT_ENTRY(11),
		[23] = SLISTUT_INIT_ENTRY(11),
		[24] = SLISTUT_INIT_ENTRY(11),
		[25] = SLISTUT_INIT_ENTRY(11),
		[26] = SLISTUT_INIT_ENTRY(11),
		[27] = SLISTUT_INIT_ENTRY(11),
		[28] = SLISTUT_INIT_ENTRY(11),
		[29] = SLISTUT_INIT_ENTRY(11),

		[30] = SLISTUT_INIT_ENTRY(9),
		[31] = SLISTUT_INIT_ENTRY(0),
		[32] = SLISTUT_INIT_ENTRY(1)
	};
	const struct slist_node *const check_nodes[] = {
		&entries[1].node,
		&entries[31].node,
		&entries[2].node,
		&entries[32].node,
		&entries[3].node,
		&entries[4].node,
		&entries[5].node,
		&entries[6].node,
		&entries[7].node,
		&entries[8].node,

		&entries[9].node,
		&entries[0].node,
		&entries[30].node,
		&entries[20].node,
		&entries[21].node,
		&entries[22].node,
		&entries[23].node,
		&entries[24].node,
		&entries[25].node,
		&entries[26].node,

		&entries[27].node,
		&entries[28].node,
		&entries[29].node,
		&entries[11].node,
		&entries[12].node,
		&entries[13].node,
		&entries[14].node,
		&entries[15].node,
		&entries[16].node,
		&entries[17].node,

		&entries[18].node,
		&entries[19].node,
		&entries[10].node
	};

	slistut_init_entries(entries, array_nr(entries));

	slistut_sort(&slistut_list, slistut_compare_entries);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static void slistut_sort_large_mix(void)
{
	slistut_dosort_large_mix();
}

static CUTE_PNP_SUITE(slistut, NULL);

static void slistut_setup_empty(void)
{
	slist_init(&slistut_list);
}

static CUTE_PNP_FIXTURED_SUITE(slistut_empty, &slistut,
                               slistut_setup_empty, NULL);

/**
 * Check an empty slist is really exposed as empty.
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_isempty, &slistut_empty)
{
	cute_ensure(slist_empty(&slistut_list) == true);
}

/**
 * Check iteration over an empty slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_iterate_empty, &slistut_empty)
{
	struct slist_node *node;
	unsigned int       cnt = 0;

	slist_foreach_node(&slistut_list, node)
		cute_fail("empty slist cannot be iterated over");

	cute_ensure(cnt == 0);
}

/**
 * Check enqueueing into an empty slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_nqueue_empty, &slistut_empty)
{
	struct slist_node  entry;
	struct slist_node *node;
	unsigned int       cnt = 0;

	slist_nqueue(&slistut_list, &entry);

	slist_foreach_node(&slistut_list, node)
		cnt++;

	cute_ensure(slist_empty(&slistut_list) == false);
	cute_ensure(cnt == 1);
	cute_ensure(slist_first(&slistut_list) == &entry);
	cute_ensure(slist_last(&slistut_list) == &entry);
}

/**
 * Check append after an empty slist's head
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_empty, &slistut_empty)
{
	struct slist_node  entry;
	struct slist_node *node;
	unsigned int       cnt = 0;

	slist_append(&slistut_list, slist_head(&slistut_list), &entry);

	slist_foreach_node(&slistut_list, node)
		cnt++;

	cute_ensure(slist_empty(&slistut_list) == false);
	cute_ensure(cnt == 1);
	cute_ensure(slist_first(&slistut_list) == &entry);
	cute_ensure(slist_last(&slistut_list) == &entry);
}

static void
slistut_setup_basic(void)
{
	unsigned int n;

	slist_init(&slistut_list);

	for (n = 0; n < array_nr(slistut_nodes); n++)
		slist_nqueue(&slistut_list, &slistut_nodes[n]);
}

static CUTE_PNP_FIXTURED_SUITE(slistut_append, &slistut,
                               slistut_setup_basic, NULL);

/**
 * Append node right after an slist's head
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_head, &slistut_append)
{
	struct slist_node              entry;
	const struct slist_node *const check_nodes[] = {
		&entry,
		&slistut_nodes[0],
		&slistut_nodes[1],
		&slistut_nodes[2],
		&slistut_nodes[3]
	};

	slist_append(&slistut_list, slist_head(&slistut_list), &entry);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Append node right after an slist's first node
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_first, &slistut_append)
{
	struct slist_node              entry;
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&entry,
		&slistut_nodes[1],
		&slistut_nodes[2],
		&slistut_nodes[3]
	};

	slist_append(&slistut_list, slist_first(&slistut_list), &entry);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Append node right after an slist's last node
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_last, &slistut_append)
{
	struct slist_node              entry;
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&slistut_nodes[1],
		&slistut_nodes[2],
		&slistut_nodes[3],
		&entry
	};

	slist_append(&slistut_list, slist_last(&slistut_list), &entry);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Append node right after an slist's penultimate node
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_penultimate, &slistut_append)
{
	struct slist_node              entry;
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&slistut_nodes[1],
		&slistut_nodes[2],
		&entry,
		&slistut_nodes[3]
	};

		
	slist_append(&slistut_list, &slistut_nodes[2], &entry);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Append node somewhere in the middle of an slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_append_middle, &slistut_append)
{
	struct slist_node              entry;
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&slistut_nodes[1],
		&entry,
		&slistut_nodes[2],
		&slistut_nodes[3]
	};

		
	slist_append(&slistut_list, &slistut_nodes[1], &entry);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static CUTE_PNP_FIXTURED_SUITE(slistut_delete, &slistut,
                               slistut_setup_basic, NULL);

/**
 * Remove first node of an slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_delete_first, &slistut_delete)
{
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[1],
		&slistut_nodes[2],
		&slistut_nodes[3]
	};

	slist_delete(&slistut_list, slist_head(&slistut_list),
	             slist_first(&slistut_list));

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Remove last node of an slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_delete_last, &slistut_delete)
{
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&slistut_nodes[1],
		&slistut_nodes[2],
	};

	slist_delete(&slistut_list, &slistut_nodes[2],
	             slist_last(&slistut_list));

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

/**
 * Remove penultimate node of an slist
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_delete_penultimate, &slistut_delete)
{
	const struct slist_node *const check_nodes[] = {
		&slistut_nodes[0],
		&slistut_nodes[1],
		&slistut_nodes[3],
	};

	slist_delete(&slistut_list, &slistut_nodes[1], &slistut_nodes[2]);

	slistut_check_nodes(check_nodes, array_nr(check_nodes));
}

static CUTE_PNP_FIXTURED_SUITE(slistut_bubble_sort, &slistut,
                               slistut_setup_bubble_sort, NULL);

/**
 * Check bubble sort operation upon an slist containing a single items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_single, &slistut_bubble_sort)
{
	slistut_sort_single();
}

/**
 * Check bubble sort operation upon an slist containing 2 items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_double, &slistut_bubble_sort)
{
	slistut_sort_double();
}

/**
 * Check bubble sort operation upon an slist containing presorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_presorted, &slistut_bubble_sort)
{
	slistut_sort_presorted();
}

/**
 * Check bubble sort operation upon an slist containing unsorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_unsorted, &slistut_bubble_sort)
{
	slistut_sort_unsorted();
}

/**
 * Check bubble sort operation upon an slist containing items presorted in
 * reverse order
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_reverse_sorted, &slistut_bubble_sort)
{
	slistut_sort_reverse_sorted();
}

/**
 * Check bubble sort operation upon an slist containing 2 duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_double_duplicates, &slistut_bubble_sort)
{
	slistut_sort_double_duplicates();
}

/**
 * Check bubble sort operation upon an slist containing duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_duplicates, &slistut_bubble_sort)
{
	slistut_sort_duplicates();
}

/**
 * Check stability of a bubble sort operation upon slist containing duplicate
 * items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_stable_unsorted, &slistut_bubble_sort)
{
	slistut_sort_stable_unsorted();
}

/**
 * Check a bubble sort operation upon slist containing items presorted according
 * to insertion sort worst case
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_worstins_sorted, &slistut_bubble_sort)
{
	slistut_sort_worstins_sorted();
}

/**
 * Check a bubble sort operation upon slist containing large number of items
 * with duplicates
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_bubble_sort_large_mix, &slistut_bubble_sort)
{
	slistut_sort_large_mix();
}

static CUTE_PNP_FIXTURED_SUITE(slistut_selection_sort, &slistut,
                               slistut_setup_selection_sort, NULL);

/**
 * Check selection sort operation upon an slist containing a single items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_single, &slistut_selection_sort)
{
	slistut_sort_single();
}

/**
 * Check selection sort operation upon an slist containing 2 items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_double, &slistut_selection_sort)
{
	slistut_sort_double();
}

/**
 * Check selection sort operation upon an slist containing presorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_presorted, &slistut_selection_sort)
{
	slistut_sort_presorted();
}

/**
 * Check selection sort operation upon an slist containing unsorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_unsorted, &slistut_selection_sort)
{
	slistut_sort_unsorted();
}

/**
 * Check selection sort operation upon an slist containing items presorted in
 * reverse order
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_reverse_sorted, &slistut_selection_sort)
{
	slistut_sort_reverse_sorted();
}

/**
 * Check selection sort operation upon an slist containing 2 duplicate items
 * only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_double_duplicates, &slistut_selection_sort)
{
	slistut_sort_double_duplicates();
}

/**
 * Check selection sort operation upon an slist containing duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_duplicates, &slistut_selection_sort)
{
	slistut_sort_duplicates();
}

/**
 * Check stability of a selection sort operation upon slist containing duplicate
 * items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_stable_unsorted, &slistut_selection_sort)
{
	slistut_sort_stable_unsorted();
}

/**
 * Check a selection sort operation upon slist containing items presorted
 * according to insertion sort worst case
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_worstins_sorted, &slistut_selection_sort)
{
	slistut_sort_worstins_sorted();
}

/**
 * Check a selection sort operation upon slist containing large number of items
 * with duplicates
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_selection_sort_large_mix, &slistut_selection_sort)
{
	slistut_sort_large_mix();
}

static CUTE_PNP_FIXTURED_SUITE(slistut_insertion_sort, &slistut,
                               slistut_setup_insertion_sort, NULL);

/**
 * Check insertion sort operation upon an slist containing a single items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_single, &slistut_insertion_sort)
{
	slistut_sort_single();
}

/**
 * Check insertion sort operation upon an slist containing 2 items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_double, &slistut_insertion_sort)
{
	slistut_sort_double();
}

/**
 * Check insertion sort operation upon an slist containing presorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_presorted, &slistut_insertion_sort)
{
	slistut_sort_presorted();
}

/**
 * Check insertion sort operation upon an slist containing unsorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_unsorted, &slistut_insertion_sort)
{
	slistut_sort_unsorted();
}

/**
 * Check insertion sort operation upon an slist containing items presorted in
 * reverse order
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_reverse_sorted, &slistut_insertion_sort)
{
	slistut_sort_reverse_sorted();
}

/**
 * Check insertion sort operation upon an slist containing 2 duplicate items
 * only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_double_duplicates, &slistut_insertion_sort)
{
	slistut_sort_double_duplicates();
}

/**
 * Check insertion sort operation upon an slist containing duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_duplicates, &slistut_insertion_sort)
{
	slistut_sort_duplicates();
}

/**
 * Check stability of insertion sort operation upon slist containing duplicate
 * items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_stable_unsorted, &slistut_insertion_sort)
{
	slistut_sort_stable_unsorted();
}

/**
 * Check insertion sort operation upon slist containing items presorted
 * according to insertion sort worst case
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_worstins_sorted, &slistut_insertion_sort)
{
	slistut_sort_worstins_sorted();
}

/**
 * Check insertion sort operation upon slist containing large number of items
 * with duplicates
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_insertion_sort_large_mix, &slistut_insertion_sort)
{
	slistut_sort_large_mix();
}

static CUTE_PNP_FIXTURED_SUITE(slistut_merge_sort, &slistut,
                               slistut_setup_merge_sort, NULL);

/**
 * Check merge sort operation upon an slist containing a single items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_single, &slistut_merge_sort)
{
	slistut_sort_single();
}

/**
 * Check merge sort operation upon an slist containing 2 items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_double, &slistut_merge_sort)
{
	slistut_sort_double();
}

/**
 * Check merge sort operation upon an slist containing presorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_presorted, &slistut_merge_sort)
{
	slistut_sort_presorted();
}

/**
 * Check merge sort operation upon an slist containing unsorted items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_unsorted, &slistut_merge_sort)
{
	slistut_sort_unsorted();
}

/**
 * Check merge sort operation upon an slist containing items presorted in
 * reverse order
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_reverse_sorted, &slistut_merge_sort)
{
	slistut_sort_reverse_sorted();
}

/**
 * Check merge sort operation upon an slist containing 2 duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_double_duplicates, &slistut_merge_sort)
{
	slistut_sort_double_duplicates();
}

/**
 * Check merge sort operation upon an slist containing duplicate items only
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_duplicates, &slistut_merge_sort)
{
	slistut_sort_duplicates();
}

/**
 * Check stability of a merge sort operation upon slist containing duplicate
 * items
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_stable_unsorted, &slistut_merge_sort)
{
	slistut_sort_stable_unsorted();
}

/**
 * Check a merge sort operation upon slist containing items presorted according
 * to insertion sort worst case
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_worstins_sorted, &slistut_merge_sort)
{
	slistut_sort_worstins_sorted();
}

/**
 * Check a merge sort operation upon slist containing large number of items with
 * duplicates
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_merge_sort_large_mix, &slistut_merge_sort)
{
	slistut_sort_large_mix();
}

static CUTE_PNP_FIXTURED_SUITE(slistut_runlen_merge_sort, &slistut,
                               slistut_setup_runlen_merge_sort, NULL);

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 1
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen1_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 1;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 2
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen2_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 2;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 3
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen3_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 3;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 4
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen4_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 4;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 5
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen5_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 5;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 6
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen6_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 6;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 7
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen7_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 7;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 8
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen8_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 8;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 13
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen13_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 13;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 16
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen16_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 16;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 27
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen27_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 27;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 32
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen32_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 32;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 53
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen53_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 53;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 64
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen64_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 64;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 91
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen91_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 91;
	slistut_dosort_large_mix();
}

/**
 * Check a hybrid merge sort operation upon slist containing large number of
 * items with duplicates using an initial run length of 128
 *
 * @ingroup slistut
 */
CUTE_PNP_TEST(slistut_runlen128_merge_sort, &slistut_runlen_merge_sort)
{
	slistut_merge_run_len = 128;
	slistut_dosort_large_mix();
}
