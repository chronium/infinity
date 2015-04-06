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
#include <infinity/virtfs.h>

struct mntpoint root;
static struct file_table_entry *file_table = NULL;

static int remove_from_file_table(struct file *file);

static struct filesystem *virtfs_fs_list = NULL;
static struct mntpoint *virtfs_find_mount_r(struct mntpoint *mnt, char *org_path, char **rel_path);

static int virtfs_open(struct file *fd, const char *path, int oflag);
static int virtfs_unlink(struct file *fd);
static int virtfs_read(struct file *fd, char *buf, off_t off, size_t len);
static int virtfs_write(struct file *fd, const char *buf, off_t off, size_t len);
static int virtfs_readdir(struct file *f, int d, struct dirent *dent);

/*
 * Initialize the virtual filesystem
 * @param dev		The device containing the intial ramdisk
 * @param initrd	The filesystem the initrd uses
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
 * @param dev		The block device the filesystem is on
 * @param fs		The filesystem of dev
 * @param path		The path to mount the fs to
 */
int virtfs_mount(struct device *dev, struct filesystem *fs, const char *path)
{
	printk(KERN_DEBUG "DEBUG: Mounting device '%s' using filesystem '%s' to %s\n", dev->dev_name, fs->fs_name, path);

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
 * @param fs		The filesystem struct
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
 * @param path		The path to the new directory
 */
int mkdir(const char *path)
{
	char *rpath = NULL;
	struct mntpoint *mnt = virtfs_find_mount(path, &rpath);

	return mnt->mt_fs->mkdir(mnt->mt_dev, rpath);
}

/*
 * Deletes a file
 * @param fd		The fildes struct of the file to delete
 */
int unlink(struct file *f)
{
	if (f->f_flags & F_SUPPORT_WRITE && !(f->f_flags & F_CLOSED)) {
        return f->unlink(f);
    }
}

/*
 * Opens a file returning a file struct
 * @param path			The path to the file
 * @param oflags		Flags to open the file with
 */
struct file *fopen(const char *path, int oflags)
{
	struct file *new_file = (struct file *)kalloc(sizeof(struct file));
	memset(new_file, 0, sizeof(struct file));
	int res = virtfs_open(new_file, path, oflags);
	if (res == 0) {
		add_to_file_table(new_file);
		return new_file;
	}

	kfree(new_file);

	return NULL;
}

/*
 * Closes a file and removes it from the global OS file
 * table
 * @param f				The file to close
 */
int fclose(struct file *f)
{
	return remove_from_file_table(f);
}

/*
 * Writes data to fd relative to fd->fd_pos
 * @param fd		The fildes struct of the file to write to
 * @param data		A buffer containing the data to write
 * @param off		The offset within data to start at
 * @param len		The length of data to write
 */
int fwrite(struct file *f, const char *data, off_t off, size_t len)
{
	if (f->f_flags & F_SUPPORT_WRITE && !(f->f_flags & F_CLOSED)) {
		return f->write(f, data, off, len);
	}
	return -1;
}

/*
 * Reads data to fd relative to fd->fd_pos
 * @param fd		The fildes struct of the file to write to
 * @param buf		A buffer containing the data to write
 * @param off		The offset within data to start at
 * @param len		The length of data to write
 */
int fread(struct file *f, char *buf, off_t off, size_t len)
{
	if (f->f_flags & F_SUPPORT_READ && !(f->f_flags & F_CLOSED)) {
		return f->read(f, buf, off, len);
	}
	return -1;
}

/*
 * Reads a directory entry from a directory
 * @param f			The directory we are reading from
 * @param d			The index of the file to read into dent
 * @param dent		A dirent struct to read into
 */
int readdir(struct file *f, int d, struct dirent *dent)
{
	if (f->f_flags & F_SUPPORT_READ && !(f->f_flags & F_CLOSED))
		return f->f_fs->readdir(f->f_dev, f->f_ino, d, dent);
	return -1;
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
 * Deletes a file
 * @param fd		The fildes struct of the file to delete
 */
static int virtfs_unlink(struct file *f)
{
	if (f->f_flags & F_SUPPORT_WRITE)
		return f->f_fs->delete(f->f_dev, f->f_ino);
	return -1;
}

/*
 * Writes data to fd relative to fd->fd_pos
 */
static int virtfs_write(struct file *f, const char *data, off_t off, size_t len)
{
    int bytes_written = f->f_fs->write(f->f_dev, f->f_ino, &data[off], f->f_pos, len);
    f->f_pos += bytes_written;
    int delta = f->f_pos - f->f_len;
    if (delta > 0)
        f->f_len += delta;
    return bytes_written;
}

/*
 * Reads data to fd relative to fd->fd_pos
 */
static int virtfs_read(struct file *f, char *buf, off_t off, size_t len)
{
    int bytes_read = f->f_fs->read(f->f_dev, f->f_ino, &buf[off], f->f_pos, len);
    f->f_pos += bytes_read;
    return bytes_read;
}

/*
 * Opens up a file copying the relevent information into f
 */ 
static int virtfs_open(struct file *f, const char *path, int oflag)
{
	char *rpath = NULL;
	struct mntpoint *mnt = virtfs_find_mount(path, &rpath);
    f->write = virtfs_write;
    f->read = virtfs_read;
    f->unlink = virtfs_unlink;
	f->f_fs = mnt->mt_fs;
	f->f_dev = mnt->mt_dev;
	return mnt->mt_fs->open(mnt->mt_dev, f, rpath, oflag);
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

