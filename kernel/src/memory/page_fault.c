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
 * page_fault.c
 * Handles page faults, this actually does alot more than
 * that (Demand paging, ect)
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/interrupt.h> 
#include <infinity/types.h>
#include <infinity/process.h>
#include <infinity/sched.h>
#include <infinity/kernel.h>


extern process_t* current_process;

void page_fault_handler(registers_t* regs)
{
	caddr_t fault;
	asm volatile("mov %%cr2, %0" : "=r" (fault)); 
	
	if(current_process)
	{
		/*
		 * Relax! No need to fear, this address belongs to the process
		 * but it just hasn't been paged so we need to allocate the page
		 * and return
		 */
		if(fault < current_process->image.image_brk && fault >= current_process->image.image_base)
		{
			frame_alloc(fault & 0xFFFFF000, 2);
			return;
		}
	}

	panic("page fault at 0x%p", fault);
}
