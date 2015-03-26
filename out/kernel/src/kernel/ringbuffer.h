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

#ifndef KERNEL_RINGBUFFER_H
#define KERNEL_RINGBUFFER_H

#include <infinity/sync.h>

struct ring_buffer {
	char *		rb_buff;
	int		rb_len;
	int		rb_pos;
	int		rb_start;
	spinlock_t	rb_lock;
};

void rb_init(struct ring_buffer *rb, int size);
void rb_push(struct ring_buffer *rb, const void *data, int len);
void rb_pop(struct ring_buffer *rb, void *buf, int len);
void *rb_flush(struct ring_buffer *rb, void *buf, int size);

#endif
