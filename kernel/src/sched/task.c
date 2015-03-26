/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
/*
 * task.c
 * Provides functions for scheduling tasks
 */
  
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/sched.h>

struct task *task_list = NULL;

/*
 * Schedules a task, it will be executed on
 * the next PIT tick. After that, the task will
 * need to be re-scheduled if it is to be used again. 
 */
void schedule_task(struct task *task)
{
	if(task_list == NULL) {
		task_list = task;
		task->next = NULL;
	} else {
		task->next = task_list;
		task_list = task;
	}
}

/*
 * Executes and deqeues the last task in 
 * the queue
 */
void dequeue_next_task()
{
	if(task_list) {
		struct task *cur = task_list;
		cur->task_handler(cur->data);
		task_list = cur->next;
	}
}
