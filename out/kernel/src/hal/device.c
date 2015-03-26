/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
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
 * File device.c
 * This file provides a way for creating registering new devices with the hardware
 * abstraction layer
 */

#include <stddef.h>
#include <infinity/heap.h>
#include <infinity/device.h>

// Linked list containing all registered devices
struct device *device_list = NULL;

static int next_devid = 0; // Unique identifier for devices

/*
 * Creates a device, returning a pointer to a newly created device struct
 * @param type		The type of device
 * @param name		The name of the device (Must be unique)
 */
struct device *device_create(devtype_t type, const char *name)
{
	struct device *new_dev = (struct device *)kalloc(sizeof(struct device));// Create a new device

	new_dev->dev_name = name;
	new_dev->dev_type = type;
	new_dev->next = NULL;
	new_dev->dev_id = next_devid++;
	struct device *i = device_list;

	if (!device_list) {
		device_list = new_dev;
		return device_list;
	} else {
		while (i->next)
			i = i->next;
		i->next = new_dev;
		return new_dev;
	}
}

/*
 * Reads from a device
 */
size_t device_read(struct device *dev, void *buff, size_t size, uint32_t addr)
{
	return dev->read(dev->dev_tag, buff, size, addr);
}

/*
 * Writes to a device
 */
size_t device_write(struct device *dev, const void *buff, size_t size, uint32_t addr)
{
	return dev->write(dev->dev_tag, buff, size, addr);
}
