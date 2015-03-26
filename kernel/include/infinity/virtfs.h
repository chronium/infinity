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

#ifndef INFINITY_VIRTFS_H
#define INFINITY_VIRTFS_H

#include <stdint.h>
#include <stddef.h>
#include <infinity/dirent.h>
#include <infinity/stat.h>
#include <infinity/sync.h>

#define F_SUPPORT_SEEK		1
#define F_SUPPORT_READ		2
#define F_SUPPORT_WRITE		4
#define F_CLOSED			8

struct mntpoint;
struct fildes;

struct file {
    ino_t				f_ino;
    uint64_t			f_pos;
    uint64_t			f_len;
    uint32_t			f_flags;
    spinlock_t			f_lock;
    struct device		*f_dev;
    struct filesystem	*f_fs;
    struct file			*next;
};

struct file_table_entry {
    struct file *file_entry;
    struct file_table_entry *next;
};

struct fildes {
    int					fd_num;
    uint32_t			fd_flags;
    struct file			*fd_file;
    struct fildes		*next;
};

struct mntpoint {
    char				mt_path[256];
    struct device		*mt_dev;
    struct filesystem	*mt_fs;
    struct mntpoint		*mt_children;
    struct mntpoint		*next;
};

struct filesystem {
    char				fs_name[128];
    int (*mkdir)		(struct device *dev, const char *dir);
    int (*open)			(struct device *dev, struct file *f, const char *path, int oflag);
    int (*delete)		(struct device *dev, ino_t ino);
    int (*write)		(struct device *dev, ino_t ino, const char *data, off_t off, size_t len);
    int (*read)			(struct device *dev, ino_t ino, char *data, off_t off, size_t len);
    int (*readdir)		(struct device *dev, ino_t ino, int d, struct dirent *dent);
    int (*rename)		(struct device *dev, ino_t ino, const char *name);
    int (*fstat)		(struct device *dev, ino_t ino, struct stat *stat_struct);
    struct filesystem	*next;
};

struct file *fopen(const char *path, int oflag);
int fclose(struct file *f);

int virtfs_init(struct device *dev, struct filesystem *initrd);
struct mntpoint *virtfs_find_mount(const char *org_path, char **rel_path);

int virtfs_mount(struct device *dev, struct filesystem *fs, const char *path);
int virtfs_open(struct file *fd, const char *path, int oflag);
int virtfs_delete(struct file *fd);
int virtfs_read(struct file *fd, char *buf, off_t off, size_t len);
int virtfs_write(struct file *fd, const char *buf, off_t off, size_t len);
int virtfs_readdir(struct file *f, int d, struct dirent *dent);
int virtfs_mkdir(const char *path);
int register_fs(struct filesystem *fs);
void init_ramdisk(void *memory, int size);

static inline char *virtfs_remove_leading_slash(const char *path) 
{
    if(path[0] == '/')
        return &path[1];
    return path;
}


#endif
