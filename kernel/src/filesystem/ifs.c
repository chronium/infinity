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
 * ifs.c
 * A driver for the Infinity Filesystem
 */

#include <stddef.h>
#include <stdbool.h>
#include <infinity/common.h>
#include <infinity/virtfs.h>
#include <infinity/kernel.h>
#include <infinity/stat.h>
#include <infinity/fs/ifs.h>


struct vnode *vfs_root;


static void ifs_get_block(struct device *dev, int index, ifs_block_t *block);
static int ifs_get_address(struct device *dev, int index);
static int ifs_get_directory(struct device *dev, int parent, char *dir);
static int ifs_get_file_entry(struct device *dev, int parent, char *dir, ifs_entry_t *dest);
static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct);
static int ifs_find_block(struct device *dev, int vsize, int block, int search, int pos);
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len);
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent);
static int ifs_open(struct device *dev, const char *path, struct file_descriptor *fd);

static struct vnode *ifs_getnode(struct vnode *this, const char *path);

void register_ifs()
{
	struct filesystem *ifs = (struct filesystem *)kalloc(sizeof(struct filesystem));

	memcpy(ifs->fs_name, "ifs", 4);
	ifs->read = ifs_read_file;
	ifs->open = ifs_open;
	ifs->readdir = ifs_read_dir;
	ifs->fstat = ifs_fstat;
	register_filesystem(ifs);
	vfs_root->mnt_fs = ifs;
}


/*
 * Helper method to find a specific character in the
 * string, used in the case souly to find the directory
 * seperator
 */
static bool contains(const char *str, char c)
{
	for (int i = 0; str[i] != 0; i++)
		if (str[i] == c)
			return true;
	return false;
}

static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct)
{
	ifs_entry_t entry;

	dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, ino));
	stat_struct->st_mode = entry.umask;
	stat_struct->st_ctime = entry.created_time;
	stat_struct->st_mtime = entry.modified_time;
	stat_struct->st_atime = entry.modified_time;
	stat_struct->st_ino = ino;
}

/*
 * Reads a file, copies data into buff
 */
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len)
{
	ifs_entry_t entry;
	ifs_volume_hdr_t vol_header;

	dev->read(&vol_header, sizeof(ifs_volume_hdr_t), 0);
	dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, ino));
	int offset = addr - ((addr / vol_header.file_block_size) * vol_header.file_block_size);
	int bytes_written = 0;
	for (int i = 0; i < len && i < entry.file_size; i += vol_header.file_block_size) {
		int block = len / vol_header.file_block_size;
		int rlen = len - i;
		dev->read(buff + i, rlen, offset + ifs_get_address(dev, ifs_find_block(dev, vol_header.file_block_size, entry.data_index, addr + i, 0)));
		offset = 0;
		bytes_written++;
	}

	return bytes_written;
}

/*
 * Reads an IFS directory entry
 */
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent)
{
	ifs_entry_t entry;
	int32_t *directory = (int32_t *)kalloc(1024);

	dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, ino));
	dev->read(directory, 1024, ifs_get_address(dev, entry.data_index));

	if (directory[d] == -1)
		return -1;
	dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, directory[d]));
	dent->d_ino = directory[d];
	memcpy(dent->d_name, entry.file_name, 256);
	kfree(directory);
}

/*
 * Copies a block from the IFS Block pool
 * into pointer
 */
static void ifs_get_block(struct device *dev, int index, ifs_block_t *block)
{
	dev->read(block, sizeof(ifs_block_t), sizeof(ifs_volume_hdr_t) + (index * sizeof(ifs_block_t)));
}

/*
 * Returns the address of an BlockEntry stored
 * within the IFS Block pool
 */
static int ifs_get_address(struct device *dev, int index)
{
	ifs_block_t block;

	ifs_get_block(dev, index, &block);
	return block.data;
}

/*
 * Returns the index to a directory table
 */
static int ifs_get_directory(struct device *dev, int parent, char *dir)
{
	bool has_sep = contains(dir, '/');
	/*
	 * We could just make allocate directory on the stack, but
	 * allocating over 1KB of local space in a recursive method
	 * sounds like a bad idea to me
	 */
	int32_t *directory = (int32_t *)kalloc(1024);

	ifs_entry_t dentry;

	dev->read(&dentry, sizeof(ifs_entry_t), ifs_get_address(dev, parent));
	int dtable = dentry.data_index;
	dev->read(directory, 1024, ifs_get_address(dev, dtable));

	for (int i = 0; i < 256 && directory[i] != -1; i++) {
		int e = directory[i];
		ifs_entry_t entry;
		dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, e));
		if (has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0) {
			kfree(directory);
			return ifs_get_directory(dev, e, strrchr(dir, '/') + 1);
		} else if (!has_sep) {
			if (strncmp(entry.file_name, dir, strlen(entry.file_name)) == 0) {
				kfree(directory);
				return e;
			}
		}
	}
	kfree(directory);
	return -1;
}

/*
 * Gets an IFSFileEntry from a certain directory, and copies it
 * into dest
 */
static int ifs_get_file_entry(struct device *dev, int parent, char *dir, ifs_entry_t *dest)
{
	bool has_sep = contains(dir, '/');
	int32_t *directory = (int32_t *)kalloc(1024);
	ifs_entry_t dentry;

	dev->read(&dentry, sizeof(ifs_entry_t), ifs_get_address(dev, parent));
	int dtable = dentry.data_index;
	dev->read(directory, 1024, ifs_get_address(dev, dtable));
	for (int i = 0; i < 256 && directory[i] != -1; i++) {
		int e = directory[i];
		ifs_entry_t entry;
		dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, e));
		if (has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0) {
			kfree(directory);
			return ifs_get_file_entry(dev, e, strlchr(dir, '/') + 1, dest);
		} else if (!has_sep) {
			if (strcmp(entry.file_name, dir) == 0) {
				memcpy(dest, &entry, sizeof(ifs_entry_t));
				kfree(directory);
				return 1;
			}
		}
	}
	kfree(directory);
	return 0;
}

/*
 * Used to find the block that holds the address search
 * within a file
 */
static int ifs_find_block(struct device *dev, int vsize, int block, int search, int pos)
{
	ifs_block_t parent;

	ifs_get_block(dev, block, &parent);
	if (search < pos + vsize)
		return block;
	else
		return ifs_find_block(dev, vsize, parent.next, search, pos + parent.size);
}

static int ifs_open(struct device *dev, const char *path, struct file_descriptor *fd)
{
	if (ifs_get_directory(dev, 0, path) != -1) {
		fd->inode = ifs_get_directory(dev, 0, path);
		return 0;
	} else {
		return -1;
	}
}
