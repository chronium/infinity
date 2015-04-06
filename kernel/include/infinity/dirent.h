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


#ifndef INFINITY_DIRENT_H
#define INFINITY_DIRENT_H

#include <stdint.h>
#include <infinity/types.h>

#define DT_UNKNOWN  0x00
#define DT_REG      0x01
#define DT_DIR      0x02
#define DT_FIFO     0x03
#define DT_SOCK     0x04
#define DT_CHR      0x05
#define DT_BLK      0x06


typedef struct {
    int     dd_fd;
    int     dd_loc;
    int     dd_seek;
    char *  dd_buf;
    int     dd_len;
    int     dd_size;
} DIR;

struct dirent {
    ino_t       d_ino;
    off_t       d_off;
    uint16_t    d_reclen;
    uint8_t     d_type;
    char        d_name[256];
};



#endif
