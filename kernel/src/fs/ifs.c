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
#include <infinity/virtfs.h>
#include <infinity/kernel.h>
#include <infinity/stat.h>
#include <infinity/fs/ifs.h>

struct filesystem ifs_filesystem;

static void ifs_get_block(struct device *dev, int index, struct ifs_block *block);
static int ifs_get_last_block(struct device *dev, int start);
static void ifs_write_block(struct device *dev, int index, struct ifs_block *block);
static int ifs_get_address(struct device *dev, int index);
static int ifs_get_directory(struct device *dev, int parent, char *dir);
static int ifs_get_file_entry(struct device *dev, int parent, char *dir, struct ifs_entry *dest);
static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct);
static int ifs_find_block(struct device *dev, int vsize, int block, int search, int pos);
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len);
static size_t ifs_write_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len);
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent);
static int ifs_open(struct device *dev, struct file *f, const char *path, int oflags);
static int ifs_read_ino(struct device *dev, struct inode *ino, const char *path);
static int ifs_mkdir(struct device *dev, const char *dir);

void register_ifs()
{
    memcpy(ifs_filesystem.fs_name, "ifs", 4);
    ifs_filesystem.read = ifs_read_file;
    ifs_filesystem.write = ifs_write_file;
    ifs_filesystem.open = ifs_open;
    ifs_filesystem.readdir = ifs_read_dir;
    ifs_filesystem.fstat = ifs_fstat;
    ifs_filesystem.mkdir = ifs_mkdir;
    ifs_filesystem.readino = ifs_read_ino;
    register_fs(&ifs_filesystem);
}

static int ifs_mount(struct device *dev)
{
    struct ifs_volume_hdr hdr;


    device_read(dev, &hdr, 0, sizeof(struct ifs_volume_hdr));

    /*
     * Yes Courtney, even though you completely fucked me over
     * in just about every respect, I immortalized you in the IFS
     * magic number. Not that you will ever see this. Maybe because
     * I still love you, maybe because, well I have no fucking clue....
     *
     * Magic number: 0xCB0A0D0D (Big endian)
     */
    if (hdr.mag0 != 0xCB || hdr.mag1 != 0x0A || hdr.mag2 != 0x0D || hdr.mag3 != 0x0D)
        return -1;

    return 1;
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
        *strrchr(tmp, '/') = 0;
        parent = ifs_get_directory(dev, 0, tmp);
    }
    return parent;
}

/*
 * Allocates a block in the IFS block pool with a
 * specified size
 */
static int ifs_block_alloc(struct device *dev, int size)
{
    struct ifs_block blk;
    struct ifs_volume_hdr hdr;

    device_read(dev, &hdr, 0, sizeof(struct ifs_volume_hdr));

    for (int i = 0; i < hdr.block_pool_size; i += sizeof(struct ifs_block)) {
        device_read(dev, &blk, sizeof(struct ifs_volume_hdr) + i, sizeof(struct ifs_block));
        if ((blk.state == IFS_BLOCK_FREE && blk.size == size) || blk.state == IFS_BLOCK_NONEXISTENT) {
            blk.size = size;
            blk.state = IFS_BLOCK_ALLOCATED;
            blk.data = hdr.placement_new;
            hdr.placement_new += size;
            device_write(dev, &hdr, 0, sizeof(struct ifs_volume_hdr));
            device_write(dev, &blk, sizeof(struct ifs_volume_hdr) + i, sizeof(struct ifs_block));
            return i / sizeof(struct ifs_block);
        }
    }
    return -1;
}


static void ifs_write_block(struct device *dev, int index, struct ifs_block *block)
{
    device_write(dev, block, sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)), sizeof(struct ifs_block));
}


static int ifs_fstat(struct device *dev, ino_t ino, struct stat *stat_struct)
{
    struct ifs_entry entry;

    device_read(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
    stat_struct->st_mode = entry.umask;
    stat_struct->st_ctime = entry.created_time;
    stat_struct->st_mtime = entry.modified_time;
    stat_struct->st_atime = entry.modified_time;
    stat_struct->st_ino = ino;
    stat_struct->st_size = entry.file_size;
}

/*
 * Reads a file, copies data into buff
 */
static size_t ifs_read_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len)
{
    struct ifs_entry entry;
    struct ifs_volume_hdr vol_header;
    size_t bytes_read = 0;

    device_read(dev, &vol_header, 0, sizeof(struct ifs_volume_hdr));
    device_read(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
    
    int i = 0;
    int offset = addr % vol_header.file_block_size;
    int blk = entry.data_index;
    int start = addr - addr % vol_header.file_block_size;
    int end = addr + len;
    do {
        struct ifs_block block;
        ifs_get_block(dev, blk, &block);
        if(i >= start && i < end) {
            int rlen = i + vol_header.file_block_size > len ? len % vol_header.file_block_size : vol_header.file_block_size;

            bytes_read += device_read(dev, buff + bytes_read, block.data + offset, rlen);
            offset = 0;
        }
        i += vol_header.file_block_size;
        blk = block.next;
        
    } while(blk);
    
    return bytes_read;
}

static size_t ifs_write_file(struct device *dev, ino_t ino, char *buff, off_t addr, size_t len)
{
    struct ifs_entry entry;
    struct ifs_volume_hdr vol_header;

    device_read(dev, &vol_header, 0, sizeof(struct ifs_volume_hdr));
    device_read(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
    int offset = addr - ((addr / vol_header.file_block_size) * vol_header.file_block_size);
    int bytes_written = 0;

    int blocks = (entry.file_size + entry.file_size % vol_header.file_block_size) / vol_header.file_block_size;
    int blocks_needed = (entry.file_size + entry.file_size % vol_header.file_block_size) / vol_header.file_block_size;
    int delta = blocks_needed > blocks ? blocks_needed - blocks : 0;
    struct ifs_block last;
    int lastp = ifs_get_last_block(dev, entry.data_index);
    ifs_get_block(dev, lastp, &last);

    for (int i = 0; i < delta; i++) {
        struct ifs_block prev;
        int nb = ifs_block_alloc(dev, vol_header.file_block_size);
        last.next = nb;
        ifs_write_block(dev, lastp, &last);
        lastp = nb;
    }

    for (int i = 0; i < len && i < entry.file_size; i += vol_header.file_block_size) {
        int block = len / vol_header.file_block_size;
        int rlen = len - i;
        device_write(dev, buff + i, offset + ifs_get_address(dev, ifs_find_block(dev, vol_header.file_block_size, entry.data_index, addr + i, 0)), rlen);
        offset = 0;
        bytes_written += rlen;
    }

    entry.file_size = addr + bytes_written;
    device_write(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
    return bytes_written;
}

/*
 * Blocks containing file data form a linked list (There is no guarantee that the
 * file data is all together, so this function will return a block index for a
 * offset inside a file
 */
static int ifs_find_block(struct device *dev, int vsize, int block, int search, int pos)
{
    struct ifs_block parent;

    ifs_get_block(dev, block, &parent);
    if (search < pos + vsize)
        return block;
    else
        return ifs_find_block(dev, vsize, parent.next, search, pos + parent.size);
}

/*
 * Reads an IFS directory entry
 */
static int ifs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent)
{
    struct ifs_entry entry;
    int32_t *directory = (int32_t *)kalloc(1024);

    device_read(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
    device_read(dev, directory, ifs_get_address(dev, entry.data_index), 1024);

    if (directory[d] == -1)
        return -1;
    device_read(dev, &entry, ifs_get_address(dev, directory[d]), sizeof(struct ifs_entry));
    dent->d_ino = directory[d];
    
    switch(entry.file_type) {
        case IFS_DIRECTORY:
            dent->d_type = DT_DIR;
            break;
        case IFS_REG_FILE:
            dent->d_type = DT_REG;
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
    device_read(dev, block, sizeof(struct ifs_volume_hdr) + (index * sizeof(struct ifs_block)), sizeof(struct ifs_block));
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
    /*
     * We could just make allocate directory on the stack, but
     * allocating over 1KB of local space in a recursive method
     * sounds like a bad idea to me
     */
    int32_t *directory = (int32_t *)kalloc(1024);
    struct ifs_entry dentry;

    device_read(dev, &dentry, ifs_get_address(dev, parent), sizeof(struct ifs_entry));
    int dtable = dentry.data_index;
    device_read(dev, directory, ifs_get_address(dev, dtable), 1024);

    for (int i = 0; i < 256 && directory[i] != -1; i++) {
        int e = directory[i];
        struct ifs_entry entry;
        device_read(dev, &entry, ifs_get_address(dev, e), sizeof(struct ifs_entry));

        if (has_sep && strncmp(dir, entry.file_name, strindx(dir, '/')) == 0) {
            kfree(directory);
            return ifs_get_directory(dev, e, strrchr(dir, '/') + 1);
        } else if (!has_sep) {
            if (strncmp(entry.file_name, dir, strlen(entry.file_name)) == 0) {
                kfree(directory);
                return e;
            }
        }
    }
    kfree(directory);
    return -1;
}

static int ifs_read_ino(struct device *dev, struct inode *inos, const char *path)
{
    struct ifs_entry entry;
    if (ifs_get_directory(dev, 0, path) != -1) {
        ino_t ino = ifs_get_directory(dev, 0, path);
        device_read(dev, &entry, ifs_get_address(dev, ino), sizeof(struct ifs_entry));
        inos->i_ino = ino;
        inos->i_size = entry.file_size;
        inos->i_dev = dev->dev_id;
        return 0;
    } else {
        return -1;
    }
}

static int ifs_open(struct device *dev, struct file *f, const char *path, int oflags)
{

}


static int ifs_mkdir(struct device *dev, const char *path)
{
    int ent = ifs_block_alloc(dev, sizeof(struct ifs_entry));
    int table = ifs_block_alloc(dev, 1024);

    if (ent == -1 || table == -1) {
        printk(KERN_ERR "ERROR ifs_mkdir() failed! Could not allocate block! Disk full or corrupt!\n");
        return -1;
    }

    struct ifs_entry dir;
    dir.umask = 484;
    dir.block_index = ent;

    if (contains(path, '/'))
        strcpy(dir.file_name, strrchr(path, '/') + 1);
    else
        strcpy(dir.file_name, path);

    int parent = ifs_get_parent(dev, path);

    int32_t *directory = (int32_t *)kalloc(1024);
    struct ifs_entry dentry;
    device_read(dev, directory, ifs_get_address(dev, parent), 1024);

    int pos;
    for (pos = 0; pos < 256 && directory[pos] != -1; pos++);

    directory[pos] = ent;

    device_write(dev, directory, ifs_get_address(dev, parent), 1024);
    device_write(dev, &dir, ifs_get_address(dev, ent), sizeof(struct ifs_entry));

    kfree(directory);
}
