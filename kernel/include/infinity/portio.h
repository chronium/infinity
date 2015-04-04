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

#ifndef INFINITY_PORTIO_H
#define INFINITY_PORTIO_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;

	asm volatile ("inb %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t ret;

	asm volatile ("inw %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline uint32_t inl(uint16_t port)
{
	uint32_t ret;

	asm volatile ("inl %1, %0" : "=a" (ret) : "Nd" (port));
	return ret;
}

static inline void insl(uint16_t port, void *address, int count)
{
	asm volatile ("cld; rep insl" :
		      "=D" (address), "=c" (count) :
		      "d" (port), "0" (address), "1" (count) :
		      "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t val)
{
	asm volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

static inline void outw(uint16_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a" (val), "Nd" (port));
}

static inline void outl(uint16_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a" (val), "Nd" (port));
}

#endif
