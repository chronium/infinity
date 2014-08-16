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

#ifndef IFS_H
#define IFS_H

#include <infinity/virtfs.h>

typedef enum
{
	IFS_BLOCK_FREE = 1,
	IFS_BLOCK_ALLOCATED = 2,
	IFS_BLOCK_RESERVED = 3,
	IFS_BLOCK_NONEXISTENT = 4
}  ifs_blockstate_t;

typedef enum ifs_filetype {
	MOUNT_POINT = 0,
	DIRECTORY = 1,
	LINK = 2,
	SOCKET = 3,
	PIPE = 4,
	BLOCK_DEVICE = 6,
	CHAR_DEVICE = 7
} ifs_filetype_t;

typedef struct ifs_volume_hdr
{
	uint8_t 		mag0;
	uint8_t 		mag1;
	uint8_t 		mag2;
	uint8_t 		mag3;
	char 			volume_name[128];
	uint32_t 		id;
	uint32_t 		volume_size;
	uint32_t 		file_block_size;
	uint32_t 		file_block_bount;
	uint32_t 		block_pool;
	uint32_t 		block_pool_size;
	uint32_t 		root_directory;
	uint32_t 		file_count;
}  ifs_volume_hdr_t;

typedef struct isf_entry
{
	char 			file_name[128];
	int 			created_time;
	int 			modified_time;
	uint32_t 		block_index;
	uint32_t 		data_index;
	uint32_t 		file_size;
	ifs_filetype_t 	file_type;
	uint32_t 		umask;
	uint32_t 		uid;
	uint32_t 		gid;
} ifs_entry_t;

typedef struct ifs_block
{
	uint32_t data;
	uint32_t size;
	uint32_t next;
	uint32_t state;
} ifs_block_t;

extern void mount_initrd(void* rd);
extern void register_ifs();
#endif
