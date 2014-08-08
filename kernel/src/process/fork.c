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

extern page_directory_t* current_directory;
extern process_t* current_process;
static uint32_t next_pid = 1;

static void create_stack(uint32_t org_stack, page_directory_t* dir)
{
	void* new_stack = malloc_pa(0x10000);
	memcpy(new_stack, org_stack, 0x10000);
	for(int i = 0; i < 0x10000; i += 0x1000)
		page_remap(dir, org_stack + i, new_stack + i);

}

pid_t fork()
{
	process_t* nproc = kalloc(sizeof(process_t));
	memcpy(nproc, current_process, sizeof(process_t));
	page_directory_t* pdir = (page_directory_t*)malloc_pa(sizeof(page_directory_t));
	memcpy(pdir, current_directory, sizeof(page_directory_t));
	
	create_stack(nproc->image.stack_base, pdir);
	
	nproc->image.page_directory = pdir;
	nproc->next_proc = NULL;
	nproc->register_context.eip = __builtin_return_address(0);
	nproc->register_context.eax = next_pid;
	nproc->pid = next_pid++;
	schedule_process(nproc);

	return 0;
}

