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
#include <mboot.h>
#include <infinity/idt.h>
#include <infinity/gdt.h>
#include <infinity/interrupt.h>
#include <infinity/device.h>
#include <infinity/kheap.h>
#include <infinity/paging.h>
#include <infinity/textscreen.h>

extern device_t textscreen_device;

void kmain(multiboot_info_t* mbootinfo)
{
	init_textscreen();
	init_gdt();
	init_idt();
	init_kheap(*(uint32_t*)(mbootinfo->mods_addr+4));
	init_paging();
	init_sched();
	int test = fork();
	if(test)
	{
		printk("I'm the child");
	}
	else
	{
		printk("I'm the parent thread");
	}
	while(1);
}

