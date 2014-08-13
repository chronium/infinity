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
#include <infinity/file.h>


#define PATH_MAX_LENGTH			1024

typedef struct filesystem filesystem_t;
typedef struct vnode vnode_t;
typedef struct file_descriptor file_descriptor_t;

struct file_descriptor {
	int fd_num;
	device_t* fd_dev;
	filesystem_t* fd_fs;
	ino_t inode;
	mode_t fd_mode;
	void* fd_metadata;
	int fd_position;
	file_descriptor_t* fd_next;
};

struct vnode {
	char mnt_path[PATH_MAX_LENGTH];
	device_t* mnt_dev;
	filesystem_t* mnt_fs;
	vnode_t* subnodes;
};


struct filesystem {
	char fs_name[128];
	int (*delete)(device_t* dev, const char* path);
	int (*write)(device_t* dev, ino_t ino, const char* data, off_t off, size_t len);
	int (*read)(device_t* dev, ino_t ino, char* data, off_t off, size_t len);
	int (*readdir) (device_t* dev, DIR* dir, dirent_t* dent);
	int (*rename) (device_t* dev, ino_t ino, const char* name);
	int (*open) (device_t* dev, const char* path, file_descriptor_t* fd);
	filesystem_t* next_fs;
};

void register_filesystem(filesystem_t* fs);
int mount(const char* path, const char* fs, device_t* dev);
vnode_t* vfs_getnode(const char* path);
void vfs_chroot(vnode_t* root);

#endif
