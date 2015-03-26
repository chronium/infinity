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

struct gdt_entry gdt[6];
struct tss_entry tss_ent;
static struct gdt_ptr ptr;

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
 * Sets up the global descriptor table
 */
void init_gdt()
{
    ptr.limit = (sizeof(struct gdt_entry) * 6);
    ptr.base = (uint32_t)&gdt;
    gdt_set_gate(0, 0, 0, 0, 0);                    // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);     // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);     // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);     // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);     // User mode data segment
    write_tss(5, 0x10, 0x0);
    gdt_flush((uint32_t)&ptr);
    tss_flush();
}

void set_kernel_stack(uint32_t stack)
{
    tss_ent.esp0 = stack;
}

static void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t)&tss_ent;
    uint32_t limit = base + sizeof(struct tss_entry);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    for (int i = 0; i < sizeof(struct tss_entry); i++)
        ((char *)&tss_ent)[i] = 0;
    tss_ent.ss0 = ss0;
    tss_ent.esp0 = esp0;
    tss_ent.cs = 0x0b;
    tss_ent.ss = tss_ent.ds = tss_ent.es = tss_ent.fs = tss_ent.gs = 0x13;
}

void tss_flush()
{
    asm (
        ".intel_syntax noprefix;"
        "mov ax, 0x2B;"
        "ltr ax;"
        ".att_syntax noprefix;"
        );
}
