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
 * @file      task.h
 * @author    Grégor Boirie <gregor.boirie@free.fr>
 * @date      29 May 2017
 * @copyright GNU Public License v3
 *
 * Task interface.
 *
 * @defgroup  Task
 */
#ifndef _TASK_H
#define _TASK_H

#include "utils.h"
#include <pthread.h>

struct mapred_operation;

/**
 * Operation function protoype.
 *
 * Returned error code influences further task behaviour.
 *
 * @retval -EAGAIN keep executing subsequent operations
 * @retval 0 exit
 *
 * @see mapred_operation
 *
 * @ingroup Task
 */
typedef int (mapred_process_fn)(struct mapred_operation *);

/**
 * Operation.
 *
 * An operation is the primary inter-task communication object. A producer task
 * enqueues operations into arbitrary queue(s). A consumer task dequeues
 * operations from its attached queue and run them one after the other untill an
 * operation execution instruct the task to exit.
 *
 * @see mapred_task
 * @see mapred_task_queue
 *
 * @ingroup Task
 */
struct mapred_operation {
	/* Task call this function in order to run the operation. */
	mapred_process_fn *process;
};

/**
 * Maximum number of operations that can sit into a task queue.
 *
 * @see mapred_operation
 * @see mapred_task
 * @see mapred_task_queue
 *
 * @ingroup Task
 */
#define MAPRED_TASK_QUEUE_COUNT_MAX (32U)

/**
 * Task queue.
 *
 * Tasks use queues to exchange mapred_operation between themselves. Typical
 * usage is when a task wants to hand an asynchronous operation to another
 * thread for parallel or differed execution purpose.
 *
 * @see mapred_task
 *
 * @ingroup Task
 */
struct mapred_task_queue {
	/** Mutex ensuring consistent concurrent accesses. */
	pthread_mutex_t          tsk_lock;
	/** Indicate queue is being drained by a consumer task. */
	pthread_cond_t           tsk_drain;
	/** Indicate queue is being filled by a producer task. */
	pthread_cond_t           tsk_fill;
	/** Queue head index (consumer side). */
	unsigned int             tsk_head;
	/** Current count of operations sitting in the queue. */
	unsigned int             tsk_count;
	/** Queue data, i.e. operations. */
	struct mapred_operation *tsk_operations[MAPRED_TASK_QUEUE_COUNT_MAX];
};

/**
 * Enqueue an operation to a task queue.
 *
 * @param queue     Task queue to enqueue operation into.
 * @param operation Operation to enqueue.
 *
 * A task waiting on this queue will pop this operation out and execute it.
 *
 * @ingroup Task
 */
extern void mapred_nqueue_task_operation(struct mapred_task_queue *queue,
                                         struct mapred_operation  *operation);

/**
 * Dequeue an operation from a task queue.
 *
 * @param queue Task queue to dequeue operation from.
 *
 * @return A task operation (cannot fail).
 *
 * @ingroup Task
 */
extern struct mapred_operation *
mapred_dqueue_task_operation(struct mapred_task_queue *queue);

/**
 * Initialize a task queue.
 *
 * @param queue Queue to init.
 *
 * @return An errno like error code if failure, 0 otherwise.
 *
 * @ingroup Task
 */
extern int mapred_init_task_queue(struct mapred_task_queue *queue);

/**
 * Finish a task queue.
 *
 * @param queue Queue to finish.
 *
 * Will free resources queue allocated at init time.
 *
 * @ingroup Task
 */
extern void mapred_fini_task_queue(struct mapred_task_queue *queue);

/**
 * Task.
 *
 * Simple wrapper around pthread with queue for inter-task collaborations.
 *
 * A task basically waits for an operation on its queue. Each time an operation
 * is dequeued, task runs it. Depending on return code, task go back polling for
 * new operations or simply exits.
 *
 * @see mapred_operation
 *
 * @ingroup Task
 */
struct mapred_task {
	/** Attached queue. */
	struct mapred_task_queue *tsk_queue;
	/** Underlying pthread. */
	pthread_t                 tsk_thread;
};

/**
 * Initialize a task.
 *
 * @param task  The task to init.
 * @param queue Queue to attach to this task.
 *
 * This will spawn the underlying pthread which will block waiting on queue
 * input, i.e. waiting for operations to run.
 *
 * @return An errno like error code if failure, 0 otherwise.
 *
 * @ingroup Task
 */
extern int mapred_init_task(struct mapred_task *task,
                            struct mapred_task_queue *queue);

/**
 * Finish a task.
 *
 * @param task The task to finish.
 *
 * @note For now, this is simply a dummy function since tasks are spawned in
 * detached state.
 *
 * @ingroup Task
 */
static inline void
mapred_fini_task(struct mapred_task *task __unused)
{
}

#endif
