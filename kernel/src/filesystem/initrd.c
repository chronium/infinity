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
 * initrd.c
 * A driver for the initial ramdisk which is mounted on
 * startup
 */
 
#include <stddef.h>
#include <stdbool.h>
#include <infinity/common.h>
#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/kernel.h>
#include <infinity/fs/ifs.h>

extern vnode_t* vfs_root;

static void* initrd_ptr;
static device_t initrd_dev;

static size_t initrd_write(const char* buff, size_t size, int off);
static size_t initrd_read(char* buff, size_t size, int off);

void mount_initrd(void* rd)
{
	initrd_ptr = rd;
	vfs_root = (vnode_t*)kalloc(sizeof(vnode_t));
	register_ifs();
	initrd_dev.write = initrd_write;
	initrd_dev.read = initrd_read;
	register_device(&initrd_dev);
	vfs_root->mnt_dev = &initrd_dev;
	memcpy(vfs_root->mnt_path, "/", 2);
}

static size_t initrd_write(const char* buff, size_t size, int off)
{
	memcpy(initrd_ptr + off, buff, size);
	return size;
}

static size_t initrd_read(char* buff, size_t size, int off)
{
	memcpy(buff, initrd_ptr + off, size);
	return size;
}
