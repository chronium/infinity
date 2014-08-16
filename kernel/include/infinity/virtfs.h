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

#ifndef VIRTFS_H
#define VIRTFS_H

#include <stdint.h>
#include <stddef.h>
#include <infinity/device.h>
#include <infinity/dirent.h>
#include <infinity/types.h>
#include <infinity/fcntl.h>
#include <infinity/stat.h>

#define PATH_MAX_LENGTH			1024


struct file_descriptor {
	int 						fd_num;
	struct device* 				fd_dev;
	struct filesystem* 			fd_fs;
	ino_t 						inode;
	int 						fd_flags;
	uint32_t 					fd_position;
	uint32_t 					fd_closed;
	struct file_descriptor* 	fd_next;
};


struct vnode {
	char 						mnt_path[PATH_MAX_LENGTH];
	struct device* 				mnt_dev;
	struct filesystem* 			mnt_fs;
	struct vnode* 				subnodes;
};


struct filesystem {
	char 					fs_name[128];
	int (*delete)			(struct device* dev, const char* path);
	int (*write)			(struct device*, ino_t ino, const char* data, off_t off, size_t len);
	int (*read)				(struct device*, ino_t ino, char* data, off_t off, size_t len);
	int (*readdir) 			(struct device*, ino_t ino, int d, struct dirent* dent);
	int (*rename) 			(struct device*, ino_t ino, const char* name);
	int (*fstat) 			(struct device*, ino_t ino, struct stat* stat_struct);
	int (*open) 			(struct device*, const char* path, struct file_descriptor* fd);
	struct filesystem* 		next_fs;
};

void register_filesystem(struct filesystem* fs);
int mount(const char* path, const char* fs, struct device* dev);
struct vnode* vfs_getnode(const char* path);
void vfs_chroot(struct vnode* root);

#endif
