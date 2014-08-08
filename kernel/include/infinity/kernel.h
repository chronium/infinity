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

#ifndef KERNEL_H
#define KERNEL_H

#include <infinity/device.h>

typedef struct kernelmsg_t kernelmsg_t;

struct kernelmsg_t {
	char* msg_string;
	kernelmsg_t* last_msg;
};

extern void kernel_log(int log);
extern void set_kernel_output(device_t* dev);
extern void printk(const char* format, ...);
extern void panic(char* msg);
extern void shutdown();

#endif