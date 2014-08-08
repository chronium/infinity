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
#include <infinity/textscreen.h>
#include <infinity/interrupt.h>
#include <infinity/kernel.h>

extern device_t textscreen_device;

static void display_registers(registers_t* regs);

void panic(const char* format, ...)
{
	
	klog_output(&textscreen_device);
	clrscr();
	
	/*
	 * The kernel log is useless at this point so disable it
	 */
	klog(0);
	
	char msg_buff[512];
	va_list argp;
	va_start(argp, format);	
	vsprintf(msg_buff, format, argp);
	va_end(argp);
	
	printk("fatal kernel panic: %s\n\n", msg_buff);
	printk("The system is HALTED!\n\0");
	
	
	while(1);
}
