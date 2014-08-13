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


#ifndef DIRENT_H
#define DIRENT_H

#include <infinity/types.h>

typedef struct {
    int dd_fd;
    int dd_loc;	
    int dd_seek;
    char* dd_buf;
    int dd_len;
    int dd_size; 
} DIR;

typedef struct dirent
{   
	ino_t d_ino;
	off_t d_off;

	unsigned short int d_reclen;
	unsigned char d_type;
	char d_name[256];
} dirent_t;

#endif
