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


#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;      
	uint8_t access; 
	uint8_t granularity;
	uint8_t base_high;         
} __attribute__((packed)) gdtentry_t;

typedef struct 
{
	uint16_t limit;
	uint32_t base;
}  __attribute__((packed)) gdtptr_t;


extern void init_gdt();
extern void gdt_flush(uint32_t ptr);
extern void set_kernel_stack(uint32_t stack);
#endif
