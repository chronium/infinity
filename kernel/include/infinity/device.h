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


#ifndef DEVICE_H
#define DEVICE_H

#include <stddef.h>

typedef enum 
{
	DEV_BLOCK_DEVICE = 0,
	DEV_CHAR_DEVICE = 1,
} devtype_t;


typedef struct device_t device_t;

struct device_t
{
	devtype_t device_type;
	char* device_name;
	size_t (*read) (char* buffer, size_t, uint32_t addr);
	size_t (*write) (const char* data, size_t, uint32_t add);
	device_t* next_device;
};

device_t* get_device_by_name(const char* dev);
void register_device(device_t* dev);

#endif
