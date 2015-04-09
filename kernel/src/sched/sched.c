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
 * Provides functions for scheduling threads and context switching
 */

#include <infinity/common.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/sched.h>
#include <infinity/memmanager.h>
#include <infinity/paging.h>
#include <infinity/portio.h>
#include <infinity/event.h>
#include <infinity/elf32.h>


static int pid_next = 0;

void *next_page_directory = NULL;
extern struct page_directory *kernel_directory;
extern struct page_directory *current_directory;
    
struct process *proc_list = NULL;
struct process *current_proc = NULL;

static int *init_stack(struct page_directory *pdir, int *stack, int eip);
static void add_process(struct process *new_proc);
static void remove_process(struct process *proc);
static void free_process();
static struct process *sched_get_next(); 
static struct process *get_process(pid_t id);

void init_sched(void *callback)
{
    struct process *kproc = (struct process*)kalloc(sizeof(struct process));
    kproc->p_id = pid_next++;
    
    struct page_directory *ndir = (struct page_directory*)malloc_pa(sizeof(struct page_directory));

    memcpy(ndir, kernel_directory, sizeof(struct page_directory));
    
    switch_page_directory(ndir);
    
    for(int i = 0x4000000; i <= 0x4020000; i += 0x1000) {
        frame_alloc_d(ndir, (void*)i, PAGE_USER | PAGE_RW);
    }
    
    kproc->p_pdir = ndir;
    current_proc = kproc;
    kproc->p_esp = init_stack(ndir, (int*)0x4020000, callback);
    
    current_proc = NULL;
    add_process(kproc);
    
    while(1) {
        thread_yield();
    }
}

/*
 * Peforms a context switch, changing the current
 * process
 * @param esp       The previous stack pointer
 */
int perform_context_switch(int esp)
{
    BEGIN_CRITICAL_REGION;
    if (proc_list) {
        
        if (current_proc) {
            current_proc->p_esp = esp;
        }
        
        current_proc = sched_get_next();
        
        if(current_proc->p_kill) {
            if(--current_proc->p_ttl == 0) {
                switch_page_directory(proc_list->p_pdir);
                free_process();
                asm("sti");
                asm("hlt");
            }
            current_proc = sched_get_next();
        }
        
        next_page_directory = (int32_t)&current_proc->p_pdir->tables_physical;
   
        END_CRITICAL_REGION;
        return current_proc->p_esp;
    }

    END_CRITICAL_REGION;
    
    return esp;
}

/*
 * Spawns a new process, specified by path. The executable
 * will be loaded into an new virtual address space, and 
 * a new stack will be setup
 * @param mode      The mode to spawn the process in
 * @param path      The path containing the process's
 *                  main executable.
 * @param argv      Command line arguments to pass
 * @param envp      Environmental variables to pass
 * 
 */
int spawnve(int mode, char *path, char **argv, char **envp)
{
    BEGIN_CRITICAL_REGION;

    static char *s_path;
    static void *elf;
    s_path = path;
    
    struct process *nproc = (struct process*)kalloc(sizeof(struct process));
    nproc->p_id = pid_next++;
    
    struct page_directory *ndir = (struct page_directory*)malloc_pa(sizeof(struct page_directory));
    
    memcpy(ndir, kernel_directory, sizeof(struct page_directory));

    for(int i = 0x4000000; i <= 0x4020000; i += 0x1000) {
        frame_alloc_d(ndir, (void*)i, PAGE_USER | PAGE_RW);
    }
    
    add_process(nproc);
    
    elf = elf_open_v(ndir, path);
    
    struct elf32_ehdr *hdr = (struct elf32_ehdr*)elf;

    nproc->p_esp = init_stack(ndir, (int*)0x4020000, hdr->e_entry);
    nproc->p_pdir = ndir;
    

    END_CRITICAL_REGION;
    
    thread_yield();
    
}

/*
 * Wait's for a process to exit
 * @param id            The ID of the process to wait for
 * @param status        A pointer to write the exit status
 */
pid_t waitpid(pid_t id, int *status)
{
    struct process *proc = get_process(id);
    
    if(proc) {
        while(proc->p_kill == 0) {
            thread_yield();
        }
        status = proc->p_status;
        return id;
    } else {
        return -1;
    }
}

/*
 * Exits, destroying the process and returning a
 * status.
 * @param status        The status to return
 */
void exit(int status)
{
    current_proc->p_kill = 1;
    current_proc->p_ttl = 2;
    current_proc->p_status = status;
    asm("sti");
    while(1) {
        thread_yield();
    }
}

/*
 * Gets the process ID number for this process
 */ 
pid_t getpid()
{
    return current_proc->p_id;
}


/*
 * Gets the user ID number for this process
 */
uid_t getuid()
{
    return current_proc->p_uid;
}

/*
 * Free all allocated frames and close all open
 * file descriptors
 */
static void free_process()
{
    frame_free_all(current_proc->p_pdir);
    
    struct fildes *i = current_proc->p_fildes_table;
    while(i) {
        int fd = i->fd_num;
        i = i->next;
        close(fd);
    }
    
    remove_process(current_proc);
	outb(0x20, 0x20);
}

/*
 * Initialize a processes stack
 */
static int *init_stack(struct page_directory *pdir, int *stack, int eip)
{
    /*
     * I feel like I need to explain myself, since this will
     * need to write to a different virtual address space, we
     * must switch into it. Doing that will mean that EBP will
     * no longer be valid, as it points to memory in this address
     * space, not pdir's. With that said, the following locals 
     * must be static (Meaning they will be in the identity mapped
     * portion of kernel memory, and not relative to EBP on the stack)
     */
     
    static int *s_stack;
    static int s_eip;
    static int s_ebp;
    
    s_eip = eip;
    s_stack = stack;
    s_ebp = stack + 60;
    
	asm volatile ("mov %0, %%cr3" :: "r" (&pdir->tables_physical));
	asm volatile ("mov %cr3, %eax; mov %eax, %cr3;");
    
    *--s_stack = 0x202;       
    *--s_stack = 0x08;        
    *--s_stack = s_eip;

    *--s_stack = 0;           
    *--s_stack = 0;   
    *--s_stack = 0;           
    *--s_stack = 0;           
    *--s_stack = 0;         
    *--s_stack = 0;          
    *--s_stack = 0;           
    *--s_stack = 0;           

    *--s_stack = 0x10;        
    *--s_stack = 0x10;        
    *--s_stack = 0x10;        
    *--s_stack = 0x10;        


	asm volatile ("mov %0, %%cr3" :: "r" (&current_directory->tables_physical));
	asm volatile ("mov %cr3, %eax; mov %eax, %cr3;");
    
    return s_stack;
}


/*
 * Allocates a thread placing it on the queue
 */
static void add_process(struct process *new_proc)
{
    new_proc->next = NULL;

    struct process *t = proc_list;

    if (proc_list) {
        struct process *i = proc_list;
        while (i->next)
            i = i->next;
        i->next = new_proc;
    } else {
        proc_list = new_proc;
    }
    
    event_dispatch(PROCESS_CREATE, new_proc);
}

/*
 * Removes a process 
 */
static void remove_process(struct process *proc)
{
    struct process *i = proc_list;
    
    event_dispatch(PROCESS_DESTROY, proc);
    
    if(i == proc) {
        panic("Attempted to kill idle process!");
    } else {
        struct process *last = NULL;
        while(i) {
            if(i == proc) {
                last->next = proc->next;
                break;
            }
            last = i;
            i = i->next;
        }
    }
    
    kfree(proc);
    
}

/*
 * Gets the next thread in the queue to be executed
 */
static struct process *sched_get_next()
{
    struct process *i = proc_list;

    while (i) {
        if (i == current_proc) {
            if (i->next)
                return i->next;
            else
                return proc_list;
        }
        i = i->next;
    }
    return proc_list;
}

/*
 * Finds a process based on its PID
 */
static struct process *get_process(pid_t id)
{
    struct process *i = proc_list;

    while (i) {
        if (i->p_id == id) {
            return i;
        }
        i = i->next;
    }
    return NULL;
}

/*
 * Gives up resource time and switches to the next thread
 */
void thread_yield()
{
    asm ("hlt");
}
