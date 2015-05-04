/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
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
 * virtfs.c
 * Provides functions for interacting with the virtual filesystem
 */

#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/dirent.h>
#include <infinity/heap.h>
#include <infinity/sched.h>
#include <infinity/errno.h>
#include <infinity/fs.h>

struct mntpoint root;
static struct file_table_entry *file_table = NULL;

static int remove_from_file_table(struct file *file);
static struct file *find_from_file_table(dev_t dev, ino_t ino);

static struct filesystem *virtfs_fs_list = NULL;
static struct mntpoint *virtfs_find_mount_r(struct mntpoint *mnt, char *org_path, char **rel_path);

static int virtfs_readino(struct inode *ino, const char *path);

static int default_write(struct file *f, const char *data, off_t off, size_t len);
static int default_read(struct file *f, char *buf, off_t off, size_t len);
static int default_fstat(struct file *f, struct stat *st);
static int default_ioctl(struct file *f, int arg1, int arg2, int arg3);

static int can_write(struct inode *ino);
static int can_read(struct inode *ino);

/*
 * Initialize the virtual filesystem
 * @param dev       The device containing the intial ramdisk
 * @param initrd    The filesystem the initrd uses
 */
int virtfs_init(struct device *dev, struct filesystem *initrd)
{
    strcpy(root.mt_path, "/");
    root.mt_fs = initrd;
    root.mt_dev = dev;
    root.mt_children = NULL;
    root.next = NULL;
}

/*
 * Will mount a filesystem on the virtual filesystem.
 * @param dev       The block device the filesystem is on
 * @param fs        The filesystem of dev
 * @param path      The path to mount the fs to
 */
int virtfs_mount(struct device *dev, struct filesystem *fs, const char *path)
{
    printk(KERN_DEBUG "Mounting device '%s' using filesystem '%s' to %s\n", dev->dev_name, fs->fs_name, path);

    char *rpath = NULL;
    struct mntpoint *mnt = virtfs_find_mount(path, &rpath);

    struct mntpoint *nmnt = (struct mntpoint *)kalloc(sizeof(struct mntpoint));
    strcpy(nmnt->mt_path, rpath);
    nmnt->mt_children = NULL;
    nmnt->next = NULL;
    nmnt->mt_fs = fs;

    if (!mnt->mt_children) {
        mnt->mt_children = nmnt;
    } else {
        struct mntpoint *i = mnt->mt_children;
        while (i->next)
            i = i->next;
        i->next = nmnt;
    }

    return 0;
}

/*
 * Registers a filesystem with the kernel
 * @param fs        The filesystem struct
 */
int register_fs(struct filesystem *fs)
{
    fs->next = NULL;
    if (virtfs_fs_list) {
        struct filesystem *i = virtfs_fs_list;

        while (i->next)
            i = i->next;
        i->next = fs;
    } else {
        virtfs_fs_list = fs;
    }

    return 0;
}

/*
 * Creates a new directory
 * @param path      The path to the new directory
 */
int mkdir(const char *_path)
{
    char path[256];
    char parent[256];
    canonicalize_path(_path, path);
    dirname(path, parent);
    struct inode ino;
    if(virtfs_readino(&ino, parent) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->mkdir(mnt->mt_dev, rpath, ino.i_mode);
    }
    return ENOENT;
    
}

/*
 * Reads a symbolic link
 */
int readlink(const char *_path, const char *buf, int len)
{
    char path[256];
    canonicalize_path(_path, path);
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->readlink(mnt->mt_dev, rpath, buf, len);
    }
    return ENOENT;
}


int symlink(const char *from, const char *_to)
{
    char path[256];
    char to[256];
    char parent[256];
    dirname(path, parent);
    canonicalize_path(from, path);
    canonicalize_path(_to, to);
    
    struct inode ino;
    struct inode t_ino;
    if(virtfs_readino(&ino, parent) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->symlink(mnt->mt_dev, rpath, to);
    }
    return ENOENT;
}

/*
 * Unlinks a file (Deleting it), _path 
 * should NOT be a non empty directory. 
 * @param path      The path of the file to remove
 */
int unlink(const char *_path)
{
    char path[256];
    canonicalize_path(_path, path);
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        int res = mnt->mt_fs->unlink(mnt->mt_dev, rpath);
        if(res == 0) {
            struct file *node = find_from_file_table(mnt->mt_dev->dev_id, ino.i_ino);
            remove_from_file_table(node);
        }
        return res;
    }
    return ENOENT;
}

/*
 * Changes the mode of a file
 * @param _path     The path of the item to modify
 * @param newmode   The new mode to change _path to
 */
int chmod(const char *_path, mode_t newmode)
{
    char path[256];
    canonicalize_path(_path, path);
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        int res = mnt->mt_fs->chmod(mnt->mt_dev, rpath, newmode);
        return res;
    }
    return ENOENT;
}

/*
 * Creates a new directory
 * @param path      The path to the new directory
 */
int mkfifo(const char *_path, mode_t mode)
{
    char path[256];
    char parent[256];
    canonicalize_path(_path, path);
    dirname(path, parent);
    struct inode ino;
    if(virtfs_readino(&ino, parent) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->mkfifo(mnt->mt_dev, rpath, mode);
    }
    return ENOENT;
    
}

/*
 * Recursively removes all entries in a directory
 * @path path       The path to remove
 */
int rmdir(const char *_path)
{
    char path[256];
    canonicalize_path(_path, path);
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        if(!can_write(&ino)) return EACCES;
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        int res = mnt->mt_fs->rmdir(mnt->mt_dev, rpath);
        if(res == 0) {
            struct file *node = find_from_file_table(mnt->mt_dev->dev_id, ino.i_ino);
            remove_from_file_table(node);
        }
        return res;
    }
    return ENOENT;
}

/*
 * Fills a stat struct from an absolute path
 * @param path      The path of the file to stat
 * @param s         A stat struct to fill
 */
int stat(const char *path, struct stat *s)
{
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->fstat(mnt->mt_dev, ino.i_ino, s);
    }
    return ENOENT;
}

/*
 * Fills a stat struct from an absolute path
 * ignoring any symlinks.
 * @param path      The path of the file to stat
 * @param s         A stat struct to fill
 */
int lstat(const char *path, struct stat *s)
{
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        char *rpath = NULL;
        struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
        return mnt->mt_fs->fstat(mnt->mt_dev, ino.i_ino, s);
    }
    return ENOENT;
}
/*
 * Reads a directory entry from a directory
 * @param f         The directory we are reading from
 * @param d         The index of the file to read into dent
 * @param dent      A dirent struct to read into
 */
int virtfs_readdir(struct file *f, int d, struct dirent *dent)
{
    return f->f_fs->readdir(f->f_dev, f->f_ino->i_ino, d, dent);
}

/*
 * Adds a file struct to the global OS file table
 * @param nfile     The file to add to the table
 */
void add_to_file_table(struct file *nfile)
{
    struct file_table_entry *entry = (struct file_table_entry *)kalloc(sizeof(struct file_table_entry));

    entry->next = NULL;
    entry->file_entry = nfile;
    if (!file_table) {
        file_table = entry;
    } else {
        struct file_table_entry *i = file_table;
        while (i->next)
            i = i->next;
        i->next = entry;
    }
}

/*
 * Creates a struct file from a path. 
 */
struct file *virtfs_open(const char *path)
{
    struct inode ino;
    if(virtfs_readino(&ino, path) == 0) {
        if(ino.i_islnk) {
            char tmp[128];
            readlink(path, tmp, 128);
            return virtfs_open(tmp);
        } else {
            struct file *f = find_from_file_table(ino.i_dev, ino.i_ino);
            if(f == NULL) {
                char *rpath = NULL;
                struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
                f = (struct file*)kalloc(sizeof(struct file));
                f->f_fs = mnt->mt_fs;
                f->f_dev = mnt->mt_dev;
                f->write = default_write;
                f->read = default_read;
                f->fstat = default_fstat;
                f->ioctl = default_ioctl;
                f->f_ino = (struct inode*)kalloc(sizeof(struct inode));
                memcpy(f->f_ino, &ino, sizeof(struct inode));
                
                if(ino.i_isfifo) {
                    named_pipe_open(f);
                }
                add_to_file_table(f);
                
            }
            return f;
        }
    }
    
    return NULL;
}

int canonicalize_path(const char *path, char *buf) 
{
    extern struct process *current_proc;
    buf[0] = 0;
    if(path[0] == '/') {
        strcpy(buf, path);
    } else {
        sprintf(buf, "%s/%s", current_proc->p_wd, path);
    }
    return 0;
}

/*
 * Writes data to fd relative to fd->fd_pos
 */
static int default_write(struct file *f, const char *data, off_t off, size_t len)
{
    return f->f_fs->write(f->f_dev, f->f_ino->i_ino, data, off, len);
}

/*
 * Reads data to fd relative to fd->fd_pos
 */
static int default_read(struct file *f, char *buf, off_t off, size_t len)
{
    return f->f_fs->read(f->f_dev, f->f_ino->i_ino, buf, off, len);
}

static int default_fstat(struct file *f, struct stat *st)
{
    return f->f_fs->fstat(f->f_dev, f->f_ino->i_ino, st);
}

static int default_ioctl(struct file *f, int arg1, int arg2, int arg3)
{
    return f->f_fs->ioctl(f->f_dev, f->f_ino->i_ino, arg1, arg2, arg3);
}

/*
 * Fill ino with relevent information!
 */
static int virtfs_readino(struct inode *ino, const char *path)
{
	char *rpath = NULL;
    struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
    return mnt->mt_fs->readino(mnt->mt_dev, ino, rpath);
}

/*
 * Find a mount point
 */
struct mntpoint *virtfs_find_mount(const char *org_path, char **rel_path)
{
    return virtfs_find_mount_r(&root, org_path, rel_path);
}

/*
 * Will recursively try to find a mount point in mnt's children, and if not
 * return mnt
 */
static struct mntpoint *virtfs_find_mount_r(struct mntpoint *mnt, char *org_path, char **rel_path)
{
    struct mntpoint *i = mnt->mt_children;
    char *rel = virtfs_remove_leading_slash(&org_path[strlen(mnt->mt_path)]);

    while (i) {
        int slen = strlen(i->mt_path);
        if (!strncmp(i->mt_path, rel, slen)) 
            return virtfs_find_mount_r(i, rel, rel_path);
        i = i->next;
    }
    *rel_path = rel;
    return mnt;
}

static struct file *find_from_file_table(dev_t dev, ino_t ino)
{
    struct file_table_entry *i = file_table;

    while (i) {
        struct file *f = i->file_entry;
        if(f->f_ino->i_ino == ino && f->f_ino->i_dev == dev)
            return f;
        i = i->next;
    }
    
    return NULL;
}


/*
 * Removes a file from the global OS file table
 */
static int remove_from_file_table(struct file *file)
{
    struct file_table_entry *i = file_table;
    struct file_table_entry *last = NULL;

    while (i) {
        if (i->file_entry == file) {
            if (last)
                last->next = i->next;
            else
                file_table = i->next;
            kfree(i->file_entry);
            kfree(i);
            return 0;
        }

        last = i;
        i = i->next;
    }
    return -1;
}

/*
 * Can we write?
 */
static int can_write(struct inode *ino)
{
    if((ino->i_mode & S_IWUSR) && (ino->i_uid == getuid() || getuid() == 0)) {
        return 1;
    } else if(ino->i_gid == getgid() && (ino->i_mode & S_IWGRP)) {
        return 0;
    }
    return ino->i_mode & S_IWOTH;
}

static int can_read(struct inode *ino)
{
    if((ino->i_mode & S_IRUSR) && (ino->i_uid == getuid() || getuid() == 0)) {
        return 1;
    } else if(ino->i_gid == getgid() && (ino->i_mode & S_IRGRP)) {
        return 1;
    }
    return ino->i_mode & S_IROTH != 0;
}
