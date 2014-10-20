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


struct vnode *vfs_root;

static struct filesystem *filesystems = NULL;

static struct vnode *vfs_getnode_recurse(struct vnode *parent, const char *path);

/*
 * Registers a filesystem
 * @oaram fs	The filesystem to register
 */
void register_filesystem(struct filesystem *fs)
{
	fs->next_fs = NULL;
	if (!filesystems) {
		filesystems = fs;
		return;
	} else {
		struct filesystem *curr = filesystems;
		while (curr->next_fs)
			curr = curr->next_fs;
		curr->next_fs = fs;
	}
}

/*
 * Mounts a filesystem onto the virtual filesystem, returns
 * -1 if failure
 */
int mount(const char *path, const char *fs, struct device *dev)
{
	return -1;
}

struct vnode *vfs_getnode(const char *path)
{
	return vfs_getnode_recurse(vfs_root, path);
}

static struct vnode *vfs_getnode_recurse(struct vnode *parent, const char *path)
{
	return parent;
}
