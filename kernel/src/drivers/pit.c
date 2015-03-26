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
 * pit.c
 * Driver for the programmable interveral timer
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <infinity/common.h>
#include <infinity/interrupt.h>
#include <infinity/portio.h>
#include <infinity/kernel.h>


uint64_t pit_tick = 0;
extern struct regs *pit_irq_regs;
struct regs *pit_irq_regs = NULL;

static void pit_irq(struct regs *state);


void init_pit(uint32_t freq)
{
	if(!request_irq(0, pit_irq)) {
		uint32_t divisor = 1193180 / freq;
		outb(0x43, 0x36);
		outb(0x40, divisor & 0xFF);
		outb(0x40, (divisor >> 8) & 0xFF);
		printk(KERN_DEBUG "[DEBUG] PIT initialized, divisor %d\n", divisor);
	} else {
		printk(KERN_ERR "[ERROR] Could not retrieve PIT IRQ, threading will not work. Expect a panic() soon\n");
	}
}


static void pit_irq(struct regs *state)
{
	pit_irq_regs = state;
	extern void dequeue_next_task();
	dequeue_next_task();
	
	perform_context_switch(state);
	pit_tick++;
}
