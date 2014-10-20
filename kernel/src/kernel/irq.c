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
 * irq.c
 * Provides a simple interface for kernel level drivers
 * to request IRQ handlers
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/interrupt.h>

inthandler_t irq_handlers[16];

/*
 * Request an IRQ handler, returns 0 if the specified
 * handler is free.
 * @param inum		The IRQ to hook
 * @param handler	The IRQ handler
 */
int request_irq(int inum, inthandler_t handler)
{
	if (inum > 16 || irq_handlers[inum])
		return -1;
	else
		irq_handlers[inum] = handler;
	return 0;
}

/*
 * Frees an IRQ
 * @param inum		The IRQ to free
 */
int free_irq(int inum)
{
	irq_handlers[inum] = NULL;
	return 0;
}

/*
 * This is an IRQ call back. This shouldn't be called
 * in normal code
 * @param regs		Register state before IRQ was invoked
 */
void handle_irq(struct regs registers)
{
	int i = registers.interrupt & 0xFF;

	if (i >= 40)
		outb(0xA0, 0x20);
	if (irq_handlers[i - 32])
		irq_handlers[i - 32](&registers);
		
	outb(0x20, 0x20);
}
