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

#ifndef INFINITY_DEVICE_H
#define INFINITY_DEVICE_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum devtype {
    BLOCK_DEVICE = 0,
    CHAR_DEVICE = 1,
} devtype_t;


struct device {
    char            dev_name[64];
    devtype_t       dev_type;
    int             dev_id;
    void *          dev_tag;
    size_t          (*read)     (void *tag, void *buffer, size_t, uint32_t addr);
    size_t          (*write)    (void *tag, const void *data, size_t, uint32_t addr);
    int             (*ioctl)    (void *tag, int arg1, int arg2, int arg3);
    struct device * next;
};

struct device *device_create(devtype_t type, const char *name);
size_t device_read(struct device *dev, void *buff, size_t size, uint32_t addr);
size_t device_write(struct device *dev, const void *buff, size_t size, uint32_t addr);
size_t device_ioctl(struct device *dev, int arg1, int arg2, int arg3);
void init_devfs();

#endif
