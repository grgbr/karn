#include "slist.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

static unsigned int run_len = 0;

typedef void (sort_fn)(struct slist *, unsigned int, slist_compare_fn *);

static void
insertion_sort(struct slist     *list,
               unsigned int      nr __unused,
               slist_compare_fn *compare)
{
	slist_insertion_sort(list, compare);
}

static void
selection_sort(struct slist     *list,
               unsigned int      nr __unused,
               slist_compare_fn *compare)
{
	slist_selection_sort(list, compare);
}

static void
bubble_sort(struct slist     *list,
            unsigned int      nr __unused,
            slist_compare_fn *compare)
{
	slist_bubble_sort(list, compare);
}

static void
merge_sort(struct slist *list, unsigned int nr, slist_compare_fn *compare)
{
	if (!run_len)
		slist_merge_sort(list, nr, compare);
	else
		slist_hybrid_merge_sort(list, nr, run_len, compare);
}

struct slist_uint {
	struct slist_node node;
	uint32_t          value;
};

static struct timespec
tspec_sub(const struct timespec *restrict a, const struct timespec *restrict b)
{
	struct timespec res = {
		.tv_sec  = a->tv_sec - b->tv_sec,
		.tv_nsec = a->tv_nsec - b->tv_nsec
	};

	if (res.tv_nsec < 0) {
		res.tv_sec--;
		res.tv_nsec += 1000000000;
	}

	return res;
}

static int
compare(const struct slist_node *a, const struct slist_node *b)
{
	return slist_entry(a, struct slist_uint, node)->value -
	       slist_entry(b, struct slist_uint, node)->value;
}

static unsigned long long
account_sort(FILE              *file,
             struct slist      *list,
             struct slist_uint *keys,
             sort_fn           *sort,
             unsigned int       key_nr)
{
	struct slist_uint *k;
	struct timespec    start, elapse;

	rewind(file);

	slist_init(list);
	k = keys;
	while (fread(&k->value, sizeof(k->value), 1, file) == 1) {
		slist_nqueue(list, &k->node);
		k++;
	}

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
	sort(list, key_nr, compare);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &elapse);

	elapse = tspec_sub(&elapse, &start);

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
	FILE               *file;
	int                 nr;
	char               *errstr;
	unsigned int        loops = 0;
	struct slist        list;
	struct slist_uint  *keys;
	struct slist_node  *n;
	struct sched_param  parm = { 0, };
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
			parm.sched_priority = (int)strtoul(optarg, &errstr, 0);
			if (*errstr) {
				fprintf(stderr, "Invalid priority\n");
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

	if (!strcmp(argv[optind + 1], "insertion"))
		sort = insertion_sort;
	else if (!strcmp(argv[optind + 1], "selection"))
		sort = selection_sort;
	else if (!strcmp(argv[optind + 1], "bubble"))
		sort = bubble_sort;
	else if (!strcmp(argv[optind + 1], "merge"))
		sort = merge_sort;
	else {
		fprintf(stderr, "Invalid sort algorithm \"%s\"\n",
		        argv[optind + 1]);
		return EXIT_FAILURE;
	}

	loops = strtoul(argv[optind + 2], &errstr, 0);
	if (*errstr)
		loops = 0;
	if (!loops) {
		fprintf(stderr, "Invalid number of loops\n");
		return EXIT_FAILURE;
	}

	file = fopen(argv[optind], "r");
	if (!file) {
		fprintf(stderr, "Failed to open file %s: %s\n", argv[optind],
		        strerror(errno));
		return EXIT_FAILURE;
	}

	if (fseek(file, 0, SEEK_END)) {
		perror("Failed to probe file end");
		return EXIT_FAILURE;
	}

	nr = ftell(file);
	if (nr < 0) {
		perror("Failed to probe file size");
		return EXIT_FAILURE;
	}

	nr /= (int)sizeof(uint32_t);
	if (nr <= 0) {
		fprintf(stderr, "Invalid file content\n");
		return EXIT_FAILURE;
	}

	keys = malloc(nr * sizeof(keys[0]));
	if (!keys)
		return EXIT_FAILURE;

	account_sort(file, &list, keys, sort, nr);

	slist_foreach_node(&list, n) {
		const struct slist_node *nxt = slist_next(n);

		if (!nxt)
			break;

		if (compare(n, nxt) > 0) {
			fprintf(stderr, "Bogus sorting scheme\n");
			return EXIT_FAILURE;
		}
	}

	if (parm.sched_priority) {
		if (sched_setscheduler(getpid(), SCHED_FIFO, &parm)) {
			perror("Failed set scheduling policy");
			return EXIT_FAILURE;
		}
	}

	while (loops--) {
		const struct slist_perf_events *evts;

		slist_clear_perf_events();

		nsec = account_sort(file, &list, keys, sort, nr);

		evts = slist_fetch_perf_events();
		if (evts)
			printf("nsec=%llu cmp=%llu swap=%llu\n",
			       nsec, evts->compare, evts->swap);
		else
			printf("nsec: %llu\n", nsec);
	}

	return EXIT_SUCCESS;
}
