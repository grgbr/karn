#include "karn_pt.h"
#include <karn/slist.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

static unsigned int run_len = 0;

typedef void (sort_fn)(struct slist *, unsigned int, slist_compare_fn *);

#if defined(CONFIG_KARN_SLIST_BUBBLE_SORT)

static void
bubble_sort(struct slist     *list,
            unsigned int      nr __unused,
            slist_compare_fn *compare)
{
	slist_bubble_sort(list, compare);
}

static sort_fn *
setup_bubble_sort(const char *scheme)
{
	if (!strcmp(scheme, "bubble"))
		return bubble_sort;

	errno = EINVAL;
	return NULL;
}

#else /* !defined(CONFIG_KARN_SLIST_BUBBLE_SORT) */

static sort_fn *
setup_bubble_sort(const char *scheme __unused)
{
	if (!strcmp(scheme, "bubble"))
		errno = ENOSYS;
	else
		errno = EINVAL;

	return NULL;
}

#endif /* defined(CONFIG_KARN_SLIST_BUBBLE_SORT) */

#if defined(CONFIG_KARN_SLIST_SELECTION_SORT)

static void
selection_sort(struct slist     *list,
               unsigned int      nr __unused,
               slist_compare_fn *compare)
{
	slist_selection_sort(list, compare);
}

static sort_fn *
setup_selection_sort(const char *scheme)
{
	if (!strcmp(scheme, "selection"))
		return selection_sort;

	errno = EINVAL;
	return NULL;
}

#else /* !defined(CONFIG_KARN_SLIST_SELECTION_SORT) */

static sort_fn *
setup_selection_sort(const char *scheme __unused)
{
	if (!strcmp(scheme, "selection"))
		errno = ENOSYS;
	else
		errno = EINVAL;

	return NULL;
}

#endif /* defined(CONFIG_KARN_SLIST_SELECTION_SORT) */

#if defined(CONFIG_KARN_SLIST_INSERTION_SORT)

static void
insertion_sort(struct slist     *list,
               unsigned int      nr __unused,
               slist_compare_fn *compare)
{
	slist_insertion_sort(list, compare);
}

static sort_fn *
setup_insertion_sort(const char *scheme)
{
	if (!strcmp(scheme, "insertion"))
		return insertion_sort;

	errno = EINVAL;
	return NULL;
}

#else /* !defined(CONFIG_KARN_SLIST_INSERTION_SORT) */

static sort_fn *
setup_insertion_sort(const char *scheme __unused)
{
	if (!strcmp(scheme, "insertion"))
		errno = ENOSYS;
	else
		errno = EINVAL;

	return NULL;
}

#endif /* defined(CONFIG_KARN_SLIST_INSERTION_SORT) */

#if defined(CONFIG_KARN_SLIST_MERGE_SORT)

static void
merge_sort(struct slist *list, unsigned int nr, slist_compare_fn *compare)
{
	if (!run_len)
		slist_merge_sort(list, nr, compare);
	else
		slist_hybrid_merge_sort(list, nr, run_len, compare);
}

static sort_fn *
setup_merge_sort(const char *scheme)
{
	if (!strcmp(scheme, "merge"))
		return merge_sort;

	errno = EINVAL;
	return NULL;
}

#else /* !defined(CONFIG_KARN_SLIST_MERGE_SORT) */

static sort_fn *
setup_merge_sort(const char *scheme __unused)
{
	if (!strcmp(scheme, "merge"))
		errno = ENOSYS;
	else
		errno = EINVAL;

	return NULL;
}

#endif /* defined(CONFIG_KARN_SLIST_MERGE_SORT) */

static sort_fn *
setup_sort(const char *scheme)
{
	typedef sort_fn * (setup_sort_fn)(const char *);
	static setup_sort_fn * const setup[] = {
		setup_bubble_sort,
		setup_selection_sort,
		setup_insertion_sort,
		setup_merge_sort
	};
	unsigned int                 s;

	for (s = 0; s < array_nr(setup); s++) {
		sort_fn *sort;

		sort = setup[s](scheme);
		if (sort)
			return sort;

		if (errno == ENOSYS)
			return NULL;
	}

	errno = EINVAL;
	return NULL;
}

struct slist_uint {
	struct slist_node node;
	uint32_t          value;
};

static inline int
compare(const struct slist_node *a, const struct slist_node *b)
{
	return pt_compare_min((char *)&slist_entry(a, struct slist_uint,
	                                           node)->value,
	                      (char *)&slist_entry(b, struct slist_uint,
	                                           node)->value);
}

static unsigned long long
account_sort(const struct pt_entries *entries,
             struct slist            *list,
             struct slist_uint       *keys,
             sort_fn                 *sort)
{
	struct slist_uint *k;
	struct timespec    start, elapse;

	pt_init_entry_iter(entries);

	slist_init(list);
	k = keys;
	while (!pt_iter_entry(entries, &k->value)) {
		slist_nqueue(list, &k->node);
		k++;
	}

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	sort(list, entries->pt_nr, compare);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = pt_tspec_sub(&elapse, &start);

	return ((long long)elapse.tv_sec * 1000000000LL) +
	       (long long)elapse.tv_nsec;
}

static void
usage(const char *me)
{
	fprintf(stderr,
	        "Usage: %s [OPTIONS] FILE ALGORITHM LOOPS\n"
	        "where OPTIONS:\n"
	        "    -p|--prio  PRIORITY\n"
	        "    -r|--run   RUN_LEN\n"
	        "    -h|--help\n",
	        me);
}

int main(int argc, char *argv[])
{
	struct pt_entries   entries;
	char               *errstr;
	unsigned int        loops = 0;
	struct slist        list;
	struct slist_uint  *keys;
	struct slist_node  *n;
	int                 prio = 0;
	sort_fn            *sort;
	long long           nsec;

	while (true) {
		int                        opt;
		static const struct option lopts[] = {
			{"help",    0, NULL, 'h'},
			{"prio",    1, NULL, 'p'},
			{"run",    1, NULL,  'r'},
			{0,         0, 0,    0}
		};

		opt = getopt_long(argc, argv, "hp:r:", lopts, NULL);
		if (opt < 0)
			/* No more options: go parsing positional arguments. */
			break;

		switch (opt) {
		case 'p': /* priority */
			if (pt_parse_sched_prio(optarg, &prio)) {
				usage(argv[0]);
				return EXIT_FAILURE;
			}

			break;

		case 'r': /* init run length */
			run_len = (int)strtoul(optarg, &errstr, 0);
			if (*errstr || run_len < 1) {
				fprintf(stderr, "Invalid initial run length\n");
				usage(argv[0]);
				return EXIT_FAILURE;
			}

			break;

		case 'h': /* Help message. */
			usage(argv[0]);
			return EXIT_SUCCESS;

		case '?': /* Unknown option. */
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/*
	 * Check positional arguments are properly specified on command
	 * line.
	 */
	argc -= optind;

	switch (argc) {
	case 3:
		break;

	default:
		fprintf(stderr, "Missing argument\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	sort = setup_sort(argv[optind + 1]);
	if (!sort) {
		fprintf(stderr, "Invalid \"%s\" sort algorithm: %s\n",
		        argv[optind + 1], strerror(errno));
		return EXIT_FAILURE;
	}

	if (pt_parse_loop_nr(argv[optind + 2], &loops))
		return EXIT_FAILURE;

	if (pt_open_entries(argv[optind], &entries))
		return EXIT_FAILURE;

	keys = malloc(entries.pt_nr * sizeof(keys[0]));
	if (!keys)
		return EXIT_FAILURE;

	account_sort(&entries, &list, keys, sort);

	slist_foreach_node(&list, n) {
		const struct slist_node *nxt = slist_next(n);

		if (!nxt)
			break;

		if (compare(n, nxt) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			return EXIT_FAILURE;
		}
	}

	if (pt_setup_sched_prio(prio))
		return EXIT_FAILURE;

	while (loops--) {
		const struct slist_perf_events *evts;

		slist_clear_perf_events();

		nsec = account_sort(&entries, &list, keys, sort);

		evts = slist_fetch_perf_events();
		if (evts)
			printf("nsec=%llu cmp=%llu swap=%llu\n",
			       nsec, evts->compare, evts->swap);
		else
			printf("nsec: %llu\n", nsec);
	}

	return EXIT_SUCCESS;
}
