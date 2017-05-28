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

#ifndef _WORK_H
#define _WORK_H

#include "task.h"

struct mapred_work;

struct mapred_scheduler {
	struct mapred_task_queue  sch_commands;
	struct mapred_task_queue  sch_results;
	struct mapred_work       *sch_works;
	struct mapred_task       *sch_tasks;
	unsigned int              sch_count;
};

extern const struct mapred_token_store *
mapred_run_work_scheduler(struct mapred_scheduler *scheduler,
                          const char              *data,
                          size_t                   size);

extern void mapred_free_scheduler_works(struct mapred_scheduler *scheduler);

extern int mapred_init_work_scheduler(struct mapred_scheduler *, unsigned int);

extern void mapred_fini_work_scheduler(struct mapred_scheduler *);

#endif
