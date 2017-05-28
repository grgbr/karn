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
/**
 * @file      task.c
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 May 2017
 * @copyright GNU Public License v3
 *
 * Task implementation.
 */
#include "task.h"
#include <stdlib.h>
#include <errno.h>

/* Mostly to ease debugging and system admin. */
#define MAPRED_TASK_NAME "worker"

/* Ensure FIFO state is consistent. */
#define mapred_assert_task_queue(_queue)                      \
	mapred_assert((_queue)->tsk_head <                    \
	              array_count((_queue)->tsk_operations)); \
	mapred_assert((_queue)->tsk_count <                   \
	              array_count((_queue)->tsk_operations))

void
mapred_nqueue_task_operation(struct mapred_task_queue *queue,
                             struct mapred_operation  *operation)
{
	mapred_assert(queue);
	mapred_assert(operation);

	unsigned int slot;

	/*
	 * No need to check for error code since we use non recursive mutex with
	 * no particular priority attribute.
	 */
	pthread_mutex_lock(&queue->tsk_lock);

	mapred_assert_task_queue(queue);

	while (queue->tsk_count >= array_count(queue->tsk_operations))
		/*
		 * Queue is full: wait for drain event, i.e untill a consumer
		 * has dequeued an operation.
		 *
		 * No need to check for error code since as we don't rely upon
		 * cancelation point usage, failure conditions would be caused
		 * by programmatic errors only (might worth wrapping with an
		 * assertion though...).
		 */
		pthread_cond_wait(&queue->tsk_drain, &queue->tsk_lock);

	/* Store operation into queue in a FIFO manner. */
	slot = (queue->tsk_head + queue->tsk_count) %
	       array_count(queue->tsk_operations);
	queue->tsk_operations[slot] = operation;
	queue->tsk_count++;

	pthread_mutex_unlock(&queue->tsk_lock);

	/*
	 * Notify consumer task blocking on this queue for available operations
	 * to run. Call this outside of critical section as there is no need for
	 * particular determinism.
	 */
	pthread_cond_signal(&queue->tsk_fill);
}

struct mapred_operation *
mapred_dqueue_task_operation(struct mapred_task_queue *queue)
{
	mapred_assert(queue);

	struct mapred_operation *op;

	/*
	 * No need to check for error code since we use non recursive mutex with
	 * no particular priority attribute.
	 */
	pthread_mutex_lock(&queue->tsk_lock);

	mapred_assert_task_queue(queue);

	while (!queue->tsk_count)
		/*
		 * Queue is empty: wait for fill event, i.e untill a producer
		 * has enqueued an operation.
		 *
		 * No need to check for error code since as we don't rely upon
		 * cancelation point usage, failure conditions would be caused
		 * by programmatic errors only (might worth wrapping with an
		 * assertion though...).
		 */
		pthread_cond_wait(&queue->tsk_fill, &queue->tsk_lock);

	/* Dequeue operation in a FIFO manner. */
	op = queue->tsk_operations[queue->tsk_head];
	queue->tsk_head = (queue->tsk_head + 1) %
	                  array_count(queue->tsk_operations);
	queue->tsk_count--;

	pthread_mutex_unlock(&queue->tsk_lock);

	/*
	 * Notify producer task blocking on this queue for available free slots.
	 * Call this outside of critical section as there is no need for
	 * particular determinism.
	 */
	pthread_cond_signal(&queue->tsk_drain);

	mapred_assert(op);
	return op;
}

int
mapred_init_task_queue(struct mapred_task_queue *queue)
{
	mapred_assert(queue);

	int err;

	err = pthread_mutex_init(&queue->tsk_lock, NULL);
	if (err)
		return -err;

	err = pthread_cond_init(&queue->tsk_drain, NULL);
	if (err)
		goto lock;

	err = pthread_cond_init(&queue->tsk_fill, NULL);
	if (err)
		goto drain;

	queue->tsk_head = 0;
	queue->tsk_count = 0;

	return 0;

drain:
	pthread_cond_destroy(&queue->tsk_drain);
lock:
	pthread_mutex_destroy(&queue->tsk_lock);
	return -err;
}

void
mapred_fini_task_queue(struct mapred_task_queue *queue)
{
	mapred_assert(queue);
	mapred_assert_task_queue(queue);

	pthread_cond_destroy(&queue->tsk_drain);
	pthread_cond_destroy(&queue->tsk_fill);
	pthread_mutex_destroy(&queue->tsk_lock);
}

/*
 * Main task / pthread entry point.
 *
 * Basically keep dequeueing operations from task queue and run them untill
 * required to exit.
 */
static void *
mapred_process_task(void *task)
{
	mapred_assert(task);

	struct mapred_task *tsk = task;
	int                 err;

	/* Set task name. */
	pthread_setname_np(tsk->tsk_thread, MAPRED_TASK_NAME);

	do {
		struct mapred_operation *op;

		/* Dequeue. */
		op = mapred_dqueue_task_operation(tsk->tsk_queue);

		mapred_assert(op);
		mapred_assert(op->process);

		/* Run. */
		err = op->process(op);
	} while (err == -EAGAIN);

	return NULL;
}

int
mapred_init_task(struct mapred_task *task, struct mapred_task_queue *queue)
{
	mapred_assert(task);
	mapred_assert(queue);
	mapred_assert_task_queue(queue);

	pthread_attr_t attr;
	int            err;

	err = pthread_attr_init(&attr);
	if (err)
		return -err;

	/*
	 * Create thread in detached state as we rely upon the operations
	 * exchange mechanism to require task / thread exit.
	 */
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	task->tsk_queue = queue;

	err = pthread_create(&task->tsk_thread, &attr, mapred_process_task,
	                     task);

	pthread_attr_destroy(&attr);

	return -err;
}
