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

/*
 * ifs.c
 * A driver for the Infinity Filesystem
 */

#include <stddef.h>
#include <stdbool.h>
#include <infinity/common.h>
#include <infinity/fs.h>
#include <infinity/kernel.h>
#include <infinity/stat.h>
#include <infinity/sched.h>
#include <infinity/errno.h>
#include <infinity/fs/ifs.h>

struct filesystem ifs_filesystem;

static int ifs_block_alloc(struct device *dev, int size);
static void ifs_get_block(struct device *dev, int index, struct ifs_block *block);
static void ifs_write_block(struct device *dev, int index, struct ifs_block *block);
static int ifs_get_address(struct device *dev, int index);
static int ifs_get_directory(struct device *dev, int parent, char *dir);
static int ifs_get_file_entry(struct device *dev, int parent, char *dir, struct ifs_entry *dest);
static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct);
static int ifs_find_block(struct device *dev, int vsize, int block, int search, int pos);
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len);
static size_t ifs_write_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len);
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent);
static int ifs_read_ino(struct device *dev, struct inode *ino, const char *path);
static int ifs_mkdir(struct device *dev, const char *dir, mode_t mode);
static int ifs_mkfifo(struct device *dev, const char *path, mode_t mode);
static int ifs_creat(struct device *dev, const char *dir, mode_t mode);
static int ifs_unlink(struct device *dev, const char *path);
static int ifs_rmdir(struct device *dev, const char *path);
static int ifs_readlink(struct device *dev, const char *path, char *buf, int len);
static int ifs_symlink(struct device *dev, const char *from, const char *to);
static int ifs_free_blockgroup(struct device *dev, struct ifs_entry *entry);
static int ifs_chmod(struct device *dev, const char *path, mode_t newmode);


void register_ifs()
{
    memcpy(ifs_filesystem.fs_name, "ifs", 4);
    ifs_filesystem.read = ifs_read_file;
    ifs_filesystem.write = ifs_write_file;
    ifs_filesystem.readdir = ifs_read_dir;
    ifs_filesystem.fstat = ifs_fstat;
    ifs_filesystem.mkdir = ifs_mkdir;
    ifs_filesystem.readino = ifs_read_ino;
    ifs_filesystem.unlink = ifs_unlink;
    ifs_filesystem.rmdir = ifs_rmdir;
    ifs_filesystem.readlink = ifs_readlink;
    ifs_filesystem.symlink = ifs_symlink;
    ifs_filesystem.chmod = ifs_chmod;
    ifs_filesystem.mkfifo = ifs_mkfifo;
    register_fs(&ifs_filesystem);
}

static int ifs_mount(struct device *dev)
{
    struct ifs_volume_hdr hdr;


    device_read(dev, &hdr, sizeof(struct ifs_volume_hdr), 0);

    if (hdr.mag0 != 0xCB || hdr.mag1 != 0x0A || hdr.mag2 != 0x0D || hdr.mag3 != 0x0D)
        return -1;

    return 0;
}

/*
 * Helper method to find a specific character in the
 * string, used in the case souly to find the directory
 * seperator
 */
static inline int contains(const char *str, char c)
{
    for (int i = 0; str[i] != 0; i++)
        if (str[i] == c)
            return 1;
    return 0;
}

static inline int ifs_get_parent(struct device *dev, const char *path)
{
    int parent = 0;

    if (contains(path, '/')) {
        char tmp[256];
        strcpy(tmp, path);
        *strlchr(tmp, '/') = 0;
        parent = ifs_get_directory(dev, 0, tmp);
    }
    return parent;
}

static inline int ifs_remove_entry(struct device *dev, const char *path, struct ifs_entry *entry)
{
    int parent = ifs_get_parent(dev, path);
    struct ifs_entry p_ent;
    device_read(dev, &p_ent, sizeof(struct ifs_entry), ifs_get_address(dev, parent));
    int32_t files[256];
	device_read(dev, files, 1024, ifs_get_address(dev, p_ent.data_index));
    int i = 0;
    while(files[i] != entry->block_index && files[i] != -1) i++;
    for(int j = i; j < 255 && files[j] != -1; j++) {
        files[j] = files[j + 1];
    }
	device_write(dev, files, 1024, ifs_get_address(dev, p_ent.data_index));
   
	return 0;
}

static inline int ifs_insert_entry(struct device *dev, const char *path, struct ifs_entry *entry)
{
	int e_block = ifs_block_alloc(dev, 1024);
	entry->block_index = e_block;
    int parent = ifs_get_parent(dev, path);
    struct ifs_entry p_ent;
    device_read(dev, &p_ent, sizeof(struct ifs_entry), ifs_get_address(dev, parent));
	int32_t *files = (int32_t *)kalloc(1024);
	device_read(dev, files, 1024, ifs_get_address(dev, p_ent.data_index));
	int i = 0;
	while (files[i] != -1) i++;
	files[i] = e_block;
	device_write(dev, files, 1024, ifs_get_address(dev, p_ent.data_index));
	device_write(dev, entry, sizeof(struct ifs_entry), ifs_get_address(dev, e_block));
	kfree(files);
	return e_block;
}

/*
 * Allocates a block in the IFS block pool with a
 * specified size
 */
static int ifs_block_alloc(struct device *dev, int size)
{
    struct ifs_block blk;
    struct ifs_volume_hdr hdr;

    device_read(dev, &hdr, sizeof(struct ifs_volume_hdr), 0);

    for (int i = 0; i < hdr.block_pool_size; i += sizeof(struct ifs_block)) {
        device_read(dev, &blk, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + i);
        if (blk.state == IFS_BLOCK_NONEXISTENT) {
            blk.size = size;
            blk.state = IFS_BLOCK_ALLOCATED;
            blk.data = hdr.placement_new;
            hdr.placement_new += size;
            device_write(dev, &hdr, sizeof(struct ifs_volume_hdr), 0);
            device_write(dev, &blk, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + i);
            return i / sizeof(struct ifs_block);
        } else if (blk.state == IFS_BLOCK_FREE && blk.size == size) {
            blk.state = IFS_BLOCK_ALLOCATED;
            device_write(dev, &blk, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + i);
            return i / sizeof(struct ifs_block);
        } 
    }
    return -1;
}

/*
 * Frees a block
 */
static int ifs_free_block(struct device *dev, int block)
{
    struct ifs_volume_hdr hdr;
    struct ifs_block blk;
    ifs_get_block(dev, block, &blk);
    device_read(dev, &hdr, sizeof(struct ifs_volume_hdr), 0);
    blk.state = IFS_BLOCK_FREE;
    device_write(dev, &blk, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + (block * sizeof(struct ifs_block)));
}

static void ifs_write_block(struct device *dev, int index, struct ifs_block *block)
{
    device_write(dev, block, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)));
}


static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct)
{
    struct ifs_entry entry;
    device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
    stat_struct->st_mode = entry.mode;
    stat_struct->st_ctime = entry.created_time;
    stat_struct->st_mtime = entry.modified_time;
    stat_struct->st_atime = entry.modified_time;
    stat_struct->st_ino = ino;
    stat_struct->st_size = entry.file_size;
    stat_struct->st_uid = 0;
    stat_struct->st_gid = 1;
    return 0;
}

/*
 * Reads a file, copies data into buff
 */
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len)
{
    struct ifs_entry entry;
    struct ifs_volume_hdr vol_header;
    size_t bytes_read = 0;

    device_read(dev, &vol_header, sizeof(struct ifs_volume_hdr), 0);
    device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
    
    int i = 0;
    int offset = addr % vol_header.file_block_size;
    int blk = entry.data_index;
    int start = addr - addr % vol_header.file_block_size;
    int end = addr + len > entry.file_size ? 0 : addr + len;
    
    do {
        struct ifs_block block;
        ifs_get_block(dev, blk, &block);
        if(i >= start && i < end) {
            int rlen = i + vol_header.file_block_size >= len ? len % vol_header.file_block_size : vol_header.file_block_size;
            
            bytes_read += device_read(dev, buff + bytes_read, rlen, block.data + offset);
            
            offset = 0;
        }
        i += vol_header.file_block_size;
        blk = block.next;
        
    } while(blk);
    return bytes_read;
}

static size_t ifs_write_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len)
{
    return 0;
}

/*
 * Reads an IFS directory entry
 */
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent)
{
    struct ifs_entry entry;
    int32_t *directory = (int32_t *)kalloc(1024);

    device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
    device_read(dev, directory, 1024, ifs_get_address(dev, entry.data_index));

    if (directory[d] == -1)
        return -1;
    device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, directory[d]));
    dent->d_ino = directory[d];
    
    switch(entry.file_type) {
        case IFS_DIRECTORY:
            dent->d_type = DT_DIR;
            break;
        case IFS_REG_FILE:
            dent->d_type = DT_REG;
            break;
        case IFS_LINK:
            dent->d_type = DT_LNK;
            break;
        default:
            dent->d_type = DT_UNKNOWN;
            break;
    }
    memcpy(dent->d_name, entry.file_name, 256);
    kfree(directory);
    return 0;
}

/*
 * Copies a block from the IFS Block pool
 * into pointer
 */
static void ifs_get_block(struct device *dev, int index, struct ifs_block *block)
{
    device_read(dev, block, sizeof(struct ifs_block), sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)));
}

static int ifs_get_last_block(struct device *dev, int start)
{
    struct ifs_block block;
    int ret = start;

    ifs_get_block(dev, start, &block);
    do {
        ret = block.next;
        ifs_get_block(dev, block.next, &block);
    } while (block.next != 0);
    return ret;
}

/*
 * Returns the address of an BlockEntry stored
 * within the IFS Block pool
 */
static int ifs_get_address(struct device *dev, int index)
{
    struct ifs_block block;

    ifs_get_block(dev, index, &block);
    return block.data;
}

/*
 * Returns the index to a directory table
 */
static int ifs_get_directory(struct device *dev, int parent, char *dir)
{
    if(dir[0] == 0 && parent == 0)
        return 0;
    
    bool has_sep = contains(dir, '/');
    
	int32_t directory[256];
    struct ifs_entry dentry;

    device_read(dev, &dentry, sizeof(struct ifs_entry), ifs_get_address(dev, parent));
    int dtable = dentry.data_index;
    device_read(dev, directory, 1024, ifs_get_address(dev, dtable));

    for (int i = 0; i < 256 && directory[i] != -1; i++) {
        int e = directory[i];
        struct ifs_entry entry;
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, e));

        if (has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0) {
            return ifs_get_directory(dev, e, strrchr(dir, '/') + 1);
        } else if (!has_sep) {
            if (strncmp(entry.file_name, dir, strlen(entry.file_name)) == 0) {
                return e;
            }
        }
    }
    return -1;
}

static int ifs_read_ino(struct device *dev, struct inode *inos, const char *path)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
        inos->i_ino = ino;
        inos->i_size = entry.file_size;
        inos->i_dev = dev->dev_id;
        inos->i_mode = entry.mode;
        inos->i_uid = entry.uid;
        inos->i_gid = entry.gid;
        inos->i_islnk = entry.file_type == IFS_LINK;
        inos->i_isfifo = entry.file_type == IFS_PIPE;
        return 0;
    } else {
        return -1;
    }
}

static int ifs_mkdir(struct device *dev, const char *path, mode_t mode)
{
	int d_block = ifs_block_alloc(dev, 1024);
	struct ifs_entry dir;
	dir.mode = mode;
	dir.file_type = IFS_DIRECTORY;
	dir.data_index = d_block;
	dir.file_size = 1024;
    dir.uid = getuid();
    dir.gid = getgid();
	strcpy(dir.file_name, basename(path));
	int e = ifs_insert_entry(dev, path, &dir);
	int32_t files[256];
	memset(files, 0xFF, 1024);
	device_write(dev, files, 1024, ifs_get_address(dev, d_block));
}

static int ifs_mkfifo(struct device *dev, const char *path, mode_t mode)
{
	struct ifs_entry dir;
	dir.mode = mode;
	dir.file_type = IFS_PIPE;
	dir.data_index = 0;
	dir.file_size = 0;
    dir.uid = getuid();
    dir.gid = getgid();
	strcpy(dir.file_name, basename(path));
	ifs_insert_entry(dev, path, &dir);
}

static int ifs_creat(struct device *dev, const char *path, mode_t mode)
{
	int d_block = ifs_block_alloc(dev, 1024);
	struct ifs_entry dir;
	dir.mode = mode;
	dir.file_type = IFS_DIRECTORY;
	dir.data_index = d_block;
	dir.file_size = 0;
    dir.uid = getuid();
    dir.gid = getgid();
	strcpy(dir.file_name, basename(path));
	int e = ifs_insert_entry(dev, path, &dir);
	int32_t files[256];
	memset(files, 0xFF, 1024);
	device_write(dev, files, 1024, ifs_get_address(dev, d_block));
}

static int ifs_unlink(struct device *dev, const char *path)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
        if(entry.file_type == IFS_DIRECTORY) return EISDIR;
        ifs_remove_entry(dev, path, &entry);
        ifs_free_blockgroup(dev, &entry);
        ifs_free_block(dev, ino);
        return 0;
    }
    return ENOENT;
}

static int ifs_rmdir(struct device *dev, const char *path)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
        if(entry.file_type != IFS_DIRECTORY) return ENOTDIR;
        ifs_remove_entry(dev, path, &entry);
        ifs_free_blockgroup(dev, &entry);
        ifs_free_block(dev, ino);
        return 0;
    }
    return ENOENT;
}

/*
 * Frees all data associated with a file
 */
static int ifs_free_blockgroup(struct device *dev, struct ifs_entry *entry)
{
    int blk = entry->data_index;
    do {
        struct ifs_block block;
        ifs_get_block(dev, blk, &block);
        ifs_free_block(dev, blk);
        blk = block.next;
    } while(blk);
}


static int ifs_readlink(struct device *dev, const char *path, char *buf, int len)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
        if(entry.file_type == IFS_LINK) {
            device_read(dev, buf, len % 1024, ifs_get_address(dev, entry.data_index));
            return 0;
        }
    }
    return ENOENT;
}


static int ifs_symlink(struct device *dev, const char *from, const char *to)
{
	int d_block = ifs_block_alloc(dev, 1024);
	struct ifs_entry dir;
	dir.mode = 0444;
	dir.file_type = IFS_LINK;
	dir.data_index = d_block;
	dir.file_size = 1024;
    dir.uid = getuid();
    dir.gid = getgid();
	strcpy(dir.file_name, basename(from));
	int e = ifs_insert_entry(dev, from, &dir);
	device_write(dev, to, (strlen(to) + 1) % 1024, ifs_get_address(dev, d_block));
    return 0;
}

static int ifs_chmod(struct device *dev, const char *path, mode_t newmode)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, sizeof(struct ifs_entry), ifs_get_address(dev, ino));
        entry.mode = newmode;
        device_write(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
        return 0;
    }
    return ENOENT;
}
