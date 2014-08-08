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
#include <stddef.h>
#include <stdbool.h>
#include <infinity/kheap.h>
#include <infinity/paging.h>
#include <infinity/memmanager.h>

#define IDENTITY_MAP_END		0x1E84800 


bool paging_enabled;

page_directory_t* current_directory;

static page_directory_t* kernel_directory;

void page_alloc(page_directory_t* dir, uint32_t vaddr, uint32_t paddr, bool write, bool user)
{
	vaddr /= 0x1000; 
	uint32_t i = vaddr / 1024;
	if(!dir->tables[i])
	{
		dir->tables[i] = (page_table_t*)malloc_pa(sizeof(page_table_t));
		for(int x = 0; x < 0x1000; x++)
			((char*)dir->tables[i])[x] = 0;
		dir->tables_physical[i] = (uint32_t)dir->tables[i] | 0x7; 
	}
	
	page_t* p = &dir->tables[i]->pages[vaddr % 1024];
	if(p->present)
		return;
	p->frame = paddr >> 12;
	p->present = 1;
	p->rw = write;
	p->user = user;
}
 
 
void page_remap(page_directory_t* dir, uint32_t vaddr, uint32_t paddr)
{
	vaddr /= 0x1000; 
	uint32_t i = vaddr / 1024;
	void* old_t = dir->tables[i];
	dir->tables[i] = (page_directory_t*)malloc_pa(sizeof(page_table_t));
	memcpy(old_t, dir->tables[i], sizeof(page_table_t));
	dir->tables_physical[i] = (uint32_t)dir->tables[i] | 0x7; 
	page_t* p = &dir->tables[i]->pages[vaddr % 1024];
	p->frame = paddr >> 12;
	
}

 
void page_free(page_directory_t* dir, uint32_t vaddr)
{
	vaddr /= 0x1000; 
	uint32_t i = vaddr / 1024;
	memset(&dir->tables[i]->pages[vaddr % 1024], 0, sizeof(page_t));
	
}

page_directory_t* create_new_page_directory()
{
	page_directory_t* dir = (page_directory_t*)malloc_pa(sizeof(page_directory_t));
	memset(dir, 0, sizeof(page_directory_t));
	
	for(uint32_t ptr = 0; ptr < IDENTITY_MAP_END; ptr += 0x1000)
	{
		page_alloc(dir, ptr, ptr, 0, 0);
	}
	return dir;
}

/*
 * Initializes paging
 */
void init_paging()
{

	kernel_directory = (page_directory_t*)malloc_pa(sizeof(page_directory_t));
	memset(kernel_directory, 0, sizeof(page_directory_t));

	switch_page_directory(kernel_directory);
	for(uint32_t ptr = 0; ptr < IDENTITY_MAP_END; ptr += 0x1000)
	{
		page_alloc(kernel_directory, ptr, ptr, 0, 0);
	}
	
	enable_paging();
}

/*
 * Switches the current page directory, if this
 * is corrupt, there will be hell to pay....
 */
void switch_page_directory(page_directory_t* dir)
{
	current_directory = dir;
	asm volatile("mov %0, %%cr3":: "r"(&dir->tables_physical));
	asm volatile ("mov %cr3, %eax; mov %eax, %cr3;");
}


void disable_paging()
{
	paging_enabled = false;
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 &= 0x7fffffff;
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void enable_paging()
{
	paging_enabled = true;
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}
