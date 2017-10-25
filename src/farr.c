#include "farr.h"

#if defined(CONFIG_FARR_BUBBLE_SORT)

extern void farr_bubble_sort(char            *entries,
                             size_t           entry_size,
                             size_t           entry_nr,
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
