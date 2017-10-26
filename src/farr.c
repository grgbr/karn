#include "farr.h"

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

				copy(tmp, ent);
				copy(ent, neigh);
				copy(neigh, tmp);

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

		copy(tmp, unsort);
		copy(unsort, min);
		copy(min, tmp);

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
		char *ent = unsort - entry_size;
		char  tmp[entry_size];

		copy(tmp, unsort);

		while ((ent >= entries) && (compare(ent, tmp) > 0)) {
			copy(ent + entry_size, ent);
			ent -= entry_size;
		}

		copy(ent + entry_size, tmp);

		unsort += entry_size;
	}
}

#endif /* defined(CONFIG_FARR_INSERTION_SORT) */
