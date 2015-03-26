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

#ifndef INFINITY_STAT_H
#define INFINITY_STAT_H

#include <infinity/types.h>

struct stat {
  unsigned short 		st_dev;
  unsigned short 		st_ino;
  unsigned int 			st_mode;
  unsigned short 		st_nlink;
  unsigned short 		st_uid;
  unsigned short 		st_gid;
  unsigned short 		st_rdev;
  unsigned int 			st_size;
  unsigned int 			st_atime;
  unsigned int 			st_mtime;
  unsigned int 			st_ctime;
};

int fstat(int fd, struct stat *buf);

#endif
