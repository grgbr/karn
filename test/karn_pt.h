#ifndef _KARN_PT_H
#define _KARN_PT_H

#include <stdio.h>
#include <time.h>

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
