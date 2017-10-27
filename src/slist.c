/**
 * @file      slist.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      26 Jun 2017
 * @copyright GNU Public License v3
 *
 * Singly linked list implementation
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
#include "utils.h"
#include <errno.h>

/******************************************************************************
 * Performance events accounting
 ******************************************************************************/

#if defined(CONFIG_SLIST_PERF_EVENTS)

/* slist performance counters */
static struct slist_perf_events slist_perf_evts;

/*
 * Update the number of compare events for sorting performance evaluation
 * purposes.
 */
static void slist_account_compare_event(void)
{
	slist_perf_evts.compare++;
}

/*
 * Update the number of key swap events for sorting performance evaluation
 * purposes.
 */
static void slist_account_swap_event(void)
{
	slist_perf_evts.swap++;
}

const struct slist_perf_events * slist_fetch_perf_events(void)
{
	return &slist_perf_evts;
}

void slist_clear_perf_events(void)
{
	slist_perf_evts.compare = 0;
	slist_perf_evts.swap = 0;
}

#else /* !defined(CONFIG_SLIST_PERF_EVENTS) */

static inline void slist_account_compare_event(void) { }

static inline void slist_account_swap_event(void) { }

#endif /* defined(CONFIG_SLIST_PERF_EVENTS) */

/******************************************************************************
 * Singly linked list primitives
 ******************************************************************************/

void slist_move(struct slist      *list,
                struct slist_node *restrict at,
                struct slist_node *restrict previous,
                struct slist_node *restrict node)
{
	slist_remove(list, previous, node);
	slist_append(list, at, node);
}

void slist_splice(struct slist      *restrict result,
                  struct slist_node *restrict at,
                  struct slist      *restrict source,
                  struct slist_node *restrict first,
                  struct slist_node *restrict last)
{
	struct slist_node *fst = slist_next(first);

	slist_withdraw(source, first, last);
	slist_embed(result, at, fst, last);
}

/******************************************************************************
 * Singly linked list insertion sorting
 ******************************************************************************/

#if defined(CONFIG_SLIST_INSERTION_SORT)

/*
 * Insert node into slist in order.
 *
 * @param list    slist to insert node into
 * @param node    node to insert
 * @param compare comparison function used to perform in order insertion
 *
 * @ingroup slist
 */
static void slist_insert_inorder(struct slist      *restrict list,
                                 struct slist_node *restrict node,
                                 slist_compare_fn  *compare)
{
	assert(list);
	assert(node);
	assert(compare);

	struct slist_node *prev = slist_head(list);
	struct slist_node *cur = slist_next(prev);

	/*
	 * No need to check for end of list since cur must be inserted before
	 * current sorted list tail, i.e., cur will always be inserted before an
	 * existing node.
	 */
	while (true) {
		assert(cur);

		/*
		 * Although it seems a pretty good place to perform some
		 * prefetching, performance measurement doesn't show significant
		 * improvements... For most data sets, prefetching enhance
		 * processing time by an amount < 4/5%. In the worst case, this
		 * incurs a 3/4% penalty.  Measurements were done onto amd64
		 * platform with moderate read-only prefetching scheme (giving
		 * the best results) : prefetch(slist_next(cur),
		 * PREFETCH_ACCESS_RO, PREFETCH_LOCALITY_LOW);
		 *
		 * Additional code and complexity don't really worth it...
		 */

		slist_account_compare_event();

		if (compare(node, cur) < 0)
			break;

		prev = cur;
		cur = slist_next(cur);
	}

	slist_append(list, prev, node);
}

void slist_insertion_sort(struct slist *list, slist_compare_fn *compare)
{
	assert(!slist_empty(list));
	assert(compare);

	struct slist_node *prev = slist_first(list);
	struct slist_node *cur = slist_next(prev);

	while (cur) {
		slist_account_compare_event();

		if (compare(cur, prev) < 0) {
			slist_account_swap_event();

			slist_remove(list, prev, cur);
			slist_insert_inorder(list, cur, compare);

			cur = slist_next(prev);
			continue;
		}

		prev = cur;
		cur = slist_next(cur);
	}
}

void slist_counted_insertion_sort(struct slist     *restrict result,
                                  struct slist     *restrict source,
                                  unsigned int      count,
                                  slist_compare_fn *compare)
{
	assert(slist_empty(result));
	assert(!slist_empty(source));
	assert(compare);

	struct slist_node *prev = slist_first(source);
	struct slist_node *cur = slist_next(prev);

	while (--count && cur) {
		slist_account_compare_event();

		if (compare(cur, prev) < 0) {
			slist_account_swap_event();

			slist_remove(source, prev, cur);
			slist_insert_inorder(source, cur, compare);

			cur = slist_next(prev);
			continue;
		}

		prev = cur;
		cur = slist_next(cur);
	}

	slist_splice(result, slist_head(result),
	             source, slist_head(source), prev);
}

#endif /* defined(CONFIG_SLIST_INSERTION_SORT) */

/******************************************************************************
 * Singly linked list selection sorting
 ******************************************************************************/

#if defined(CONFIG_SLIST_SELECTION_SORT)

/*
 * Thanks to slist, implementation is always stable: swap is always performed
 * after the tail of sorted sublist.
 */
void slist_selection_sort(struct slist *list, slist_compare_fn *compare)
{
	assert(!slist_empty(list));
	assert(compare);

	/* Tail of sorted sublist. */
	struct slist_node *tail = slist_head(list);

	while (true) {
		assert(tail);

		struct slist_node *prev = slist_next(tail);
		struct slist_node *cur;
		struct slist_node *prev_min;
		struct slist_node *cur_min;

		if (prev == slist_last(list))
			break;

		cur = slist_next(prev);
		prev_min = tail;
		cur_min = prev;

		do {
			slist_account_compare_event();
			if (compare(cur, cur_min) < 0) {
				prev_min = prev;
				cur_min = cur;
			}

			prev = cur;
			cur = slist_next(cur);
		} while (cur);

		if (cur_min != slist_next(tail)) {
			slist_account_swap_event();
			slist_move(list, tail, prev_min, cur_min);
		}

		tail = cur_min;
	}
}

#endif /* defined(CONFIG_SLIST_SELECTION_SORT) */

/******************************************************************************
 * Singly linked list bubble sorting
 ******************************************************************************/

#if defined(CONFIG_SLIST_BUBBLE_SORT)

/* implementation is always stable */
void slist_bubble_sort(struct slist *list, slist_compare_fn *compare)
{
	assert(!slist_empty(list));
	assert(compare);

	struct slist_node *head = NULL;
	struct slist_node *swap;

	do {
		struct slist_node *cur = slist_head(list);
		struct slist_node *prev;
		struct slist_node *nxt;

		swap = NULL;

		while (true) {
			/*
			 * Find the next swap location by progressing along the
			 * list untill end of list.
			 */
			do {
				prev = cur;
				cur = slist_next(cur);
				nxt = slist_next(cur);
				if (nxt == head)
					nxt = NULL;

				if (!nxt)
					/* End of sublist. */
					break;

				slist_account_compare_event();
				if (compare(cur, nxt) > 0)
					/* Swap required. */
					break;
			} while (true);

			if (!nxt) {
				/* End of current pass. */
				head = cur;
				break;
			}

			/* Extract out of order element. */
			slist_remove(list, prev, cur);

			swap = cur;
			cur = nxt;
			do {
				prev = cur;
				cur = slist_next(cur);
				if (cur == head)
					cur = NULL;

				if (!cur)
					/* End of sublist. */
					break;

				slist_account_compare_event();
				if (compare(swap, cur) <= 0)
					/* Swap location found. */
					break;
			} while (true);

			slist_account_swap_event();
			slist_append(list, prev, swap);

			if (!cur) {
				/* End of current pass. */
				head = swap;
				break;
			}

			cur = swap;
		}
	} while (swap);
}

#endif /* defined(CONFIG_SLIST_BUBBLE_SORT) */

/******************************************************************************
 * Singly linked list hybrid merge sorting
 ******************************************************************************/

#if defined(CONFIG_SLIST_MERGE_SORT)

/*
 * Merge 2 sorted (sub)lists segments.
 *
 * @param result  slist within which both slists will be sorted into.
 * @param at      First node of result's segment.
 * @param source  Source slist to sort within result.
 * @param compare comparison function used to perform in order insertion
 *
 * @return Last sorted node merged into result.
 *
 * @ingroup slist
 */
static struct slist_node * slist_merge_sorted_subs(struct slist      *result,
                                                   struct slist_node *at,
                                                   struct slist      *source,
                                                   slist_compare_fn  *compare)
{
	struct slist_node *res_cur = at;
	struct slist_node *res_nxt = at;
	struct slist_node *ref = slist_first(source);
	struct slist_node *src_cur;
	struct slist_node *src_nxt;

	assert(!slist_empty(result));
	assert(at);
	assert(!slist_empty(source));

	slist_account_compare_event();
	if (compare(ref, slist_last(result)) >= 0) {
		/*
		 * Fast path: don't bother sorting every nodes if source list
		 * comes entirely after result one.
		 */
		res_cur = slist_last(result);
		src_cur = slist_last(source);
		goto splice;
	}

	/*
	 * Find the first node in result which should be located after first
	 * source list's node.
	 */
	while (true) {
		res_nxt = slist_next(res_nxt);
		if (!res_nxt)
			break;

		slist_account_compare_event();
		if (compare(res_nxt, ref) > 0)
			break;

		res_cur = res_nxt;
	}

	src_cur = slist_last(source);

	if (!res_nxt)
		/* Nothing's left in result: join source after result. */
		goto splice;

	slist_account_compare_event();
	if (compare(res_nxt, src_cur) > 0)
		/*
		 * The entire source list may be inserted before current
		 * result's node.
		 */
		goto splice;

	/*
	 * Search for the longest source nodes segment that may be inserted
	 * befor current result's node.
	 */
	src_cur = slist_head(source);
	src_nxt = src_cur;
	while (true) {
		src_nxt = slist_next(src_nxt);
		if (!src_nxt)
			break;

		slist_account_compare_event();
		if (compare(src_nxt, res_nxt) >= 0)
			break;

		src_cur = src_nxt;
	}

	/* Embed source nodes segment at the right location into result. */
splice:
	slist_account_swap_event();
	slist_splice(result, res_cur, source, slist_head(source), src_cur);

	/* Return last sorted node. */
	return src_cur;
}

void slist_merge_presort(struct slist     *result,
                         struct slist     *source,
                         slist_compare_fn *compare)
{
	assert(!slist_empty(result));
	assert(!slist_empty(source));
	assert(compare);

	struct slist_node *at = slist_head(result);

	do {
		at = slist_merge_sorted_subs(result, at, source, compare);
	} while (!slist_empty(source));
}

/*
 * Iteratively split list and merge presorted sublists.
 *
 * @param list    slist to sort.
 * @param run_len Primary sorting run length in number of nodes.
 * @param subs_nr Logarithm base 2 of number of merging pass.
 * @param compare Comparison function used to perform in order insertion.
 *
 * @note
 * Merge sorting sublists are allocated onto stack since implementation needs
 * only logarithmic auxiliary space.
 *
 * @ingroup slist
 */
static void slist_split_merge_sort(struct slist     *list,
                                   unsigned int      run_len,
                                   unsigned int      subs_nr,
                                   slist_compare_fn *compare)
{
	assert(subs_nr);

	unsigned int cnt;
	struct slist subs[subs_nr]; /* Reserve space for sublists merging. */

	for (cnt = 0; cnt < subs_nr; cnt++)
		slist_init(&subs[cnt]);

	subs_nr = 0;
	do {
		/*
		 * Sort an initial run using insertion sort and store the result
		 * in the current sublist.
		 */
		slist_counted_insertion_sort(&subs[0], list, run_len, compare);

		/* Merge up 2 runs / sublists of the same length in a row.*/
		cnt = 1;
		while (!slist_empty(&subs[cnt])) {
			slist_merge_presort(&subs[cnt], &subs[cnt - 1],
			                    compare);
			cnt++;
		}

		subs[cnt] = subs[cnt - 1];
		slist_init(&subs[cnt - 1]);

		subs_nr = max(subs_nr, cnt);
	} while (!slist_empty(list));

	/*
	 * Finally merge remaining runs into a single sorted list. To preserve
	 * stability, iterate over the sublists array in reverse order since
	 * earlier runs were "pushed" in the upper areas of the array.
	 */
	*list = subs[subs_nr];
	while (subs_nr--)
		if (!slist_empty(&subs[subs_nr]))
			slist_merge_presort(list, &subs[subs_nr], compare);
}

void slist_hybrid_merge_sort(struct slist     *list,
                             unsigned int      run_len,
                             unsigned int      nodes_nr,
                             slist_compare_fn *compare)
{
	assert(!slist_empty(list));
	assert(run_len);
	assert(nodes_nr);
	assert(compare);

	/* Perform the real merge sort. */
	slist_split_merge_sort(list, run_len,
	                       upper_pow2(max(nodes_nr / run_len, 2U)) + 2,
	                       compare);
}

void slist_merge_sort(struct slist     *list,
                      unsigned int      nodes_nr,
                      slist_compare_fn *compare)
{
	assert(nodes_nr);

	unsigned int run_len;

	if (nodes_nr <= 4)
		/* Switch to insertion sorting for trivial cases. */
		return slist_insertion_sort(list, compare);

	if (nodes_nr <= 16)
		run_len = 4;
	else if (nodes_nr <= 128)
		run_len = 8;
	else if (nodes_nr <= 1024)
		run_len = 16;
	else if (nodes_nr <= (8*1024))
		run_len = 32;
	else if (nodes_nr <= (64*1024))
		run_len = 64;
	else
		run_len = 128;

	slist_hybrid_merge_sort(list, run_len, nodes_nr, compare);
}

#endif /* defined(CONFIG_SLIST_MERGE_SORT) */

#if 0
odd-even/brick sort
cyclesort ?
introsort ?
quicksort ?
quickselect ?
introselect ?
bingosort
countingsort
#endif
