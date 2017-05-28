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

#include "work.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/******************************************************************************
 * Work unit
 ******************************************************************************/

struct mapred_work {
	struct mapred_operation    wk_operation;
	/** Main token store. */
	struct mapred_token_store  wk_tokens;
	const char                *wk_data;
	size_t                     wk_size;
	struct mapred_token_store *wk_tomerge;
	struct mapred_task_queue  *wk_results;
	int                        wk_status;
};

/*
 * Process map work unit.
 * Executed in task context, i.e. threaded with respect to scheduler task...
 */
static int
mapred_map(struct mapred_operation *operation)
{
	/* Work "inherits" from operation. */
	struct mapred_work *wk = (struct mapred_work *)operation;

	mapred_assert(wk->wk_data);
	mapred_assert(wk->wk_size > 0);
	mapred_assert(wk->wk_results);

	/*
	 * Perform the map phase over specified memory area and produce results
	 * into main work unit token store.
	 */
	wk->wk_status = mapred_tokenize(&wk->wk_tokens, wk->wk_data,
	                                wk->wk_size);

	/*
	 * Enqueue received work as result (waking up scheduler task): scheduler
	 * task will look at the status assigned above to determine success of
	 * failure condition.
	 */
	mapred_nqueue_task_operation(wk->wk_results, operation);

	return -EAGAIN;
}

/*
 * Initialize a map work unit.
 * See mapred_map() above.
 */
static int
mapred_init_map_work(struct mapred_work       *work,
                     const char               *area,
                     size_t                    size,
                     struct mapred_task_queue *results)
{
	mapred_assert(work);
	mapred_assert(area);
	mapred_assert(size > 0);
	mapred_assert(results);

	int err;

	/* Initialize (empty) token store. */
	err = mapred_init_token_store(&work->wk_tokens);
	if (err)
		return err;

	/* Register work unit processor. */
	work->wk_operation.process = mapred_map;

	/* Register memory area to perform mapping upon. */
	work->wk_data = area;
	work->wk_size = size;

	/* Register results queue for work completion notification. */
	work->wk_results = results;

	return 0;
}

/*
 * Process reduce work unit.
 * Executed in task context, i.e. threaded with respect to scheduler task...
 */
static int
mapred_reduce(struct mapred_operation *operation)
{
	struct mapred_work *wk = (struct mapred_work *)operation;

	mapred_assert(wk->wk_tomerge);
	mapred_assert(wk->wk_results);

	/*
	 * Perform the reduce phase over specified token store and produce
	 * results into main work unit token store.
	 */
	mapred_merge_token_store(&wk->wk_tokens, wk->wk_tomerge);

	wk->wk_status = 0;

	/* Enqueue received work as result (waking up scheduler task). */
	mapred_nqueue_task_operation(wk->wk_results, operation);

	return -EAGAIN;
}

/*
 * Initialize a reduce work unit.
 * See mapred_reduce() above.
 */
static void
mapred_init_reduce_work(struct mapred_work        *work,
                        struct mapred_token_store *tokens,
                        struct mapred_task_queue  *results)
{
	mapred_assert(work);
	mapred_assert(tokens);
	mapred_assert(results);

	/* Register work unit processor. */
	work->wk_operation.process = mapred_reduce;

	/*
	 * Register token store to perform reduce over. Results will be stored
	 * into this work unit's main token store.
	 */
	work->wk_tomerge = tokens;

	/* Register results queue for work completion notification. */
	work->wk_results = results;
}

/*
 * Process exit work unit.
 * Executed in task context, i.e. threaded with respect to scheduler task...
 */
static int
mapred_exit_task(struct mapred_operation *operation)
{
	struct mapred_work *wk = (struct mapred_work *)operation;

	/*
	 * Reuse posted operation to indicate scheduler exit request
	 * has been seen.
	 */
	mapred_nqueue_task_operation(wk->wk_results, operation);

	/* Return 0 to instruct task to exit. */
	return 0;
}

/*
 * Initialize an exit work unit.
 * See mapred_exit_task() above.
 */
static void
mapred_init_exit_work(struct mapred_work       *work,
                      struct mapred_task_queue *results)
{
	/* Register work unit processor. */
	work->wk_operation.process = mapred_exit_task;

	/* Register results queue for work completion notification. */
	work->wk_results = results;
}

static void
mapred_fini_work(struct mapred_work *work)
{
	mapred_fini_token_store(&work->wk_tokens);
}

/******************************************************************************
 * Work unit scheduler
 ******************************************************************************/

static void
mapred_schedule_exit_work(struct mapred_scheduler *scheduler,
                          struct mapred_work      *work)
{
	/* Setup task exit request. */
	mapred_init_exit_work(work, &scheduler->sch_results);

	/* Post task exit request. */
	mapred_nqueue_task_operation(&scheduler->sch_commands,
	                             &work->wk_operation);
}

static void
mapred_process_tasks_exit(struct mapred_scheduler *scheduler,
                          unsigned int             count)
{
	unsigned int c;

	/* Wait for each task to acknowledge their respective exit request. */
	for (c = 0; c < count; c++)
		mapred_dqueue_task_operation(&scheduler->sch_results);

	/* And free resources allocated for each task. */
	for (c = 0; c < count; c++)
		mapred_fini_task(&scheduler->sch_tasks[c]);
}

static size_t
mapred_adjust_area_size(const char *data, size_t size)
{
	if (mapred_ischar_delim(data[size - 1]))
		return size;

	return size - mapred_backward_token_len(data, size);
}

static int
mapred_schedule_map_works(struct mapred_scheduler *scheduler,
                          const char              *data,
                          size_t                   size)
{
	unsigned int count = scheduler->sch_count;
	unsigned int c;
	size_t       sz  = size / count;
	int          err;

	/* Spawn tasks meant to process work units. */
	for (c = 0; c < count; c++) {
		err = mapred_init_task(&scheduler->sch_tasks[c],
		                       &scheduler->sch_commands);
		if (err) {
			count = c;
			goto task;
		}
	}

	/* Prepare map work units. */
	err = 0;
	for (c = 0; c < (count - 1); c++) {
		size_t bytes = mapred_adjust_area_size(data, sz);

		err = mapred_init_map_work(&scheduler->sch_works[c], data,
		                           bytes, &scheduler->sch_results);
		if (err) {
			count = c;
			goto work;
		}

		data += bytes;
		size -= bytes;
	}

	err = mapred_init_map_work(&scheduler->sch_works[c], data, size,
	                           &scheduler->sch_results);
	if (err) {
		count = scheduler->sch_count;
		goto work;
	}

	/* Now submit them to tasks. */
	for (c = 0; c < count; c++)
		mapred_nqueue_task_operation(
			&scheduler->sch_commands,
			&scheduler->sch_works[c].wk_operation);

	return 0;

work:
	for (c = 0; c < count; c++)
		mapred_fini_work(&scheduler->sch_works[c]);
task:
	for (c = 0; c < count; c++)
		mapred_schedule_exit_work(scheduler, &scheduler->sch_works[c]);
	mapred_process_tasks_exit(scheduler, count);

	return err;
}

static const struct mapred_token_store *
mapred_process_reduce_works(struct mapred_scheduler *scheduler)
{
	unsigned int        count = scheduler->sch_count - 1;
	unsigned int        c;
	struct mapred_work *res_wk;
	struct mapred_work *src_wk;

	while (count > 1) {
		res_wk = (struct mapred_work *)
		         mapred_dqueue_task_operation(&scheduler->sch_results);

		src_wk = (struct mapred_work *)
		         mapred_dqueue_task_operation(&scheduler->sch_results);

		/* Post a new reduce work. */
		mapred_init_reduce_work(res_wk, &src_wk->wk_tokens,
		                        &scheduler->sch_results);
		mapred_nqueue_task_operation(&scheduler->sch_commands,
		                             &res_wk->wk_operation);

		count--;
	}

	/*
	 * Retrieve last 2 results to run last reduce phase within scheduler
	 * task. This allows parallelisation with tasks exit management in a
	 * simple mannner.
	 */
	res_wk = (struct mapred_work *)
	         mapred_dqueue_task_operation(&scheduler->sch_results);
	src_wk = (struct mapred_work *)
	         mapred_dqueue_task_operation(&scheduler->sch_results);

	/* Post exit request to each tasks. */
	count = scheduler->sch_count;
	for (c = 0; c < count; c++)
		mapred_schedule_exit_work(scheduler, &scheduler->sch_works[c]);

	/* Carry out last reduce phase. */
	mapred_merge_token_store(&res_wk->wk_tokens, &src_wk->wk_tokens);

	/* Exit all tasks. */
	mapred_process_tasks_exit(scheduler, count);

	return &res_wk->wk_tokens;
}

const struct mapred_token_store *
mapred_run_work_scheduler(struct mapred_scheduler *scheduler,
                          const char              *data,
                          size_t                   size)
{
	int err;

	err = mapred_schedule_map_works(scheduler, data, size);
	if (err) {
		errno = -err;
		return NULL;
	}

	return mapred_process_reduce_works(scheduler);
}

void
mapred_free_scheduler_works(struct mapred_scheduler *scheduler)
{
	unsigned int c;

	for (c = 0; c < scheduler->sch_count; c++)
		mapred_fini_work(&scheduler->sch_works[c]);
}

int
mapred_init_work_scheduler(struct mapred_scheduler *scheduler,
                           unsigned int             task_count)
{
	mapred_assert(scheduler);
	mapred_assert(task_count > 1);

	int err;

	err = mapred_init_task_queue(&scheduler->sch_commands);
	if (err)
		return err;

	err = mapred_init_task_queue(&scheduler->sch_results);
	if (err)
		goto cmd;

	err = -ENOMEM;

	scheduler->sch_works = malloc(task_count *
	                              sizeof(*scheduler->sch_works));
	if (!scheduler->sch_works)
		goto res;

	scheduler->sch_tasks = malloc(task_count *
	                              sizeof(*scheduler->sch_tasks));
	if (!scheduler->sch_tasks)
		goto wk;

	scheduler->sch_count = task_count;

	return 0;

wk:
	free(scheduler->sch_works);
res:
	mapred_fini_task_queue(&scheduler->sch_results);
cmd:
	mapred_fini_task_queue(&scheduler->sch_commands);

	return err;
}

void
mapred_fini_work_scheduler(struct mapred_scheduler *scheduler)
{
	free(scheduler->sch_tasks);
	free(scheduler->sch_works);
	mapred_fini_task_queue(&scheduler->sch_results);
	mapred_fini_task_queue(&scheduler->sch_commands);
}
