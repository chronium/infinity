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
#include <infinity/gdt.h>
#include <infinity/tss.h>

gdtentry_t gdt[6];
tssentry_t tss_entry;
gdtptr_t ptr;

static void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0);
extern void tss_flush();

/*
 * Places a new entry into the GDT
 */
static void gdt_set_gate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{

	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = (limit >> 16) & 0x0F;
	gdt[num].granularity |= gran & 0xF0;
	gdt[num].access = access;
} 

/*
 *Sets up the global descriptor table  
 */
void init_gdt()
{
	ptr.limit = (sizeof(gdtentry_t) * 6);
	ptr.base  = (uint32_t)&gdt;
	gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment
	write_tss(5, 0x10, 0x0);
	gdt_flush((uint32_t)&ptr);
	tss_flush();
}

void set_kernel_stack(uint32_t stack)
{
    tss_entry.esp0 = stack;
}

static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0)
{
   uint32_t base = (uint32_t) &tss_entry;
   uint32_t limit = base + sizeof(tssentry_t);
   gdt_set_gate(num, base, limit, 0xE9, 0x00);
   for(int i = 0; i < sizeof(tssentry_t); i++)
	((char*)&tss_entry)[i] = 0;
   tss_entry.ss0 = ss0; 
   tss_entry.esp0 = esp0;
   tss_entry.cs = 0x0b;
   tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void tss_flush()
{ 
	asm(
	".intel_syntax noprefix;"
	"mov ax, 0x2B;"
	"ltr ax;"
	".att_syntax noprefix;"
	);
}

