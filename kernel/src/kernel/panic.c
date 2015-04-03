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
 * panic.c
 * Brings the system down in the case of a fatal error or
 * exception
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <infinity/common.h>
#include <infinity/kernel.h>

extern struct device textscreen_device;

static void display_registers(struct regs *regs);

/*
 * Gracefully brings the kernel down in the
 * result of a fatal error
 * @param format    The format of the message to display
 */
void panic(const char *format, ...)
{
    klog(0); // Disable the kernel log because the heap could be corrupt

    char msg_buff[512];

    memset(msg_buff, 0, 512);

    va_list argp;
    va_start(argp, format);
    vsprintf(msg_buff, format, argp);
    va_end(argp);

    printk(KERN_EMERG "kernel panic: %s\n\n", msg_buff);
    printk(KERN_EMERG "The system is HALTED!\n");


    while (1)
        asm ("hlt");
}
