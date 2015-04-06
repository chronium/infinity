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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include  "ifs.h"


static char *ifs_ptr;

static void ifs_get_block(int index, struct ifs_block *block);
static int ifs_get_last_block(int start);
static void ifs_write_block(int index, struct ifs_block *block);
static int ifs_get_address(int index);
static int ifs_get_directory(int parent, char *dir);
static int ifs_get_file_entry(int parent, char *dir, struct ifs_entry *dest);
static int ifs_find_block(int vsize, int block, int search, int pos);
static size_t device_read(void *buff, uint32_t addr, size_t size);
static size_t device_write(const void *buff, uint32_t addr, size_t size);
static int ifs_insert_entry(const char *path, struct ifs_entry *entry);
static int ifs_block_alloc(int size);


void ifs_create_image(void *ptr, int size)
{
	ifs_ptr = (char *)ptr;
	memset(ptr, 0, size);
	struct ifs_volume_hdr *vol_header = (struct ifs_volume_hdr *)ifs_ptr;

	vol_header->file_block_size = 1024;
	vol_header->block_pool_size = (0xFFFF * sizeof(struct ifs_block));
	vol_header->placement_new = sizeof(struct ifs_volume_hdr) + (0xFFFF * sizeof(struct ifs_block));
	struct ifs_block *block_pool = (ptr + sizeof(struct ifs_volume_hdr));

	for (int i = 0; i < 0xFFFF; i++)
		block_pool[i].state = IFS_BLOCK_NONEXISTENT;
	vol_header->root_directory = 0;



	memset(ptr + vol_header->placement_new, 0xFF, 1024 * 4);

	vol_header->mag0 = 0xCB;
	vol_header->mag1 = 0x0A;
	vol_header->mag2 = 0x0D;
	vol_header->mag3 = 0x0D;

	int root = ifs_block_alloc(sizeof(struct ifs_entry));
	int table = ifs_block_alloc(1024);

	struct ifs_entry dir;
	dir.umask = 484;
	dir.block_index = root;
	dir.file_type = IFS_REG_FILE;
	dir.data_index = table;
	dir.block_index = 0;
	device_write(&dir, ifs_get_address(0), sizeof(struct ifs_entry));

	void *empty_dir = (void *)malloc(1024);
	memset(empty_dir, 0xFF, 1024);

	device_write(empty_dir, ifs_get_address(table), 1024);
	free(empty_dir);
}

/*
 * Helper method to find a specific character in the
 * string, used in the case souly to find the directory
 * seperator
 */
static inline int contains(const char *str, char c)
{
	for (int i = 0; str[i] != 0; i++)
		if (str[i] == c)
			return 1;
	return 0;
}

static void ifs_get_basename(char *dest, const char *org)
{
	if (contains(org, '/'))
		strcpy(dest, strrchr(org, '/') + 1);
	else
		strcpy(dest, org);
}

static inline int ifs_get_parent(const char *path)
{
	int parent = 0;

	if (contains(path, '/')) {
		char tmp[256];
		strcpy(tmp, path);
		*strrchr(tmp, '/') = 0;
		parent = ifs_get_directory(0, tmp);
	}
	return parent;
}

static inline int strindx(const char *str, char ch)
{
	int dlen = strlen(str);
	char *rstr = NULL;

	for (int i = 0; i < dlen; i++)
		if ((char *)str[i] == ch)
			return i;
	return -1;
}

/*
 * Allocates a block in the IFS block pool with a
 * specified size
 */
static int ifs_block_alloc(int size)
{
	struct ifs_block blk;
	struct ifs_volume_hdr hdr;

	device_read(&hdr, 0, sizeof(struct ifs_volume_hdr));

	for (int i = 0; i < hdr.block_pool_size; i += sizeof(struct ifs_block)) {
		device_read(&blk, sizeof(struct ifs_volume_hdr) + i, sizeof(struct ifs_block));
		if ((blk.state == IFS_BLOCK_FREE && blk.size == size) || blk.state == IFS_BLOCK_NONEXISTENT) {
			blk.size = size;
			blk.state = IFS_BLOCK_ALLOCATED;
			blk.data = hdr.placement_new;
			blk.next = 0;
			hdr.placement_new += size;
			device_write(&hdr, 0, sizeof(struct ifs_volume_hdr));
			device_write(&blk, sizeof(struct ifs_volume_hdr) + i, sizeof(struct ifs_block));
			return i / sizeof(struct ifs_block);
		}
	}
	return -1;
}


static void ifs_write_block(int index, struct ifs_block *block)
{
	device_write(block, sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)), sizeof(struct ifs_block));
}

/*
 * Reads a file, copies data into buff
 */
size_t ifs_read_file(ino_t ino, char *buff, off_t addr, size_t len)
{
	struct ifs_entry entry;
	struct ifs_volume_hdr vol_header;

	device_read(&vol_header, 0, sizeof(struct ifs_volume_hdr));
	device_read(&entry, ifs_get_address(ino), sizeof(struct ifs_entry));
	int offset = addr - ((addr / vol_header.file_block_size) * vol_header.file_block_size);
	int bytes_written = 0;
	for (int i = 0; i < len && i < entry.file_size; i += vol_header.file_block_size) {
		int block = len / vol_header.file_block_size;
		int rlen = len - i;
		device_read(buff + i, offset + ifs_get_address(ifs_find_block(vol_header.file_block_size, entry.data_index, addr + i, 0)), rlen);
		offset = 0;
		bytes_written += rlen;
	}

	return bytes_written;
}

size_t ifs_write_file(ino_t ino, char *buff, off_t addr, size_t len)
{
	struct ifs_entry entry;
	struct ifs_volume_hdr vol_header;

	device_read(&vol_header, 0, sizeof(struct ifs_volume_hdr));
	device_read(&entry, ifs_get_address(ino), sizeof(struct ifs_entry));

	int bytes_written = 0;

	int block = entry.data_index;

	do {
		int rlen = bytes_written + 1024 < len ? 1024 : len - bytes_written;
		bytes_written += device_write(buff + bytes_written, ifs_get_address(block), rlen);

		if (bytes_written < len) {
			struct ifs_block bstruct;
			ifs_get_block(block, &bstruct);
			bstruct.next = ifs_block_alloc(1024);
			ifs_write_block(block, &bstruct);
			block = bstruct.next;
		}
	} while (bytes_written < len);

	entry.file_size = len;
	device_write(&entry, ifs_get_address(ino), sizeof(struct ifs_entry));

	return bytes_written;
}

/*
 * Blocks containing file data form a linked list (There is no guarantee that the
 * file data is all together, so this function will return a block index for a
 * offset inside a file
 */
static int ifs_find_block(int vsize, int block, int search, int pos)
{
	struct ifs_block parent;

	ifs_get_block(block, &parent);

	if (search < pos + vsize)
		return block;
	else
		return ifs_find_block(vsize, parent.next, search, pos + parent.size);
}

/*
 * Copies a block from the IFS Block pool
 * into pointer
 */
static void ifs_get_block(int index, struct ifs_block *block)
{
	device_read(block, sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)), sizeof(struct ifs_block));
}

static int ifs_get_last_block(int start)
{
	struct ifs_block block;
	int ret = start;

	ifs_get_block(start, &block);
	do {
		ret = block.next;
		ifs_get_block(block.next, &block);
	} while (block.next != 0);

	return ret == 0 ? start : ret;
}

/*
 * Returns the address of an BlockEntry stored
 * within the IFS Block pool
 */
static int ifs_get_address(int index)
{
	struct ifs_block block;

	ifs_get_block(index, &block);
	return block.data;
}

/*
 * Returns the index to a directory table
 */
static int ifs_get_directory(int parent, char *dir)
{
	bool has_sep = contains(dir, '/');
	/*
	 * We could just make allocate directory on the stack, but
	 * allocating over 1KB of local space in a recursive method
	 * sounds like a bad idea to me
	 */
	int32_t *directory = (int32_t *)malloc(1024);
	struct ifs_entry dentry;

	device_read(&dentry, ifs_get_address(parent), sizeof(struct ifs_entry));
	int dtable = dentry.data_index;
	device_read(directory, ifs_get_address(dtable), 1024);
	for (int i = 0; i < 256 && directory[i] != -1; i++) {
		int e = directory[i];
		struct ifs_entry entry;
		device_read(&entry, ifs_get_address(e), sizeof(struct ifs_entry));
		if (has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0) {
			free(directory);
			return ifs_get_directory(e, strchr(dir, '/') + 1);
		} else if (!has_sep) {
			if (strncmp(entry.file_name, dir, strlen(entry.file_name)) == 0) {
				free(directory);
				return e;
			}
		}
	}
	free(directory);
	return -1;
}

int ifs_open(const char *path, int oflags)
{
	if (ifs_get_directory(0, path) != -1) {
		ino_t ino = ifs_get_directory(0, path);
		return ino;
	} else {
		return -1;
	}
}

static int ifs_insert_entry(const char *path, struct ifs_entry *entry)
{
	int e_block = ifs_block_alloc(1024);

	entry->block_index = e_block;
	int parent = ifs_get_parent(path);
	struct ifs_entry p_ent;
	device_read(&p_ent, ifs_get_address(parent), sizeof(struct ifs_entry));
	int32_t *files = (int32_t *)malloc(1024);
	device_read(files, ifs_get_address(p_ent.data_index), 1024);
	int i = 0;
	while (files[i] != -1) i++;
	files[i] = e_block;
	device_write(files, ifs_get_address(p_ent.data_index), 1024);
	device_write(entry, ifs_get_address(e_block), sizeof(struct ifs_entry));
	free(files);
	return e_block;
}

int ifs_mkdir(const char *path)
{
	int d_block = ifs_block_alloc(1024);

	struct ifs_entry dir;

	dir.umask = 484;
	dir.file_type = IFS_DIRECTORY;
	dir.data_index = d_block;
	dir.file_size = 0;
	ifs_get_basename(dir.file_name, path);
	int e = ifs_insert_entry(path, &dir);
	int32_t *files = (int32_t *)malloc(1024);
	memset(files, 0xFF, 1024);
	device_write(files, ifs_get_address(d_block), 1024);
	free(files);
}

int ifs_add_file(const char *path, const void *file, size_t size)
{
	int d_block = ifs_block_alloc(1024);

	struct ifs_entry dir;

	dir.umask = 484;
	dir.file_type = IFS_REG_FILE;
	dir.data_index = d_block;
	dir.file_size = size;
	ifs_get_basename(dir.file_name, path);
	int e = ifs_insert_entry(path, &dir);
	ifs_write_file(e, file, 0, size);
}

static size_t device_read(void *buff, uint32_t addr, size_t size)
{
	memcpy(buff, &ifs_ptr[addr], size);
	return size;
}


static size_t device_write(const void *buff, uint32_t addr, size_t size)
{
	memcpy(&ifs_ptr[addr], buff, size);
	return size;
}
