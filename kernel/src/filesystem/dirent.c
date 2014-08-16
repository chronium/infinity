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
 * dirent.c
 * Contains methods for handling directories
 */

#include <stdarg.h>
#include <infinity/sched.h>
#include <infinity/process.h>
#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/types.h>
#include <infinity/fcntl.h>
#include "file-descriptor.h"

DIR *opendir(const char *name)
{
	int rd = open(name, O_RDONLY);

	printk("Descriptor %d\n", rd);
	if (rd != -1)
		return fdopendir(rd);
	else
		return NULL;
}

DIR *fdopendir(int fd)
{
	DIR *dir = (DIR *)kalloc(sizeof(DIR));

	memset(dir, 0, sizeof(DIR));
	dir->dd_fd = fd;
	dir->dd_buf = kalloc(sizeof(struct dirent));
	dir->dd_size = 1;
	return dir;
}


struct dirent *readdir(DIR *dir)
{
	struct file_descriptor *fd = get_file_descriptor(dir->dd_fd);

	if (!fd)
		return NULL;
	if (dir->dd_loc * sizeof(struct dirent) > dir->dd_size * sizeof(struct dirent)) {
		dir->dd_buf = realloc(dir->dd_buf, dir->dd_loc * sizeof(struct dirent));
		dir->dd_size++;
	}
	struct dirent *new_dir = (struct dirent *)(dir->dd_buf + dir->dd_loc * sizeof(struct dirent));
	fd->fd_fs->readdir(fd->fd_dev, fd->inode, dir->dd_loc, new_dir);
	dir->dd_loc++;
	return new_dir;
}
