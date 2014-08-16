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

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ifs.h"


/*
 * The IFS Volume header
 */
static IFSVolume* vol_header;

/*
 * The IFS Block pool
 */
static IFSBlock* block_pool;

/*
 * A pointer to the intitial ramdisk
 */
static void* initrd;

static unsigned int placement;
/*
 * Function prototypes
 */

static void ifs_get_block(int index, IFSBlock* block);
static int ifs_get_address(int index);
static int ifs_get_directory(int parent, char* dir);
static int ifs_get_file_entry(int parent, char* dir, IFSFileEntry* dest);
static int ifs_find_block(int vsize, int block, int search, int pos);
static size_t ifs_read_file (int inode, char* buff, size_t len, uint32_t addr);
static IFSBlock* ifs_block_alloc();
static IFSBlock* ifs_block_alloc_s(int size);
static IFSBlock* ifs_block_alloc_i(int size, int* index);
static int* ifs_get_directory_table(int inode);
static IFSFileEntry* ifs_create_file_entry();

static int strindx (const char* str, char ch);
static bool contains(const char* str, char c);

/*
 * Creates an image
 */
void create_image(void* image, int size)
{
	initrd = image;
	vol_header = (IFSVolume*)image;
	memset(vol_header, 0, size);
	vol_header->file_block_size = 1024;
	vol_header->block_pool_size = (0xFFFF * sizeof(IFSBlock));
	placement = sizeof(IFSVolume) + (0xFFFF * sizeof(IFSBlock)) ;
	block_pool = (image + sizeof(IFSVolume));
	for(int i = 0; i < 0xFFFF; i++)
		block_pool[i].state = BLOCK_NONEXISTENT;
	vol_header->root_directory = 0;
	IFSFileEntry* rootdir = ifs_create_file_entry();
	rootdir->data_index = 1;
	IFSBlock* root = ifs_block_alloc();
	
	memset(initrd + ifs_get_address(1), 0xFF, 1024);

	/*
	 * Yes Courtney, even though you completely fucked me over
	 * in just about every respect, I immortalized you in the IFS
	 * magic number. Not that you will ever see this. Maybe because
	 * I still love you, maybe because, well I have no fucking clue....
	 *
	 * Magic number: 0xCB0A0D0D (Big endian)
	 */
	vol_header->mag0 = 0xCB;
	vol_header->mag1 = 0x0A;
	vol_header->mag2 = 0x0D;
	vol_header->mag3 = 0x0D;
}


/*
 * Creates a new IFS Directory
 */
void make_dir(char* name)
{
	if(name[0] == '/') // Ignore leading /
		name++;
	int parent = 1; // Root by default
	if(contains(name, '/')) // Parent directory?
	{
		char* tmp = (char*)malloc(strlen(name));
		strcpy(tmp, name);
		*strrchr(tmp, '/') = 0;
		parent = ifs_get_directory(1, tmp);
		free(tmp);
	}
	int* directory = ifs_get_directory_table(parent);
	IFSFileEntry* ent = ifs_create_file_entry();
	if(contains(name, '/'))
		strcpy(ent->file_name, strrchr(name, '/') + 1);
	else
		strcpy(ent->file_name, name);
	ent->file_type = DIRECTORY;
	int pos = 0;
	while(directory[pos] != -1) pos++;
	directory[pos] = ent->block_index;

	int ptr = ifs_block_alloc_i(1024, &ent->data_index)->data;
	memset(initrd + ptr, 0xFF, 1024);
}

/*
 * Creates a new IFS file
 */
void add_file(char* name, char* contents, int len)
{
	if(name[0] == '/') // Ignore leading /
		name++;
	int parent = 1; // Root by default
	if(contains(name, '/')) // Parent directory?
	{
		char* tmp = (char*)malloc(strlen(name));
		strcpy(tmp, name);
		*strrchr(tmp, '/') = 0;
		parent = ifs_get_directory(1, tmp);
		free(tmp);
	}
	int* directory = ifs_get_directory_table(parent);
	IFSFileEntry* ent = ifs_create_file_entry();
	if(contains(name, '/'))
		strcpy(ent->file_name, strrchr(name, '/') + 1);
	else
		strcpy(ent->file_name, name);
	ent->file_type = REG_FILE;
	int pos = 0;
	while(directory[pos] != -1) pos++;
	directory[pos] = ent->block_index;
	ent->file_size = len;

	int prevI = 0;
	for(int i = 0; i < len; i += vol_header->file_block_size)
	{
		int index = 0;

		void* ptr = (void*)(initrd + ifs_block_alloc_i(vol_header->file_block_size, &index)->data);
		if(i == 0)
			ent->data_index = (uint)index;
		else
			block_pool[prevI].next = index;

		memcpy(ptr, contents + i, (int)(len - i > vol_header->file_block_size ? vol_header->file_block_size : len - i));
		prevI = index;
	}

}


/*
 * Helper method to find a specific character in the
 * string, used in the case solely to find the directory
 * separator
 */
static bool contains(const char* str, char c)
{
	for(int i = 0; str[i] != 0; i++)
		if(str[i] == c)
			return true;
	return false;
}


static int strindx (const char* str, char ch)
{
	int dlen = strlen(str);
	char* rstr = NULL;
	for(int i = 0; i < dlen; i++)
	{
		if((char*)str[i] == ch)
		{
			return i;
		}
	}
	return -1;
}
/*
 * Allocates a block in the IFS block pool with the default
 * size
 */
static IFSBlock* ifs_block_alloc()
{
	return ifs_block_alloc_s(vol_header->file_block_size);
}
/*
 * Allocates a block in the IFS block pool with a
 * specified size
 */
static IFSBlock* ifs_block_alloc_s(int size)
{
	int dummy = 0;
	return ifs_block_alloc_i(size, &dummy);
}

/*
 * Allocates a block in the IFS block pool, puts the
 * index of the block in index
 */
static IFSBlock* ifs_block_alloc_i(int size, int* index)
{
	int i = 0;
	for(i = 0; block_pool[i].state != BLOCK_NONEXISTENT; i++)
	{
		if(block_pool[i].state == BLOCK_FREE && block_pool[i].size <= size)
		{
			*index = i;
			return &block_pool[i];
		}
	}
	*index = i;
	block_pool[i].size = size;
	block_pool[i].state = BLOCK_ALLOCATED;
	block_pool[i].data = placement;
	placement += size;
	return &block_pool[i];
}

/*
 * Creates a new IFS File Entry
 */
static IFSFileEntry* ifs_create_file_entry()
{
	int index = 0;
	int ptr = ifs_block_alloc_i(sizeof(IFSFileEntry), &index)->data;
	memset(initrd + ptr, 0, sizeof(IFSFileEntry));
	IFSFileEntry* entry = (IFSFileEntry*)(initrd + ptr);
	entry->block_index = index;
	entry->mode = 484;
	entry->created_time = time(NULL);
	entry->modified_time = time(NULL);
	return entry;
}
/*
 * Reads a file, copies data into buff
 */
static size_t ifs_read_file (int inode, char* buff, size_t len, uint32_t addr)
{
	IFSFileEntry* entry = (IFSFileEntry*)(initrd + ifs_get_address(inode));
	int offset = addr - ((addr / vol_header->file_block_size) * vol_header->file_block_size);

	for(int i = 0; i < len; i += vol_header->file_block_size)
	{
		int block = len / vol_header->file_block_size;
		int rlen = len - i;
		memcpy(buff + i, initrd + offset + ifs_get_address(ifs_find_block(vol_header->file_block_size, entry->data_index, addr + i, 0)), rlen);

		offset = 0;
	}
	return len;
}

/*
 * Copies a block from the IFS Block pool
 * into pointer
 */
static void ifs_get_block(int index, IFSBlock* block)
{
	memcpy(block, initrd + vol_header->block_pool + (index * sizeof(IFSBlock)), sizeof(IFSBlock));
}

/*
 * Returns the address of an BlockEntry stored
 * within the IFS Block pool
 */
static int ifs_get_address(int index)
{
	return block_pool[index].data;
}

/*
 * Returns the index to a directory table
 */
static int ifs_get_directory(int parent, char* dir)
{
	bool has_sep = contains(dir, '/');
	int32_t* directory = (int32_t*)(initrd + ifs_get_address(parent));
	for(int i = 0; i < 256 && directory[i] != -1; i++)
	{
		int e = directory[i];
		IFSFileEntry* entry = (IFSFileEntry*)(initrd + ifs_get_address(e));

		if(has_sep && strncmp(dir, entry->file_name, strindx(dir, '/')) == 0)
		{
			return ifs_get_directory(entry->data_index, strrchr(dir, '/') + 1);
		}
		else if (!has_sep)
		{
			if(strncmp(entry->file_name, dir, strlen(entry->file_name)) == 0)
			{
				return e;
			}
		}
	}
	return -1;
}

/*
 * Gets an IFSFileEntry from a certain directory, and copies it
 * into dest
 */
static int ifs_get_file_entry(int parent, char* dir, IFSFileEntry* dest)
{
	bool has_sep = contains(dir, '/');
	int32_t* directory = (int32_t*)(initrd + ifs_get_address(parent));
	for(int i = 0; i < 256&& directory[i] != -1; i++)
	{
		int e = directory[i];
		IFSFileEntry* entry = (IFSFileEntry*)(initrd + ifs_get_address(e));

		if(has_sep && strncmp(dir, entry->file_name, strindx(dir, '/')) == 0)
		{
			return ifs_get_file_entry(entry->data_index, strrchr(dir, '/') + 1, dest);
		}
		else if (!has_sep)
		{
			if(strcmp(entry->file_name, dir) == 0)
			{
				memcpy(dest, entry, sizeof(IFSFileEntry));

				return 1;
			}
		}
	}
	return 0;
}

/*
 * Used to find the block that holds the address search
 * within a file
 */
static int ifs_find_block(int vsize, int block, int search, int pos)
{
	IFSBlock parent;
	ifs_get_block(block, &parent);
	if(search < pos + vsize)
		return block;
	else
		return ifs_find_block(vsize, parent.next, search, pos + parent.size);
}

/*
 * Returns an IFS directory table containing up to
 * 256 files
 */
static int* ifs_get_directory_table(int inode)
{
	return (int*)(initrd + ifs_get_address(inode));
}

