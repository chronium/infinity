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

struct page {
    uint32_t    present    : 1;
    uint32_t    rw         : 1;
    uint32_t    user       : 1;
    uint32_t    accessed   : 1;
    uint32_t    dirty      : 1;
    uint32_t    unused     : 7;
    uint32_t    frame      : 20;
} __attribute__((packed));

struct page_table {
    struct page pages[1024];
};

struct page_directory {
    struct page_table * tables[1024];
    uint32_t            tables_physical[1024];
    uint32_t            physical_addr;
};


void init_paging();
void switch_page_directory(struct page_directory *dir);
void enable_paging();
void disable_paging();
void page_alloc(struct page_directory *dir, uint32_t vaddr, uint32_t paddr, bool write, bool user);
void page_remap(struct page_directory *dir, uint32_t vaddr, uint32_t paddr);
void page_free(struct page_directory *dir, uint32_t vaddr);
struct page_directory *create_new_page_directory();

static inline void invlpg(void *m)
{
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}
#endif
