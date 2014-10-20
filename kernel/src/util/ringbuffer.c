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
 * ringbuffer.c
 * Implemenation of a ring buffer datastructure
 */

#include <infinity/sync.h>
#include <infinity/ringbuffer.h>
#include <infinity/kheap.h>

static void rb_push_byte(struct ring_buffer *rb, char b);
static int rb_pop_byte(struct ring_buffer *rb);

void rb_init(struct ring_buffer *rb, int size)
{
	rb->rb_len = size;
	rb->rb_pos = 0;
	rb->rb_start = 0;
	rb->rb_buff = kalloc(size);
	spin_unlock(&rb->rb_lock);
}

void rb_push(struct ring_buffer *rb, const void *data, int len)
{
	spin_lock(&rb->rb_lock);
	for (int i = 0; i < len; i++)
		rb_push_byte(rb, ((char *)data)[i]);
	spin_unlock(&rb->rb_lock);
}

void rb_pop(struct ring_buffer *rb, void *buf, int len)
{
	spin_lock(&rb->rb_lock);
	for (int i = len - 1; i >= 0; i--)
		((char *)buf)[i] = (char)rb_pop_byte(rb);
	spin_unlock(&rb->rb_lock);
}

void *rb_flush(struct ring_buffer *rb, void *buf, int size)
{
	int pos = rb->rb_start;
	for(int i = 0; i < rb->rb_len && i < size; pos = (pos + 1) % rb->rb_len)
	{
		((char*)buf)[i++] = rb->rb_buff[pos];
	}
}

static void rb_push_byte(struct ring_buffer *rb, char b)
{
	rb->rb_buff[rb->rb_pos % rb->rb_len] = b;
	rb->rb_pos++;
	if(rb->rb_pos % rb->rb_len > rb->rb_len)
		rb->rb_start = rb->rb_pos % rb->rb_len;
}

static int rb_pop_byte(struct ring_buffer *rb)
{
	rb->rb_pos--;
	int ret = rb->rb_buff[rb->rb_pos % rb->rb_len];
	return ret;
}
