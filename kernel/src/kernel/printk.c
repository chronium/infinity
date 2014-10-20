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

#define QUEUE_SIZE		sizeof(kernelmsg_t) * 256

extern struct device textscreen_device;

static struct device *printk_output = &textscreen_device;
static int should_log_messages = 1;
static struct ring_buffer msg_queue;

static void kernel_log_msg(kernelmsg_t *msg);

/*
 * Logs a kernel message and prints it to 
 * an output device if specified
 * @param format	Format of the string
 */
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

/*
 * Turns logging on/off
 * @param log	Should we log?
 */
void klog(int log)
{
	if (msg_queue.rb_len != QUEUE_SIZE)
		rb_init(&msg_queue, QUEUE_SIZE);
	should_log_messages = log;
}

/*
 * Flushes the kernel log into a buffer
 * @param buf	A buffer to store the kernel log
 * @param size	The size of buf
 */
void flush_klog(char *buf, int size)
{
	int logsz = msg_queue.rb_pos > msg_queue.rb_len ? msg_queue.rb_len : msg_queue.rb_pos;

	char *log = (char*)kalloc(logsz);
	rb_flush(&msg_queue, log, logsz);	

	int bptr = 0;
	for(int i = 0; i < logsz && bptr < size; i += sizeof(struct kernelmsg))
	{
		struct kernelmsg *msg = (struct kernelmsg*)(log + i);
		char tmp[290];
		memset(tmp, 0, 290);
		sprintf(tmp, "[%d:%d] %s\n", msg->msg_tm.tm_hour, msg->msg_tm.tm_min, msg->msg_string);
		memcpy(buf + bptr, tmp, strlen(tmp));
		bptr += strlen(tmp);
	}
	kfree(log);
}

/*
 * Sets the output device
 * @param dev	The device to display messages
 */
void klog_output(struct device *dev)
{
	printk_output = dev;
}

static void kernel_log_msg(kernelmsg_t *msg)
{
	rb_push(&msg_queue, msg, sizeof(kernelmsg_t));
}
