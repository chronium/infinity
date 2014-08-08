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

#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	uint32_t present    : 1;
	uint32_t rw         : 1;
	uint32_t user       : 1;
	uint32_t accessed   : 1;
	uint32_t dirty      : 1;
	uint32_t unused     : 7;
	uint32_t frame      : 20;
} __attribute__((packed)) page_t;

typedef struct
{
	page_t pages[1024];
} page_table_t;

typedef struct
{  
	page_table_t* tables[1024];
	uint32_t tables_physical[1024];
	uint32_t physical_addr;
} page_directory_t;


extern void init_paging();
extern void switch_page_directory(page_directory_t* dir);
extern void enable_paging();
extern void disable_paging();
extern void page_alloc(page_directory_t* dir, uint32_t vaddr, uint32_t paddr, bool write, bool user);
extern void page_remap(page_directory_t* dir, uint32_t vaddr, uint32_t paddr);
extern void page_free(page_directory_t* dir, uint32_t vaddr);
extern page_directory_t* create_new_page_directory();

#endif
