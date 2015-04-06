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
 * pipe.c
 * Provides functions for creating and dealing with pipes.
 */

#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/dirent.h>
#include <infinity/heap.h>
#include <infinity/virtfs.h>
#include <infinity/sched.h>
#include <infinity/sync.h>

#define PIPE_BUFFER_SIZE   4096

struct pipe_buf {
    int     p_pos; 
    char    p_buf[PIPE_BUFFER_SIZE];
};

static void pipe_writec(struct pipe_buf *pipe, char c);
static char pipe_readc(struct pipe_buf *pipe);
static int pipe_write(struct file *fd, const char *buf, off_t off, size_t len);
static int pipe_read(struct file *fd, char *buf, off_t off, size_t len);

/*
 * Creates a unidirectional pipe with f[1] being the
 * input for f[0]. 
 * @param f     An array of struct file pointers, this 
 *              should be empty.
 */
int fpipe(struct file *f[])
{
    struct file *f1 = kalloc(sizeof(struct file));
    struct file *f2 = kalloc(sizeof(struct file));
    memset(f1, 0, sizeof(struct file));
    memset(f2, 0, sizeof(struct file));
    f1->read = pipe_read;
    f2->write = pipe_write;
    struct pipe_buf *buf = (struct pipe_buf*)kalloc(sizeof(struct pipe_buf));
    buf->p_pos = 0;
    f1->f_tag = buf;
    f2->f_tag = buf;
    add_to_file_table(f1);
    add_to_file_table(f2);
    f2->f_flags |= F_SUPPORT_WRITE;
    f1->f_flags |= F_SUPPORT_READ;
    f[0] = f1;
    f[1] = f2;
}

/*
 * Blocks the thread until there is enough room to
 * fit into buf, then proceeds to write the data
 */
static int pipe_write(struct file *fd, const char *buf, off_t off, size_t len)
{
    struct pipe_buf *pipe = fd->f_tag;
    
    while(pipe->p_pos + len > PIPE_BUFFER_SIZE);
        thread_yield();
        
    spin_lock(fd->f_lock);
    memcpy(pipe->p_buf, buf, len);
    pipe->p_pos += len;
    spin_unlock(fd->f_lock);
    return len;
}

/*
 * Blocks the thread until there the buffer has 
 * some data in it, then reads it into buf
 */
static int pipe_read(struct file *fd, char *buf, off_t off, size_t len)
{
    struct pipe_buf *pipe = fd->f_tag;
    
    while(pipe->p_pos < len)
        thread_yield();
    
    spin_lock(fd->f_lock);
    int pp = pipe->p_buf;
    buf[0] = pipe->p_buf[0];
    memcpy(buf, pipe->p_buf, 1);
    memcpy(pipe->p_buf, pipe->p_buf + len, PIPE_BUFFER_SIZE - len);
    pipe->p_pos -= len;
    spin_unlock(fd->f_lock);
    return len;
}
