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

/*
 * sched.c
 * The scheduler, handles multitasking and process
 * creation
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <infinity/common.h>
#include <infinity/interrupt.h>
#include <infinity/types.h>
#include <infinity/kheap.h>
#include <infinity/paging.h>
#include <infinity/process.h>
#include <infinity/kernel.h>
#include "pit.h"

extern page_directory_t* current_directory;

process_t* current_process = NULL;

static process_t* proc_queue;

static char scheduler_enabled = 1;

void init_sched()
{
	process_t* nproc = kalloc(sizeof(process_t));
	nproc->next_proc = NULL;
	nproc->pid = 0;
	nproc->image.stack_base = malloc_pa(0x10000);
	nproc->register_context.useresp = nproc->image.stack_base +0x10000;
	nproc->image.page_directory = current_directory;
	set_kernel_stack(kalloc(0xA0000) + 0xA0000);
	proc_queue = nproc;
	current_process = nproc;
	init_pit(100);
	asm("sti");
	for(int i = 0; i < 10; i++)
	asm("hlt");
	
}


void perform_context_switch(registers_t* state)
{
	static int first_switch = 1;
	if(scheduler_enabled)
	{
		memcpy(&current_process->register_context, state, sizeof(registers_t));
		current_process->image.page_directory = current_directory;
		current_process = current_process->next_proc;
		if(!current_process)
			current_process = proc_queue;
		if(first_switch)
		{
			current_process->image.stack_base = state->esp - 0x1000;
			first_switch = 0;
		}
		memcpy(state, &current_process->register_context, sizeof(registers_t));

		switch_page_directory(current_process->image.page_directory);
	}
}

void schedule_process(process_t* proc)
{
	process_t* plist = proc_queue;
	proc->image.kernel_stack = kalloc(0x10000);
	proc->register_context.esp = proc->image.kernel_stack + 0x10000;
	while(plist->next_proc != NULL) plist = plist->next_proc;
	plist->next_proc = proc;
}

void scheduler_enable()
{
	scheduler_enabled = 1;
}

void scheduler_disable()
{
	scheduler_enabled = 0;
}
