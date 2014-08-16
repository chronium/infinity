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
 * sched.c
 * The scheduler, handles multitasking and process
 * creation
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <infinity/common.h>
#include <infinity/interrupt.h>
#include <infinity/types.h>
#include <infinity/kheap.h>
#include <infinity/paging.h>
#include <infinity/process.h>
#include <infinity/kernel.h>

static void pit_irq(struct regs *state);


void init_pit(uint32_t freq)
{
	request_irq(0, pit_irq);
	uint32_t divisor = 1193180 / freq;
	outb(0x43, 0x36);
	outb(0x40, divisor & 0xFF);
	outb(0x40, (divisor >> 8) & 0xFF);
}

static void pit_irq(struct regs *state)
{
	perform_context_switch(state);
}
