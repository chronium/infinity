/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
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


#ifndef INFINITY_HEAP_H
#define INFINITY_HEAP_H

#include <stddef.h>
#include <stdint.h>

struct mblock {
    void *  memory;
    size_t  size;
    void *  next_block;
};

extern void init_kheap(uint32_t i);
extern void *kalloc(size_t s);
extern void *malloc_pa(size_t s);
extern void *realloc(void *ptr, size_t size);
extern void kfree(void *ptr);
extern size_t ksize(void *ptr);

#endif
