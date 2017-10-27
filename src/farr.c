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
	const char *end = &entries[entry_nr * entry_size];
	char       *unsort = entries + entry_size;

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

static const char * farr_quick_sort_pivot(char            *begin,
                                          char            *end,
                                          size_t           entry_size,
                                          farr_compare_fn *compare,
                                          farr_copy_fn    *copy)
{
	char *mid = begin + (((end - begin) / (2 * entry_size)) * entry_size);
	char  tmp[entry_size];

	if (compare(begin, mid) > 0)
		farr_swap(begin, mid, tmp, copy);

	if (compare(mid, end) > 0)
		farr_swap(mid, end, tmp, copy);

	if (compare(begin, end) > 0)
		farr_swap(begin, end, tmp, copy);

	return mid;
}

static char * farr_quick_part(char            *begin,
                              char            *end,
                              size_t           entry_size,
                              farr_compare_fn *compare,
                              farr_copy_fn    *copy)
{
	char pivot[entry_size];

	copy(pivot,
	     farr_quick_sort_pivot(begin, end, entry_size, compare, copy));

	begin -= entry_size;
	end += entry_size;

	while (true) {
		char tmp[entry_size];

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

static void farr_quick_sort_part(char            *begin,
                                 char            *end,
                                 size_t           entry_size,
                                 farr_compare_fn *compare,
                                 farr_copy_fn    *copy)
{
	if ((size_t)(end - begin) > (32 * entry_size)) {
		char *border;

		border = farr_quick_part(begin, end, entry_size, compare, copy);

		farr_quick_sort_part(begin, border, entry_size, compare, copy);
		farr_quick_sort_part(border + entry_size, end, entry_size,
		                     compare, copy);
	}
	else
		farr_insertion_sort(begin, entry_size,
		                    ((end - begin) / entry_size) + 1, compare,
		                    copy);
}

void farr_quick_sort(char            *entries,
                     size_t           entry_size,
                     unsigned int     entry_nr,
                     farr_compare_fn *compare,
                     farr_copy_fn    *copy)
{
	farr_quick_sort_part(entries, &entries[(entry_nr - 1) * entry_size],
	                     entry_size, compare, copy);
}

#endif /* defined(CONFIG_FARR_QUICK_SORT) */
