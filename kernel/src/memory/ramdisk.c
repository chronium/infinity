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
 * ramdisk.c
 * Provides functions for interacting with the initial RAM disk
 * and creating the initrd device
 */
  
#include <infinity/common.h>
#include <infinity/device.h>
#include <infinity/virtfs.h>
#include <infinity/fs/ifs.h>
#include <infinity/kernel.h>

extern struct filesystem ifs_filesystem;

struct device *ramdisk_dev;

static char *initrd_ptr;

static size_t ramdisk_write(void *tag, const void *buff, int off, size_t size);
static size_t ramdisk_read(void *tag, void *buff, int off, size_t size);

/*
 * Initializes the initrd
 * @param memory		The address in RAM where the initrd starts
 * @param size			The size of the initrd
 */
void init_ramdisk(void *memory, int size) 
{
	printk(KERN_DEBUG "DEBUG: Preparing to create initrd (Start %x)\n", memory);
	ramdisk_dev = device_create(BLOCK_DEVICE, "ramdisk");
	ramdisk_dev->read = ramdisk_read;
	ramdisk_dev->write = ramdisk_write;
	initrd_ptr = memory;
	
	register_ifs();
	virtfs_init(ramdisk_dev, &ifs_filesystem);
}

static size_t ramdisk_write(void *tag, const void *buff, int off, size_t size)
{
	memcpy(&initrd_ptr[off], buff, size);
	return size;
}

static size_t ramdisk_read(void *tag, void *buff, int off, size_t size)
{
	memcpy(buff, &initrd_ptr[off], size);
	return size;
}
