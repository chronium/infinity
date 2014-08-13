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
#include <infinity/fs/ifs.h>


vnode_t* vfs_root;

static void ifs_get_block(device_t* dev, int index, ifs_block_t* block);
static int ifs_get_address(device_t* dev, int index);
static int ifs_get_directory(device_t* dev, int parent, char* dir);
static int ifs_get_file_entry(device_t* dev, int parent, char* dir, ifs_entry_t* dest);
static int ifs_find_block(device_t* dev, int vsize, int block, int search, int pos);
static size_t ifs_read_file (device_t* dev, ino_t ino, char* buff, off_t addr, size_t len);
static int ifs_open(device_t* dev, const char* path, file_descriptor_t* fd);

static vnode_t* ifs_getnode(vnode_t* this, const char* path);

void register_ifs()
{
	filesystem_t* ifs = (filesystem_t*)kalloc(sizeof(filesystem_t));
	memcpy(ifs->fs_name, "ifs", 4); 
	ifs->read = ifs_read_file;
	ifs->open = ifs_open;
	register_filesystem(ifs);
	vfs_root->mnt_fs = ifs;
}


/*
 * Helper method to find a specific character in the
 * string, used in the case souly to find the directory
 * seperator
 */
static bool contains(const char* str, char c)
{
	for(int i = 0; str[i] != 0; i++)
		if(str[i] == c)
			return true;
	return false;
}

/*
 * Reads a file, copies data into buff
 */
static size_t ifs_read_file (device_t* dev, ino_t ino, char* buff, off_t addr, size_t len)
{
	ifs_entry_t entry;
	ifs_volume_hdr_t vol_header;
	dev->read(&vol_header, sizeof(ifs_volume_hdr_t), 0);
	dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, ino));
	int offset = addr - ((addr / vol_header.file_block_size) * vol_header.file_block_size);
	int bytes_written = 0;
	for(int i = 0; i < len && i < entry.file_size; i += vol_header.file_block_size )
	{
		int block = len / vol_header.file_block_size;
		int rlen = len - i;
		dev->read(buff + i, rlen, offset + ifs_get_address(dev, ifs_find_block(dev, vol_header.file_block_size, entry.data_index, addr + i, 0)));
		offset = 0;
		bytes_written++;
	}

	return bytes_written;
}

/*
 * Copies a block from the IFS Block pool 
 * into pointer
 */
static void ifs_get_block(device_t* dev, int index, ifs_block_t* block)
{
	dev->read(block, sizeof(ifs_block_t), sizeof(ifs_volume_hdr_t) + (index * sizeof(ifs_block_t)));
}

/*
 * Returns the address of an BlockEntry stored
 * within the IFS Block pool
 */
static int ifs_get_address(device_t* dev, int index)
{
	ifs_block_t block;
	ifs_get_block(dev, index, &block);
	return block.data;
}

/*
 * Returns the index to a directory table
 */
static int ifs_get_directory(device_t* dev, int parent, char* dir)
{
	bool has_sep = contains(dir, '/');
	/*
	 * We could just make allocate directory on the stack, but
	 * allocating over 1KB of local space in a recursive method
	 * sounds like a bad idea to me
	 */
	int32_t* directory = (int32_t*)kalloc(1024);

	dev->read(directory, 1024, ifs_get_address(dev, parent));
	for(int i = 0; i < 256 && directory[i] != -1; i++)
	{
		int e = directory[i];
		ifs_entry_t entry;
		dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, e));
		if(has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0)
		{
			kfree(directory);
			return ifs_get_directory(dev, entry.data_index, strrchr(dir, '/') + 1);
		}
		else if (!has_sep)
		{
			if(strncmp(entry.file_name, dir, strlen(entry.file_name)) == 0)
			{
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
static int ifs_get_file_entry(device_t* dev, int parent, char* dir, ifs_entry_t* dest)
{
	bool has_sep = contains(dir, '/');
	int32_t* directory = (int32_t*)kalloc(1024);
	dev->read(directory, 1024, ifs_get_address(dev, parent));
	for(int i = 0; i < 256 && directory[i] != -1; i++)
	{
		int e = directory[i];
		ifs_entry_t entry;
		dev->read(&entry, sizeof(ifs_entry_t), ifs_get_address(dev, e));

		if(has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0)
		{
			kfree(directory);
			return ifs_get_file_entry(dev, entry.data_index, strlchr(dir, '/') + 1, dest);
		}
		else if (!has_sep)
		{
			if(strcmp(entry.file_name, dir) == 0)
			{
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
static int ifs_find_block(device_t* dev, int vsize, int block, int search, int pos)
{
	ifs_block_t parent; 
	ifs_get_block(dev, block, &parent);
	if(search < pos + vsize)
		return block;
	else
		return ifs_find_block(dev, vsize, parent.next, search, pos + parent.size);
}

static int ifs_open(device_t* dev, const char* path, file_descriptor_t* fd)
{
	if(ifs_get_directory(dev, 0, path) != -1)
	{
		fd->inode = ifs_get_directory(dev, 0, path);
		return 0;
	}
	else
		return -1;
}
