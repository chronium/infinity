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
 * file.c
 * Contains methods for basic file IO
 */

#include <stdarg.h>
#include <infinity/sched.h>
#include <infinity/process.h>
#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/types.h>
#include <infinity/fcntl.h>
#include "file-descriptor.h"

extern struct process *current_process;

int open(const char *_path, int flags)
{
	struct file_descriptor *fd = (struct file_descriptor *)get_new_fd();

	fd->fd_closed = 0;
	fd->fd_flags = flags;
	struct vnode *node = vfs_getnode(_path);
	char *path = &_path[strlen(node->mnt_path)];
	if (!node->mnt_fs->open(node->mnt_dev, path, fd)) {
		fd->fd_dev = node->mnt_dev;
		fd->fd_fs = node->mnt_fs;
		if (fd->fd_next) {
			return fd->fd_num;
		} else {
			struct file_descriptor *i = current_process->file_descriptors;
			if (!i) {
				current_process->file_descriptors = fd;
			} else {
				while (i->fd_next)
					i = i->fd_next;
				i->fd_next = fd;
			}
			return fd->fd_num;
		}
	} else {
		return -1;
	}
}

int close(int _fd)
{
	struct file_descriptor *i = current_process->file_descriptors;

	while (i) {
		if (!i->fd_closed && i->fd_num == _fd) {
			i->fd_closed = 1;
			return 0;
		}
		i = i->fd_next;
	}
}

size_t read(int _fd, void *buf, size_t nbytes)
{
	struct file_descriptor *fd = get_file_descriptor(_fd);

	if (fd && !fd->fd_closed)
		return fd->fd_fs->read(fd->fd_dev, fd->inode, buf, 0, nbytes);
	return -1;
}

size_t write(int _fd, const void *buf, size_t nbytes)
{
	struct file_descriptor *fd = get_file_descriptor(_fd);

	if (fd && !fd->fd_closed && (fd->fd_flags & O_WRONLY || fd->fd_flags & O_RDWR))
		return fd->fd_fs->write(fd->fd_dev, fd->inode, buf, 0, nbytes);
	return -1;
}

int fcntl(int _fd, int cmd, ...)
{
	va_list argp;

	va_start(argp, cmd);
	struct file_descriptor *fd = get_file_descriptor(_fd);
	if (!fd)
		return -1;
	switch (cmd) {
	case F_GETFL:
		va_end(argp);
		return fd->fd_flags;
	case F_SETFL:
		fd->fd_flags = va_arg(argp, int);
		break;
	default:
		va_end(argp);
		return -1;
	}
	va_end(argp);
	return 0;
}

struct file_descriptor *get_file_descriptor(int fd)
{
	struct file_descriptor *i = current_process->file_descriptors;

	while (i) {
		if (i->fd_num == fd && !i->fd_closed)
			return i;
		i = i->fd_next;
	}
	return NULL;
}

struct file_descriptor *get_new_fd()
{
	struct file_descriptor *i = current_process->file_descriptors;
	int cnt = 0;

	while (i) {
		if (i->fd_closed)
			return i;
		i = i->fd_next;
		cnt++;
	}
	struct file_descriptor *fd = (struct file_descriptor *)kalloc(sizeof(struct file_descriptor));
	fd->fd_num = cnt;
	fd->fd_next = NULL;
	return fd;
}
