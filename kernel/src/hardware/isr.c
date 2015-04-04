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
 * isr.c
 * Provides a simple interface for kernel level drivers
 * to request ISR handlers
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/portio.h>
#include <infinity/interrupt.h>

inthandler_t isr_handlers[256] = { 0 };

/*
 * Requests an ISR handler
 * @param inum		The interrupt to hook
 * @param handler	A callback for the ISR
 */
int request_isr(int inum, inthandler_t handler)
{
	if (isr_handlers[inum])
		return -1;
	else
		isr_handlers[inum] = handler;
	return 0;
}

/*
 * Frees an ISR handler
 * @param inum		The ISR to free
 */
int free_isr(int inum)
{
	isr_handlers[inum] = NULL;
	return 0;
}

/*
 * This is an ISR call back. This shouldn't be called
 * in normal code
 * @param regs		Register state before ISR was invoked
 */
void handle_isr(struct regs *registers)
{
	int i = registers->interrupt & 0xFF;
    printk(KERN_INFO "ISR %d (errcode %d)\n", i, registers->eip);
    while(1);
    return;
	if (isr_handlers[i])
		isr_handlers[i](registers);
	outb(0x20, 0x20);
}
