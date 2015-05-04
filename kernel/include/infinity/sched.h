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
#include <infinity/arch/pic.h>
#include <infinity/fs.h>
#include <infinity/signal.h>
#include <infinity/paging.h>

#define BEGIN_CRITICAL_REGION   asm ("cli");
#define END_CRITICAL_REGION     asm ("sti");

#define P_WAIT      0        
#define P_NOWAIT    1
#define P_DETACH    4
#define P_SUSPEND   8
#define P_CHILD     16

struct process {
    char                    p_name[64];
    pid_t                   p_id;
    uid_t                   p_uid;
    gid_t                   p_gid;
    char                    p_kill;
    int                     p_ttl;
    int                     p_status;
    int                     p_nextfd;
    char                    p_wd[512];
    void *                  p_mstart;
    void *                  p_mbreak;
    void *                  p_esp;
    struct page_directory * p_pdir;
    struct fildes *         p_fildes_table;
    struct process *        next;
};

struct task {
    void            (*task_handler) (void *data);
    void *          data;
    struct task *   next;
};

extern struct process *current_proc;

void init_sched(void *callback, void *args);
int spawnve(int mode, char *path, char **argv, char **envp);
void thread_yield();
void *sbrk(int delta);
void exit(int status);
pid_t waitpid(pid_t id, int *status);
pid_t getpid();
uid_t getuid();
uid_t getgid();
int setuid(uid_t uid);
int setgid(gid_t gid);
char *getwd(char *buf);
int setwd(char *buf);
pid_t fork();
struct process *get_process(pid_t id);
#endif
