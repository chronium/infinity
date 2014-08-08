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
#include <infinity/idt.h>
#include <infinity/portio.h>
#include <infinity/interrupt.h>
#include <infinity/common.h>

static idtentry_t idt[256];

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

extern inthandler_t* isr_handlers[];
extern inthandler_t* irq_handlers[];

/*
 * Install handler in the interrupt descriptor table
 */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
	idt[num].offset_1 = base & 0xFFFF;
	idt[num].offset_2 = (base >> 16) & 0xFFFF;
	idt[num].selector = sel;
	idt[num].zero = 0;
	idt[num].type_attr = flags;
} 

/*
 * Setup the interrupt descriptor table
 */
void init_idt()
{
	idtptr_t ptr;
	for(int i = 0; i < 8 * 256; i++)
		((char*)&idt)[i] = 0;
	ptr.limit = 8 * 256 -1;
	ptr.base  = (uint32_t)&idt;
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	idt_set_gate(0, (uint32_t)isr0 , 0x08, 0x8E);
	idt_set_gate(1, (uint32_t)isr1 , 0x08, 0x8E);
	idt_set_gate(2, (uint32_t)isr2 , 0x08, 0x8E);
	idt_set_gate(3, (uint32_t)isr3 , 0x08, 0x8E);
	idt_set_gate(4, (uint32_t)isr4 , 0x08, 0x8E);
	idt_set_gate(5, (uint32_t)isr5 , 0x08, 0x8E);
	idt_set_gate(6, (uint32_t)isr6 , 0x08, 0x8E);
	idt_set_gate(7, (uint32_t)isr7 , 0x08, 0x8E);
	idt_set_gate(8, (uint32_t)isr8 , 0x08, 0x8E);
	idt_set_gate(9, (uint32_t)isr9 , 0x08, 0x8E);
	idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
	idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
	idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
	idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
	idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
	idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
	idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
	idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
	idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
	idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
	idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
	idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
	idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
	idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
	idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
	idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
	idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
	idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
	idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
	idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
	idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
	idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

	idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
	idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
	idt_set_gate(33, (uint32_t)irq2, 0x08, 0x8E);
	idt_set_gate(34, (uint32_t)irq3, 0x08, 0x8E);
	idt_set_gate(35, (uint32_t)irq4, 0x08, 0x8E);
	idt_set_gate(36, (uint32_t)irq5, 0x08, 0x8E);
	idt_set_gate(37, (uint32_t)irq6, 0x08, 0x8E);
	idt_set_gate(38, (uint32_t)irq7, 0x08, 0x8E);
	idt_set_gate(39, (uint32_t)irq8, 0x08, 0x8E);
	idt_set_gate(40, (uint32_t)irq9, 0x08, 0x8E);
	idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
	idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
	idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
	idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
	idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
	idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
	idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
	idt_set_gate(0x80, (uint32_t)isr128, 0x08, 0xEE);
 
	memset(&isr_handlers, 0, 256 * 4);
	memset(&irq_handlers, 0, 16 * 4);
	
    asm ("lidt (%0)" : : "p"(&ptr));

    asm volatile("sti");


}
