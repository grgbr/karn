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

void farr_insertion_sort(char            *entries,
                         size_t           entry_size,
                         unsigned int     entry_nr,
                         farr_compare_fn *compare,
                         farr_copy_fn    *copy)
{
	char       *unsort = entries + entry_size;
	const char *end = &entries[entry_nr * entry_size];

	while (unsort < end) {
		char  tmp[entry_size];
		char *ent = unsort - entry_size;

		copy(tmp, unsort);

		while ((ent >= entries) && (compare(tmp, ent) <= 0)) {
			copy(ent + entry_size, ent);
			ent -= entry_size;
		}

		copy(ent + entry_size, tmp);

		unsort += entry_size;
	}
}

#endif /* defined(CONFIG_FARR_INSERTION_SORT) */

#if defined(CONFIG_FARR_QUICK_SORT)

#define FARR_QUICK_SORT_THRES (24U)

static char * farr_quick_part(char            *begin,
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

static unsigned int farr_quick_stack_nr(size_t entry_nr)
{
	return upper_pow2(max((entry_nr + FARR_QUICK_SORT_THRES - 1) /
	                      FARR_QUICK_SORT_THRES, 2U));
}

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

	struct {
		char *begin;
		char *end;
	}                     parts[farr_quick_stack_nr(entry_nr)];
	unsigned int          p = 0;
	char                 *begin = entries;
	char                 *end = &begin[(entry_nr - 1) * entry_size];

	while (true) {
		size_t  thres = FARR_QUICK_SORT_THRES * entry_size;
		char   *pivot;
		char   *high;

		while (((size_t)begin + thres) >= (size_t)end) {
			if (!p--)
				return farr_insertion_sort(entries, entry_size,
				                           entry_nr, compare,
				                           copy);
			begin = parts[p].begin;
			end = parts[p].end;
		}

		assert(p < array_nr(parts));

		pivot = farr_quick_part(begin, end, entry_size, compare, copy);

		high = pivot + entry_size;
		if ((high - begin) >= (end - pivot)) {
			parts[p].begin = begin;
			parts[p].end = pivot;

			begin = high;
		}
		else {
			parts[p].begin = high;
			parts[p].end = end;

			end = pivot;
		}

		p++;
	}
}

#endif /* defined(CONFIG_FARR_QUICK_SORT) */
