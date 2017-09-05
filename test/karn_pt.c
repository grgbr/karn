#include "karn_pt.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

struct timespec
pt_tspec_sub(const struct timespec *restrict a,
             const struct timespec *restrict b)
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

int
pt_parse_loop_nr(const char *arg, unsigned int *loop_nr)
{
	char         *str;
	unsigned int  nr;
	int           err = 0;

	nr = strtoul(arg, &str, 0);
	if (*str)
		err = EINVAL;
	else if (!nr)
		err = ERANGE;

	if (err) {
		fprintf(stderr, "Invalid number of loops specified: %s\n",
		        strerror(err));
		return EXIT_FAILURE;
	}

	*loop_nr = nr;

	return EXIT_SUCCESS;
}

int
pt_parse_sched_prio(const char *arg, int *priority)
{
	char *str;
	int   prio;
	int   err = 0;

	prio = (int)strtoul(arg, &str, 0);
	if (*str)
		err = EINVAL;
	else if ((prio < sched_get_priority_min(SCHED_FIFO)) ||
	         (prio > sched_get_priority_max(SCHED_FIFO)))
		err = ERANGE;

	if (err) {
		fprintf(stderr, "Invalid scheduling priority specified: %s\n",
		        strerror(err));
		return EXIT_FAILURE;
	}

	*priority = prio;

	return EXIT_SUCCESS;
}

int
pt_setup_sched_prio(int priority)
{
	if (priority) {
		const struct sched_param parm = { .sched_priority = priority };

		if (sched_setscheduler(getpid(), SCHED_FIFO, &parm)) {
			perror("Failed set scheduling policy");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

void
pt_init_entry_iter(const struct pt_entries *entries)
{
	rewind(entries->pt_file);
}

int
pt_iter_entry(const struct pt_entries *entries, void *entry)
{
	if (fread(entry, sizeof(uint32_t), 1, entries->pt_file) == 1)
		return 0;

	if (feof(entries->pt_file))
		return -EPERM;

	fprintf(stderr, "Failed to fetch entry\n");

	return -EIO;
}

int
pt_open_entries(const char *pathname, struct pt_entries *entries)
{
	entries->pt_file = fopen(pathname, "r");
	if (!entries->pt_file) {
		fprintf(stderr, "Failed to open file %s: %s\n", pathname,
		        strerror(errno));
		return EXIT_FAILURE;
	}

	if (fseek(entries->pt_file, 0, SEEK_END)) {
		perror("Failed to probe file end");
		return EXIT_FAILURE;
	}

	entries->pt_nr = (int)ftell(entries->pt_file);
	if (entries->pt_nr < 0) {
		perror("Failed to probe file size");
		return EXIT_FAILURE;
	}

	entries->pt_nr /= (int)sizeof(uint32_t);
	if (entries->pt_nr <= 0) {
		fprintf(stderr, "Invalid file content\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void
pt_close_entries(const struct pt_entries *entries)
{
	fclose(entries->pt_file);
}
