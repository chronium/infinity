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
 * stat.c
 * Contains methods for accessing file metadata
 */

#include <stdarg.h>
#include <infinity/sched.h>
#include <infinity/process.h>
#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/types.h>
#include <infinity/fcntl.h>
#include <infinity/stat.h>
#include "file-descriptor.h"


int fstat(int _fd, struct stat *buf)
{
	struct file_descriptor *fd = get_file_descriptor(_fd);

	if (!fd)
		return -1;
	fd->fd_fs->fstat(fd->fd_dev, fd->inode, buf);
	return 0;
}
