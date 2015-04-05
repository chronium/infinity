/* Copyright (C) 2014 - GruntTheDivine (Sloan Crandell)
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

#ifndef INFINITY_SCHED_H
#define INFINITY_SCHED_H

#include <stddef.h>
#include <infinity/types.h>
#include <infinity/interrupt.h>
#include <infinity/virtfs.h>
#include <infinity/paging.h>

#define BEGIN_CRITICAL_REGION   asm ("cli");
#define END_CRITICAL_REGION     asm ("sti");

struct process {
    char                    p_name[64];
    pid_t                   p_id;
    uid_t                   p_uid;
    gid_t                   p_gid;
    int                     p_nextfd;
    struct page_directory * p_pdir;
    struct fildes *         p_fildes_table;
};

struct thread {
    pid_t           t_id;
    void *          t_esp;
    struct process *t_proc;
    struct thread * next;
};

struct task {
    void            (*task_handler) (void *data);
    void *          data;
    struct task *   next;
};

extern struct process *current_proc;

void init_sched();
void schedule_task(struct task *task);
void thread_create(void *target, void *arg);
void thread_yield();
void process_create(const char *name);

pid_t getpid();
uid_t getuid();

#endif
