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

#include <stdint.h>
#include <stddef.h>
#include <infinity/device.h>
#include <infinity/virtfs.h>


vnode_t* vfs_root;

static filesystem_t* filesystems = NULL;

static vnode_t* vfs_getnode_recurse(vnode_t* parent, const char* path);

void register_filesystem(filesystem_t* fs)
{
	fs->next_fs = NULL;
	if(!filesystems)
	{
		filesystems = fs;
		return;
	}
	else
	{
		filesystem_t* curr = filesystems;
		while(curr->next_fs)
			curr = curr->next_fs;
		curr->next_fs = fs;
	}
}

int mount(const char* path, const char* fs, device_t* dev)
{

	return -1;
}

vnode_t* vfs_getnode(const char* path)
{
	return vfs_getnode_recurse(vfs_root, path);
}

static vnode_t* vfs_getnode_recurse(vnode_t* parent, const char* path)
{
	return parent;
}
