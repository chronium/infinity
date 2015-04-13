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

#ifndef INFINITY_FCNTL_H
#define INFINITY_FCNTL_H

#include <infinity/virtfs.h>
#include <infinity/device.h>
#include <infinity/types.h>

/* Modes for open */
#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_CREAT         0x0200
#define O_TRUNC         0x0400

/* Commands for fcntl */
#define F_DUPFD         0
#define F_GETFD         1
#define F_SETFD         2
#define F_GETFL         3
#define F_SETFL         4

/* Seeking */
#define SEEK_SET                0       // Seek relative to begining of file
#define SEEK_CUR                1       // Seek relative to current file position
#define SEEK_END                2       // Seek relative to end of file

extern int fcntl(int fd, int cmd, ...);
extern int open(const char *path, int flags);
extern int close(int fd);
extern size_t read(int fd, void *buf, size_t nbytes);
extern size_t write(int _fd, const void *buf, size_t nbytes);
int lseek(int fd, int offset, int whence);
int fstat(int fd, struct stat *buf);
#endif
