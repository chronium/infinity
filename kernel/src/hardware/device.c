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
 * device.c
 * Provides an abstraction layer for char/block devices
 */

#include <stddef.h>
#include <infinity/kheap.h>
#include <infinity/device.h>

struct device *device_list = NULL;

void register_device(struct device *dev)
{
	dev->next_device = NULL;
	if (!device_list) {
		device_list = dev;
	} else {
		struct device *curr = device_list;
		while (curr->next_device)
			curr = curr->next_device;
		curr->next_device = dev;
	}
}


struct device *get_device_by_name(const char *dev)
{
	struct device *curr = device_list;

	while (curr->next_device)
		curr = curr->next_device;
	return NULL;
}
