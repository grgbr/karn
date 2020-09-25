#ifndef _KARN_PERF_H
#define _KARN_PERF_H

#include <stdio.h>
#include <time.h>

static inline int
pt_compare_min(const char *a, const char *b)
{
	unsigned int _a = *(unsigned int *)a;
	unsigned int _b = *(unsigned int *)b;

	if (_a < _b)
		return -1;
	else if (_a > _b)
		return 1;
	else
		return 0;
}

static inline int
pt_compare_max(const char *a, const char *b)
{
	return 0 - pt_compare_min(a, b);
}

static inline int
pt_qsort_compare(const void *a, const void *b)
{
	return pt_compare_min((char *)a, (char *)b);
}

static inline void
pt_copy_key(char *restrict dst, const char *restrict src)
{
	*(unsigned int *)dst = *(unsigned int *)src;
}

extern struct timespec pt_tspec_sub(const struct timespec *restrict a,
                                    const struct timespec *restrict b);

static inline unsigned long long
pt_tspec2ns(const struct timespec *tspec)
{
	return ((unsigned long long)tspec->tv_sec * 1000000000ULL) +
	       (unsigned long long)tspec->tv_nsec;
}

extern int pt_parse_loop_nr(const char *arg, unsigned int *loop_nr);

extern int pt_parse_sched_prio(const char *arg, int *priority);

extern int pt_setup_sched_prio(int priority);

struct pt_entries {
	FILE *pt_file;
	int   pt_nr;
};

extern void pt_init_entry_iter(const struct pt_entries *entries);

extern int pt_iter_entry(const struct pt_entries *entries, void *entry);

extern int pt_open_entries(const char *pathname, struct pt_entries *entries);

extern void pt_close_entries(const struct pt_entries *entries);

#endif
