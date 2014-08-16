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
 * printk.c
 * Used to output messages from kernel land, note we
 * don't always print to the screen. Basically this
 * will usually just log the message, but we can also
 * specify a device to output to (The console, a serial
 * port, ect)
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/device.h>
#include <infinity/common.h>
#include <infinity/textscreen.h>
#include <infinity/ringbuffer.h>
#include <infinity/kernel.h>

extern struct device textscreen_device;

static struct device *printk_output = &textscreen_device;
static int should_log_messages = 1;
static struct ring_buffer msg_queue;

static void kernel_log_msg(kernelmsg_t *msg);

void printk(const char *format, ...)
{
	va_list argp;

	va_start(argp, format);
	char tmp_buff[512];
	memset(tmp_buff, 0, 512);
	vsprintf(tmp_buff, format, argp);
	va_end(argp);
	if (should_log_messages) {
		kernelmsg_t msg;
		memcpy(&msg.msg_string, tmp_buff, 512);
		gmtime_r(time(NULL), &msg.msg_tm);
		kernel_log_msg(&msg);
	}
	if (printk_output)
		printk_output->write(tmp_buff, strlen(tmp_buff), 0);
}

void klog(int log)
{
	if (log)
		rb_init(&msg_queue, sizeof(kernelmsg_t) * 256);
	should_log_messages = log;
}

void klog_output(struct device *dev)
{
	printk_output = dev;
}

static void kernel_log_msg(kernelmsg_t *msg)
{
	rb_push(&msg_queue, msg, sizeof(kernelmsg_t));
}
