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
 
#include <infinity/sched.h>
#include <infinity/process.h>
#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/types.h>

extern process_t* current_process;

static file_descriptor_t* get_file_descriptor(int fd);

int open(const char* _path, mode_t mode)
{
	file_descriptor_t* fd = (file_descriptor_t*)kalloc(sizeof(file_descriptor_t));
	fd->fd_next = NULL;
	fd->fd_num = 0;
	vnode_t* node = vfs_getnode(_path);
	char* path = &_path[strlen(node->mnt_path)];
	if(!node->mnt_fs->open(node->mnt_dev, path, fd))
	{
		fd->fd_dev = node->mnt_dev;
		fd->fd_fs = node->mnt_fs;
		file_descriptor_t* i = current_process->file_descriptors;
		if(!i)
			current_process->file_descriptors = fd;
		else
		{
			while(i->fd_next)
				i = i->fd_next;
			i->fd_next = fd;
		}
		return 0;//fd->fd_num;
	}
	else
		return -1;
	
}

size_t read(int _fd, void* buf, size_t nbytes)
{
	file_descriptor_t* fd = get_file_descriptor(_fd);
	if(fd)
		return fd->fd_fs->read(fd->fd_dev, fd->inode, buf, 0, nbytes); 
	else
		return -1;
}

size_t write(int _fd, const void* buf, size_t nbytes)
{
	file_descriptor_t* fd = get_file_descriptor(_fd);
	if(fd)
		return fd->fd_fs->write(fd->fd_dev, fd->inode, buf, 0, nbytes); 
	else
		return -1;
}

static file_descriptor_t* get_file_descriptor(int fd)
{
	file_descriptor_t* i = current_process->file_descriptors;
	while(i)
	{
		if(i->fd_num == fd)
			return i;
		i = i->fd_next;
	}
	return NULL;
}
