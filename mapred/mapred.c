/******************************************************************************
 * This file is part of Mapred
 *
 * Copyright (C) 2017 Grégor Boirie <gregor.boirie@free.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "token.h"
#include "work.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define MAPRED_TASK_COUNT_MAX (256U)

/******************************************************************************
 * File handling.
 ******************************************************************************/

struct mapred_file {
	const char *fl_data;
	size_t      fl_size;
	int         fl_fd;
};

static int
mapred_open_file(struct mapred_file *file, const char *file_path)
{
	struct stat st;
	int         err;

	file->fl_fd = open(file_path, O_RDONLY);
	if (file->fl_fd < 0)
		return -errno;

	if (fstat(file->fl_fd, &st))
		goto close;

	file->fl_size = (size_t)st.st_size;

	file->fl_data = mmap(NULL, file->fl_size, PROT_READ, MAP_PRIVATE,
	                     file->fl_fd, 0);

	if (file->fl_data == MAP_FAILED)
		goto close;

	return 0;

close:
	err = -errno;
	close(file->fl_fd);
	return err;
}

static void
mapred_close_file(const struct mapred_file *file)
{
	munmap((void *)file->fl_data, file->fl_size);
	close(file->fl_fd);
}

/******************************************************************************
 * Main and options / arguments parsing.
 ******************************************************************************/

static int
mapred_run_multiple(const char *data, size_t size, unsigned int task_count)
{
	struct mapred_scheduler          sched;
	const struct mapred_token_store *store;
	int                              err;

	err = mapred_init_work_scheduler(&sched, task_count);
	if (err) {
		fprintf(stderr, "Failed to init scheduler: %s.\n",
		        strerror(-err));
		goto fini;
	}

	store = mapred_run_work_scheduler(&sched, data, size);
	if (!store) {
		fprintf(stderr, "Failed to run work scheduler: %s.\n",
		        strerror(errno));
		goto fini;
	}

	err = mapred_dump_token_store(store);
	if (err)
		fprintf(stderr, "Failed to dump scheduler tokens: %s.\n",
		        strerror(-err));

	mapred_free_scheduler_works(&sched);

fini:
	mapred_fini_work_scheduler(&sched);

	if (err)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int
mapred_run_single(const char *data, size_t size)
{
	struct mapred_token_store store;
	int                       err;

	err = mapred_init_token_store(&store);
	if (err) {
		fprintf(stderr, "Failed to init token store: %s.\n",
		        strerror(-err));
		return err;
	}

	err = mapred_tokenize(&store, data, size);
	if (err) {
		fprintf(stderr, "Failed to tokenize: %s.\n", strerror(-err));
		goto fini;
	}

	err = mapred_dump_token_store(&store);
	if (err)
		fprintf(stderr, "Failed to dump tokens: %s.\n", strerror(-err));


fini:
	mapred_fini_token_store(&store);

	if (err)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static void
usage(const char *me)
{
	fprintf(stderr, "Usage: %s -h FILE [NR]\n", me);
}

int main(int argc, char *argv[])
{
	struct mapred_file  file = { 0, };
	const char         *path;
	char               *errstr;
	unsigned long       count = 0;
	int                 err;

	while (true) {
		int                        opt;
		static const struct option lopts[] = {
			{"help",    0, NULL, 'h'},
			{0,         0, 0,    0}
		};

		opt = getopt_long(argc, argv, "h", lopts, NULL);
		if (opt < 0)
			/* No more options: go parsing positional arguments. */
			break;

		switch (opt) {
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
	case 1:
		break;

	case 2:
		count = strtoul(argv[2], &errstr, 0);
		if (*errstr)
			count = ULONG_MAX;
		break;

	default:
		fprintf(stderr, "Missing argument.\n");
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (count > MAPRED_TASK_COUNT_MAX) {
		fprintf(stderr, "Invalid number of workers (< %u)\n",
		        MAPRED_TASK_COUNT_MAX);
		return EXIT_FAILURE;
	}

	path = argv[1];

	err = mapred_open_file(&file, path);
	if (err) {
		fprintf(stderr, "Failed to open \"%s\" file: %s.\n",
		        argv[optind], strerror(-err));
		return EXIT_FAILURE;
	}

	if (count <= 1)
		err = mapred_run_single(file.fl_data, file.fl_size);
	else
		err = mapred_run_multiple(file.fl_data, file.fl_size, count);

	mapred_close_file(&file);

	return err;
}
