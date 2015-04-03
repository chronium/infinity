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


#ifndef MEMMANAGER_H
#define MEMMANAGER_H

#include <stdint.h>

#define PAGE_RW                 0x01
#define PAGE_USER               0x02

struct page_frame;

struct page_frame {
    uint16_t                ref_count;
    uint32_t                phys_addr;
    uint32_t                virt_addr;
    uint32_t                index;
    struct page_frame *     last_frame;
    struct page_directory * page_directory;
};

void *frame_alloc(void *vaddr, int flags);
void *frame_alloc_d(struct page_directory *dir, void *vaddr, int flags);
void frame_free(void *vaddr);
void *frame_free_d(struct page_directory *dir, void *vaddr);
void *frame_map(void *vaddr, void *freeaddr);

#endif
