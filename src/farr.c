#include "farr.h"
#include <stdbool.h>

static inline void farr_swap(char         *first,
                             char         *second,
                             char         *auxiliary,
                             farr_copy_fn *copy)
{
	copy(auxiliary, first);
	copy(first, second);
	copy(second, auxiliary);
}

#if defined(CONFIG_FARR_BUBBLE_SORT)

void farr_bubble_sort(char            *entries,
                      size_t           entry_size,
                      unsigned int     entry_nr,
                      farr_compare_fn *compare,
                      farr_copy_fn    *copy)
{
	const char *end = &entries[(entry_nr - 1) * entry_size];

	while (entries < end) {
		char       *ent = entries;
		const char *last = entries;

		do {
			char *neigh = ent + entry_size;

			if (compare(ent, neigh) > 0) {
				char tmp[entry_size];

				farr_swap(ent, neigh, tmp, copy);

				last = neigh;
			}

			ent = neigh;
		} while (ent < end);

		end = last;
	}
}

#endif /* defined(CONFIG_FARR_BUBBLE_SORT) */

#if defined(CONFIG_FARR_SELECTION_SORT)

void farr_selection_sort(char            *entries,
                         size_t           entry_size,
                         unsigned int     entry_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy)
{
	const char *end = &entries[(entry_nr - 1) * entry_size];
	char       *unsort = entries;

	while (unsort < end) {
		char *ent = unsort;
		char *min = unsort;
		char  tmp[entry_size];

		do {
			ent += entry_size;

			if (compare(ent, min) < 0)
				min = ent;
		} while (ent < end);

		farr_swap(unsort, min, tmp, copy);

		unsort += entry_size;
	}
}

#endif /* defined(CONFIG_FARR_SELECTION_SORT) */

#if defined(CONFIG_FARR_INSERTION_SORT)

static void _farr_insertion_sort(char            *begin,
                                 char            *end,
                                 size_t           entry_size,
                                 farr_compare_fn *compare,
                                 farr_copy_fn    *copy)
{
	char *unsort = begin + entry_size;

	while (unsort <= end) {
		char  tmp[entry_size];
		char *ent = unsort - entry_size;

		copy(tmp, unsort);

		while ((ent >= begin) && (compare(tmp, ent) <= 0)) {
			copy(ent + entry_size, ent);
			ent -= entry_size;
		}

		copy(ent + entry_size, tmp);

		unsort += entry_size;
	}
}

void farr_insertion_sort(char            *entries,
                         size_t           entry_size,
                         unsigned int     entry_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy)
{
	_farr_insertion_sort(entries, &entries[(entry_nr - 1) * entry_size],
	                     entry_size, compare, copy);
}


#endif /* defined(CONFIG_FARR_INSERTION_SORT) */

#if defined(CONFIG_FARR_QUICK_SORT_UTILS)

/*
 * TODO:
 *  - docs !! see https://stackoverflow.com/questions/6709055/quicksort-stack-size
 *  https://stackoverflow.com/questions/1582356/fastest-way-of-finding-the-middle-value-of-a-triple
 *  https://stackoverflow.com/questions/7559608/median-of-three-values-strategy
 */
static char * _farr_quick_hoare_part(char            *begin,
                                     char            *end,
                                     size_t           entry_size,
                                     farr_compare_fn *compare,
                                     farr_copy_fn    *copy)
{
	char  tmp[entry_size];
	char  pivot[entry_size];
	char *mid = begin + (((end - begin) / (2 * entry_size)) * entry_size);

	if (compare(begin, mid) > 0)
		farr_swap(begin, mid, tmp, copy);

	if (compare(mid, end) > 0) {
		farr_swap(mid, end, tmp, copy);

		if (compare(begin, mid) > 0)
			farr_swap(begin, mid, tmp, copy);
	}

	copy(pivot, mid);

	begin -= entry_size;
	end += entry_size;
	while (true) {
		do {
			begin += entry_size;
		} while (compare(pivot, begin) > 0);

		do {
			end -= entry_size;
		} while (compare(end, pivot) > 0);

		if (begin >= end)
			return end;

		farr_swap(begin, end, tmp, copy);
	}
}

static char * farr_quick_hoare_part(char            *begin,
                                    char            *end,
                                    size_t           entry_size,
                                    farr_compare_fn *compare,
                                    farr_copy_fn    *copy)
{
	char *pivot;

	pivot = _farr_quick_hoare_part(begin, end, entry_size, compare, copy);

	assert(begin <= pivot);
	assert(pivot < end);

	return pivot;
}

#endif /* defined(CONFIG_FARR_QUICK_SORT_UTILS) */

#if defined(CONFIG_FARR_QUICK_SORT)

#define FARR_QUICK_INSERT_THRESHOLD (32U)

static unsigned int farr_quick_stack_depth(size_t entry_nr)
{
	return upper_pow2(max((entry_nr + FARR_QUICK_INSERT_THRESHOLD - 1) /
	                      FARR_QUICK_INSERT_THRESHOLD, 2U));
}

static bool farr_quick_switch_insert(const char *begin,
                                     const char *end,
                                     size_t      entry_size)
{
	assert(end >= begin);

	return ((size_t)(end - begin) <=
	        ((FARR_QUICK_INSERT_THRESHOLD - 1) * entry_size));
}

struct farr_quick_part {
	char *begin;
	char *end;
};

void farr_quick_sort(char            *entries,
                     size_t           entry_size,
                     unsigned int     entry_nr,
                     farr_compare_fn *compare,
                     farr_copy_fn    *copy)
{
	assert(entries);
	assert(entry_size);
	assert(entry_nr);
	assert(compare);
	assert(copy);

	char                   *begin = entries;
	char                   *end = &begin[(entry_nr - 1) * entry_size];
	unsigned int            ptop = 0;
	struct farr_quick_part  parts[farr_quick_stack_depth(entry_nr)];

	while (true) {
		char *pivot;
		char *high;

		while (farr_quick_switch_insert(begin, end, entry_size)) {
			if (!ptop--)
				return farr_insertion_sort(entries, entry_size,
				                           entry_nr, compare,
				                           copy);
			begin = parts[ptop].begin;
			end = parts[ptop].end;
		}

		pivot = farr_quick_hoare_part(begin, end, entry_size, compare,
		                              copy);
		assert(ptop < array_nr(parts));

		high = pivot + entry_size;
		if ((high - begin) >= (end - pivot)) {
			parts[ptop].begin = begin;
			parts[ptop].end = pivot;
			begin = high;
		}
		else {
			parts[ptop].begin = high;
			parts[ptop].end = end;
			end = pivot;
		}

		ptop++;
	}
}

#endif /* defined(CONFIG_FARR_QUICK_SORT) */

#if defined(CONFIG_FARR_INTRO_SORT)

/*
 * TODO:
 *  - docs !! https://www.google.fr/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwi01va3-LvXAhUEORoKHc8HBWgQFggqMAA&url=http%3A%2F%2Fwww.cs.rpi.edu%2F~musser%2Fgp%2Fintrosort.ps&usg=AOvVaw0aZOd7_zwQpTwbJAb5ymAn
 */

#include "fbnr_heap.h"

#define FARR_INTRO_INSERT_THRESHOLD (32U)

static unsigned int farr_intro_stack_depth(size_t entry_nr)
{
	return upper_pow2(max((entry_nr + FARR_INTRO_INSERT_THRESHOLD - 1) /
	                      FARR_INTRO_INSERT_THRESHOLD, 2U));
}

static unsigned int farr_intro_heap_threshold(size_t entry_nr)
{
	return 2 * farr_intro_stack_depth(entry_nr);
}

static bool farr_intro_switch_insert(const char *begin,
                                     const char *end,
                                     size_t      entry_size)
{
	assert(end >= begin);

	return ((size_t)(end - begin) <=
	        ((FARR_INTRO_INSERT_THRESHOLD - 1) * entry_size));
}

struct farr_intro_part {
	char         *begin;
	char         *end;
	unsigned int  threshold;
};

void farr_intro_sort(char            *entries,
                     size_t           entry_size,
                     unsigned int     entry_nr,
                     farr_compare_fn *compare,
                     farr_copy_fn    *copy)
{
	assert(entries);
	assert(entry_size);
	assert(entry_nr);
	assert(compare);
	assert(copy);

	char                   *begin = entries;
	char                   *end = &begin[(entry_nr - 1) * entry_size];
	unsigned int            thres = farr_intro_heap_threshold(entry_nr);
	unsigned int            ptop = 0;
	struct farr_intro_part  parts[farr_intro_stack_depth(entry_nr)];

	while (true) {
		char *pivot;
		char *high;

		while (farr_intro_switch_insert(begin, end, entry_size)) {
			_farr_insertion_sort(begin, end, entry_size, compare,
			                     copy);
			if (!ptop--)
				return;

			begin = parts[ptop].begin;
			end = parts[ptop].end;
			thres = parts[ptop].threshold;
		}

		if (!thres) {
			fbnr_heap_sort(begin, entry_size,
			               (end + entry_size - begin) / entry_size,
			               compare, copy);
			if (!ptop--)
				return;

			begin = parts[ptop].begin;
			end = parts[ptop].end;
			thres = parts[ptop].threshold;

			continue;
		}

		pivot = farr_quick_hoare_part(begin, end, entry_size, compare,
		                              copy);
		assert(ptop < array_nr(parts));

		high = pivot + entry_size;
		if ((high - begin) >= (end - pivot)) {
			parts[ptop].begin = begin;
			parts[ptop].end = pivot;
			begin = high;
		}
		else {
			parts[ptop].begin = high;
			parts[ptop].end = end;
			end = pivot;
		}
		parts[ptop].threshold = --thres;

		ptop++;
	}
}

#endif /* defined(CONFIG_FARR_INTRO_SORT) */
