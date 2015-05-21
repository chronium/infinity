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
#include <infinity/common.h>
#include <infinity/types.h>
#include <infinity/dirent.h>
#include <infinity/stat.h>
#include <infinity/sync.h>

#define F_SUPPORT_SEEK          1
#define F_SUPPORT_READ          2
#define F_SUPPORT_WRITE         4
#define F_CLOSED                8

#define	S_IRWXU	0000700	
#define	S_IRUSR	0000400
#define	S_IWUSR	0000200	
#define	S_IXUSR	0000100

#define	S_IRWXG	0000070	
#define	S_IRGRP	0000040	
#define	S_IWGRP	0000020		
#define	S_IXGRP	0000010

#define	S_IRWXO	0000007	
#define	S_IROTH	0000004
#define	S_IWOTH	0000002		
#define	S_IXOTH	0000001		

struct mntpoint;
struct fildes;

struct inode {
    ino_t               i_ino;
    uid_t               i_uid;
    gid_t               i_gid;
    mode_t              i_mode;
    dev_t               i_dev;
    uint32_t            i_size;
    char                i_islnk;
    char                i_isfifo;
};

struct file {
    uint32_t            f_refs;
    uint32_t            f_len;
    struct inode *      f_ino;
    spinlock_t          f_lock;
    void    *           f_tag;
    struct device *     f_dev;
    struct filesystem * f_fs;
    int                 (*fstat)    (struct file *fd, struct stat *st);
    int                 (*write)    (struct file *fd, char *buf, off_t off, size_t len);
    int                 (*read)     (struct file *fd, const char *buf, off_t off, size_t len);
    int                 (*ioctl)    (struct file *fd, int arg1, int arg2, int arg3);
    int                 (*close)    (struct fildes *fd);
    struct file *       next;
};

struct stat {
    dev_t           st_dev;
    ino_t           st_ino;
    mode_t          st_mode;
    nlink_t         st_nlink;
    uid_t           st_uid;
    gid_t           st_gid;
    dev_t           st_rdev;
    off_t           st_size;
    time_t          st_atime;
    time_t          st_mtime;
    time_t          st_ctime;
};

struct file_table_entry {
    struct file *               file_entry;
    struct file_table_entry *   next;
};

struct fildes {
    char                fd_name[128];
    int                 fd_num;
    uint32_t            fd_flags;
    uint32_t            fd_mode;
    uint32_t            fd_pos;
    struct process *    fd_owner;
    struct file *       fd_file;
    struct fildes *     next;
};

struct mntpoint {
    char                mt_path[256];
    struct device *     mt_dev;
    struct filesystem * mt_fs;
    struct mntpoint *   mt_children;
    struct mntpoint *   next;
};

struct filesystem {
    char        fs_name[128];
    int         (*mkdir)            (struct device *dev, const char *dir, mode_t mode);
    int         (*mkfifo)           (struct device *dev, const char *dir, mode_t mode);
    int         (*creat)            (struct device *dev, const char *dir, mode_t mode);
    int         (*open)             (struct device *dev, struct file *f, const char *path, int oflag);
    int         (*unlink)           (struct device *dev, const char *path);
    int         (*rmdir)            (struct device *dev, const char *path);
    int         (*write)            (struct device *dev, ino_t ino, const char *data, off_t off, size_t len);
    int         (*read)             (struct device *dev, ino_t ino, char *data, off_t off, size_t len);
    int         (*readdir)          (struct device *dev, ino_t ino, int d, struct dirent *dent);
    int         (*readlink)         (struct device *dev, const char *path, char *buf, int blen);
    int         (*symlink)          (struct device *dev, const char *from, const char *to);
    int         (*chmod)            (struct device *dev, const char *path, mode_t newmode);
    int         (*chown)            (struct device *dev, const char *path, uid_t newuid, gid_t newgid);
    int         (*rename)           (struct device *dev, ino_t ino, const char *name);
    int         (*ioctl)            (struct device *dev, ino_t ino, int arg1, int arg2, int arg3);
    int         (*fstat)            (struct device *dev, ino_t ino, struct stat *stat_struct);
    int         (*readino)          (struct device *dev, struct inode *ino, const char *path);
    struct filesystem * next;
};

struct file *fopen(const char *path, int oflag);

int ioctl(int fd, int request, int arg1, int arg2);

int stat(const char *path, struct stat *s);
int lstat(const char *path, struct stat *s);

struct file *virtfs_open(const char *path);
struct file *virtfs_close(const char *path);
int virtfs_readdir(struct file *f, int d, struct dirent *dent);
int virtfs_init(struct device *dev, struct filesystem *initrd);
struct mntpoint *virtfs_find_mount(const char *org_path, char **rel_path);
int virtfs_mount(struct device *dev, struct filesystem *fs, const char *path);
int readlink(const char *path, const char *buf, int len);
int symlink(const char *from, const char *to);
int mkdir(const char *path);
int mkfifo(const char *path, mode_t mode);
int unlink(const char *path);
int rmdir(const char *path);
int chmod(const char *path, mode_t newmode);
int register_fs(struct filesystem *fs);
void init_ramdisk(void *memory, int size);
void add_to_file_table(struct file *nfile);
void named_pipe_open(struct file *f);

int canonicalize_path(const char *path, char *buf) ;

static inline char *virtfs_remove_leading_slash(const char *path)
{
    if (path[0] == '/')
        return &path[1];
    return path;
}

static inline char *basename(const char *filename)
{
    char *last_sep = filename;
    for(int i = 0; i < strlen(filename); i++) {
        if(filename[i] == '/')
            last_sep = &filename[i + 1];
    }
    return last_sep;
}

static inline char *dirname(const char *path, char *resolved)
{
    strcpy(resolved, path);
    if(strlchr(resolved, '/') != resolved)
        *strlchr(resolved, '/') = 0;
    else
        resolved[1] = 0;
    return resolved;
}


#endif
