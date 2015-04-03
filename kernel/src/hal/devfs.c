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
 * File devfs.c
 * Provides filesystem abstraction for device drivers
 */

#include <stddef.h>
#include <infinity/fcntl.h>
#include <infinity/kernel.h>
#include <infinity/common.h>
#include <infinity/device.h>
#include <infinity/types.h>
#include <infinity/heap.h>
#include <infinity/virtfs.h>


extern struct device *device_list;

struct filesystem devfs;

static int devfs_open(struct device *dev, struct file *f, const char *path, int oflag);
static int devfs_write(struct device *dev, ino_t ino, const char *data, off_t off, size_t len);

void init_devfs()
{
    strcpy(devfs.fs_name, "devfs");
    devfs.write = devfs_write;
    devfs.open = devfs_open;
    int res = virtfs_mount(NULL, &devfs, "/dev");
    printk(KERN_DEBUG, "DEBUG: Mounting devfs to /dev\n");
    if (res)
        printk(KERN_ERR, "ERROR: Could not mount devfs to /dev! Things are about to get ugly!\n");
}


static int devfs_open(struct device *dev, struct file *f, const char *path, int oflag)
{
    struct device *i = device_list;

    while (i) {
        if (!strcmp(path, i->dev_name)) {
            f->f_ino = i->dev_id;
            f->f_fs = &devfs;
            if (oflag & O_RDWR || oflag & O_WRONLY)
                f->f_flags |= F_SUPPORT_WRITE;
            if (oflag & O_RDONLY || oflag & O_RDWR)
                f->f_flags |= F_SUPPORT_READ;
            return 0;
        }
        i = i->next;
    }

    return -1;
}

static int devfs_write(struct device *dev, ino_t ino, const char *data, off_t off, size_t len)
{
    struct device *i = device_list;

    while (i) {
        if (i->dev_id == ino)
            return device_write(i, data, len, off);
        i = i->next;
    }

    return -1;
}
