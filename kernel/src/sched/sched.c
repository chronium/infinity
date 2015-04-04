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
 * sched.c
 * Provides functions for scheduling threads and tcontext switching
 */

#include <infinity/common.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/sched.h>
#include <infinity/paging.h>

struct thread *thread_list = NULL;
struct thread *current_thread = NULL;

static void thread_alloc(struct thread *t);
static struct thread *thread_get_next();

void init_sched()
{
}


int perform_context_switch(int esp)
{
    BEGIN_CRITICAL_REGION;

    if (thread_list) {
        if (current_thread) {
            current_thread->t_esp = esp;
        }
        
        current_thread = thread_get_next();
        
        if(current_thread->t_proc) {
            switch_page_directory(current_thread->t_proc->p_pdir);
        }
        return current_thread->t_esp;
    }

    END_CRITICAL_REGION;
    
    return esp;
}

/*
 * Allocates a thread placing it on the queue
 */
static void thread_alloc(struct thread *new_thread)
{
    new_thread->next = NULL;

    struct thread *t = thread_list;

    if (thread_list) {
        struct thread *i = thread_list;
        while (i->next)
            i = i->next;
        i->next = new_thread;
    } else {
        thread_list = new_thread;
    }
}


/*
 * Gets the next thread in the queue to be executed
 */
static struct thread *thread_get_next()
{
    struct thread *i = thread_list;

    while (i) {
        if (i == current_thread) {
            if (i->next)
                return i->next;
            else
                return thread_list;
        }
        i = i->next;
    }
    return thread_list;
}

/*
 * Creates a thread
 * @param target        The entry point
 * @param args          Arguments to be passed to the entry point
 */
void thread_create(void *target, void *arg)
{
    BEGIN_CRITICAL_REGION;

    struct thread *t = (struct thread *)kalloc(sizeof(struct thread));

    int *stack = kalloc(0x20000) + 0x20000;
    
    *--stack = arg;
    *--stack = 0x202;
    *--stack = 0x08; 
    *--stack = (int)target;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;  
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0x10;
    *--stack = 0x10;
    *--stack = 0x10;
    *--stack = 0x10;
    
    t->t_esp = stack;
    
    thread_alloc(t);
    
    END_CRITICAL_REGION;
    
    thread_yield();
}

/*
 * Changes the current threads process
 * @param name  The name of the process
 */
void process_create(const char *name)
{
    BEGIN_CRITICAL_REGION;
    
    struct process *proc = (struct process*)kalloc(sizeof(struct process));
    proc->p_id = current_thread->t_id;
    strcpy(proc->p_name, name);
    extern struct page_directory *kernel_directory;
    struct page_directory *pdir = (struct page_directory*)malloc_pa(sizeof(struct page_directory));
    memcpy(pdir, kernel_directory, sizeof(struct page_directory));

    proc->p_pdir = pdir;
    current_thread->t_proc = proc;
    
    END_CRITICAL_REGION;
    
    thread_yield();
}

/*
 * Gives up resource time and switches to the next thread
 */
void thread_yield()
{
    asm ("hlt");
}
