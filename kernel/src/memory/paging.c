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
 * paging.c
 * The low level paging code, this really should never
 * be used. memmanager.h provides more abstraction, this
 * is basically the raw code for writing to page directories
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/paging.h>
#include <infinity/memmanager.h>
//#include "page_fault.h"

#define IDENTITY_MAP_END                0x4000000



bool paging_enabled;

struct page_directory *current_directory;

struct page_directory *kernel_directory;

static struct page_table *clone_table(struct page_directory *pdir, struct page_table *src, uint32_t *phys_addr);

void page_alloc(struct page_directory *dir, uint32_t vaddr, uint32_t paddr, bool write, bool user)
{
    void *rd = vaddr;
	vaddr /= 0x1000;
	uint32_t i = vaddr / 1024;
	if (!dir->tables[i]) {
		dir->tables[i] = (struct page_table *)malloc_pa(sizeof(struct page_table));
		for (int x = 0; x < 0x1000; x++)
			((char *)dir->tables[i])[x] = 0;
		dir->tables_physical[i] = (uint32_t)dir->tables[i] | 0x7;
	}
    
	struct page *p = &dir->tables[i]->pages[vaddr % 1024];
	if (p->present) {
		return;
    }
	p->frame = paddr >> 12;
	p->present = 1;
	p->rw = write;
	p->user = user;
}

void page_remap(struct page_directory *dir, uint32_t vaddr, uint32_t paddr)
{
	vaddr /= 0x1000;
	uint32_t i = vaddr / 1024;
	void *old_t = dir->tables[i];
	dir->tables[i] = (struct page_directory *)malloc_pa(sizeof(struct page_table));
	memcpy(dir->tables[i], old_t, sizeof(struct page_table));
	dir->tables_physical[i] = (uint32_t)dir->tables[i] | 0x7;
	struct page *p = &dir->tables[i]->pages[vaddr % 1024];
	p->frame = paddr >> 12;
}

void page_free(struct page_directory *dir, uint32_t vaddr)
{
	vaddr /= 0x1000;
	uint32_t i = vaddr / 1024;
	memset(&dir->tables[i]->pages[vaddr % 1024], 0, sizeof(struct page));
}

void *get_physical_addr(struct page_directory *pdir, void *vaddr)
{
    
	uint32_t i = (uint32_t)vaddr / 0x1000 / 1024;
    
    struct page_table *tbl = pdir->tables[i];
    if(tbl) {
        struct page *p = &pdir->tables[i]->pages[((uint32_t)vaddr / 0x1000) % 1024];
        return (void*)((p->frame << 12) | ((uint32_t)vaddr & 0xFFF));
    }
    return NULL;
}


/*
 * Initializes paging
 */
void init_paging()
{
	//request_isr(14, page_fault_handler);

	kernel_directory = (struct page_directory *)malloc_pa(sizeof(struct page_directory));
	memset(kernel_directory, 0, sizeof(struct page_directory));

	switch_page_directory(kernel_directory);
	for (uint32_t ptr = 0; ptr < IDENTITY_MAP_END; ptr += 0x1000)
		page_alloc(kernel_directory, ptr, ptr, 0, 0);

	enable_paging();
        
}

