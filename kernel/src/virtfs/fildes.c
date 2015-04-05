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
 * fildes.c
 * I/O operations that utilize file descriptors. These are mainly for usermode
 * syscalls. 
 */
 
#include <infinity/heap.h>
#include <infinity/kernel.h>
#include <infinity/virtfs.h>
#include <infinity/syscalls.h>
#include <infinity/sched.h>
#include <infinity/fildes.h>


extern struct thread *current_thread;

static struct fildes *get_fildes(int num);
static void add_fildes(struct fildes *fd);
static void remove_fildes(struct fildes *fd);

/*
 * Opens up a file, returning a file descriptor
 * @param path      The path to the file to open
 * @param mode      Modes to open the file with
 */
int open(const char *path, int mode)
{
    struct file *f = fopen(path, mode);
    if(f) {
        struct fildes *fd = (struct fildes*)kalloc(sizeof(struct fildes));
        fd->fd_num = current_thread->t_proc->p_nextfd++;
        fd->next = NULL;
        fd->fd_file = f;
        add_fildes(fd);
        f->f_refs++;
        return fd->fd_num;
    } else {
        return -1;
    }
}

/*
 * Closes a file and frees any kernel resources
 * associated with it
 * @param fd        The file descriptor
 */
int close(int fd)
{
    struct fildes *f = get_fildes(fd);
    if(f) {
        f->fd_file->f_refs--;
        remove_fildes(f);
        if(f->fd_file->f_refs <= 0) {
            fclose(f);
        }
    } else {
        return -1;
    }
}

/*
 * Writes data to a file, returning the amount of
 * bytes written
 * @param fd            The file descriptor
 * @param buf           Data to write
 * @param n             Bytes to write
 */
size_t write(int fd, const void *buf, size_t n)
{
    struct fildes *f = get_fildes(fd);
    if(f) {
        return virtfs_write(f->fd_file, buf, 0, n);
    } else {
        return -1;
    }
}

/*
 * Reads data from a file, returning the amount of 
 * bytes read
 * @param fd            The file descriptor
 * @param buf           The buffer to read into
 * @param n             Bytes to read
 */
size_t read(int fd, void *buf, size_t n)
{
    struct fildes *f = get_fildes(fd);
    if(f) {
        return virtfs_read(f->fd_file, buf, 0, n);
    } else {
        return -1;
    }
}

/*
 * Gets a filedes struct from a file descriptor
 */
static struct fildes *get_fildes(int num)
{
    struct process *proc = current_thread->t_proc;
    struct fildes *i = proc->p_fildes_table;
    while(i) {
        if(i->fd_num == num) {
            return i;
        }
        i = i->next;
    }
    return NULL;
}

/*
 * Adds a fildes struct to the current processes
 * file descriptor table
 */
static void add_fildes(struct fildes *fd)
{
    struct process *proc = current_thread->t_proc;
    struct fildes *i = proc->p_fildes_table;
    if(!i) {
        proc->p_fildes_table = fd;
    } else {
        while(i->next) {
            i = i->next;
        }
        i->next = fd;
    }
}

static void remove_fildes(struct fildes *fd)
{
    struct process *proc = current_thread->t_proc;
    struct fildes *i = proc->p_fildes_table;
    if(i == fd) {
        proc->p_fildes_table = i->next;
    } else {
        struct fildes *last = NULL;
        while(i) {
            if(i == fd) {
                last->next = i->next;
            }
            last = i;
            i = i->next;
        }
        i->next = fd;
    }
}
