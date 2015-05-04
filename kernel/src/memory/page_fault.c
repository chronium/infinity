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

#include <stdint.h>
#include <infinity/arch/pic.h>
#include <infinity/memmanager.h>
#include <infinity/sched.h>

void page_fault_handler(struct regs *regs)
{
    caddr_t fault;
    
	asm volatile ("mov %%cr2, %0" : "=r" (fault));
    
    if(fault >= current_proc->p_mstart && fault < current_proc->p_mbreak) {
        frame_alloc(fault & 0xFFFFF000, PAGE_USER | PAGE_RW);
       // return;
    }
    panic_cpu(regs, "Page fault at 0x%x", fault);
}

