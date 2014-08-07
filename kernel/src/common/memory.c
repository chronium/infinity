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


#include <stddef.h>
#include <stdint.h>
#include <infinity/common.h>


void* memcpy(void* dest, void* src, size_t size)
{
	for(int i = 0; i < size; i++)
		((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
}

void* memset(void* source, uint8_t b, size_t size)
{
	for(int i = 0; i < size; i++)
		((uint8_t*)source)[i] = b;
}
