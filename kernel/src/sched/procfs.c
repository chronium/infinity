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
 * procfs.c
 * My very simple implementation of a UNIX like procfs (As of right now
 * this is still a total WIP)
 */
 
#include <stdint.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/sched.h>
#include <infinity/dirent.h>
#include <infinity/virtfs.h>
#include <infinity/event.h>

struct procfs_entry {
    char                    e_name[256];
    uint32_t                e_type;
    struct procfs_entry *   e_entries;
    struct process *        e_proc;
    struct inode            e_ino;
    struct procfs_entry *   next;
};


static struct procfs_entry *procfs_root;

static void procfs_new_proc(uint32_t e, void *args);
static void procfs_exit_proc(uint32_t e, void *args);
static void procfs_open_fd(uint32_t e, void *args);
static void procfs_close_fd(uint32_t e, void *args);

static void procfs_add_child(struct procfs_entry *parent, struct procfs_entry *child);
static void procfs_remove_child(struct procfs_entry *parent, struct procfs_entry *child);
static struct procfs_entry *procfs_get_entry(struct process *proc);
static int procfs_read_ino(struct device *dev, struct inode *ino, const char *path);
static int procfs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent);
static int procfs_read_status(struct file *f, char *buf, off_t off, size_t len);
static int procfs_fstat(struct device *dev, ino_t ino, struct stat *st);
static int procfs_unlink(struct device *dev, const char *path);
static struct procfs_entry *procfs_get_directory(struct procfs_entry *parent, const char *path);

struct filesystem procfs = {
    .readino = procfs_read_ino,
    .readdir = procfs_read_dir,
    .fstat = procfs_fstat,
    .unlink = procfs_unlink
};


void init_procfs()
{
    event_subscribe(PROCESS_CREATE, procfs_new_proc);
    event_subscribe(PROCESS_DESTROY, procfs_exit_proc);
    event_subscribe(FILDES_OPEN, procfs_open_fd);
    event_subscribe(FILDES_CLOSE, procfs_close_fd);
    
    strcpy(procfs.fs_name, "procfs");
    
    procfs_root = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    procfs_root->e_ino.i_ino = (ino_t)procfs_root;
    procfs_root->e_ino.i_dev = 0xCAB00;
    
    int res = virtfs_mount(NULL, &procfs, "/proc");
    if(res != 0) {
        printk(KERN_ERR "Could not mount procfs!\n");
    }
    printk(KERN_DEBUG, "Mounting procfs to /proc\n");
}

static inline int contains(const char *str, char c)
{
    for (int i = 0; str[i] != 0; i++)
        if (str[i] == c)
            return 1;
    return 0;
}

static void procfs_new_proc(uint32_t e, void *args)
{
    struct process *proc = (struct process*)args;
    struct procfs_entry *entry = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    struct procfs_entry *fd = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    struct procfs_entry *status = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    struct file *status_f = (struct file*)kalloc(sizeof(struct file));
    
    entry->e_proc = proc;
    entry->e_type = DT_DIR;
    fd->e_proc = proc;
    fd->e_type = DT_DIR;
    status->e_proc = proc;
    status->e_type = DT_REG;
    sprintf(entry->e_name, "%d", proc->p_id);
    sprintf(fd->e_name, "fd");
    sprintf(status->e_name, "status");
    
    procfs_add_child(procfs_root, entry);
    procfs_add_child(entry, fd);
    procfs_add_child(entry, status);
    status_f->f_ino = (struct inode*)kalloc(sizeof(struct inode));
    status_f->f_ino->i_ino = (ino_t)status;
    status_f->f_ino->i_dev = (ino_t)0xCAB00;
    status_f->f_fs = &procfs;
    status_f->read = procfs_read_status;
    
    add_to_file_table(status_f);
    
    printk(KERN_DEBUG "procfs: create new node for PID %d\n", proc->p_id);
}

static void procfs_exit_proc(uint32_t e, void *args)
{
    struct process *proc = (struct process*)args;
    struct procfs_entry *ent = procfs_get_entry(proc);
    procfs_remove_child(procfs_root, ent);
    printk(KERN_DEBUG "procfs: remove node for PID %d\n", proc->p_id);
}

static void procfs_open_fd(uint32_t i, void *args)
{
    struct fildes *fd = (struct fildes*)args;
    struct process *proc = (struct process*)fd->fd_owner;
    struct procfs_entry *ent = procfs_get_entry(proc);
    struct procfs_entry *fd_root = ent->e_entries;
    struct procfs_entry *e = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    sprintf(e->e_name, "%d", fd->fd_num);
    e->e_type = DT_LNK;
    /* TODO: We need to actually link it... */
    procfs_add_child(fd_root, e);
}

static void procfs_close_fd(uint32_t i, void *args)
{
    struct fildes *fd = (struct fildes*)args;
    struct process *proc = (struct process*)fd->fd_owner;
    struct procfs_entry *ent = procfs_get_entry(proc);
    char path[32];
    sprintf(path, "fd/%d", fd->fd_num);
    struct procfs_entry *e = procfs_get_directory(ent, path);
    procfs_remove_child(ent->e_entries, e);
}

static void procfs_add_child(struct procfs_entry *parent, struct procfs_entry *child)
{
    child->e_ino.i_ino = (ino_t)child;
    child->e_ino.i_dev = 0xCAB00;
    
    struct procfs_entry *i = parent->e_entries;
    if(i) {
        while(i->next) {
            i = i->next;
        }
        i->next = child;
    } else {
        parent->e_entries = child;
    }
}

static void procfs_remove_child(struct procfs_entry *parent, struct procfs_entry *child)
{
    if(child && parent) {
        struct procfs_entry *i = parent->e_entries;
        if(i != child) {
            struct procfs_entry *last = NULL;

            while(i) {
                if(i == child) {
                    last->next = i->next;
                    break;
                }
                last = i;
                i = i->next;
            }
        } else {
            parent->e_entries = child->next;
        }
        
        i = child->e_entries;
        while(i) {
            procfs_remove_child(child, i);
            i = i->next;
        }
        kfree(child);
    }
}


static struct procfs_entry *procfs_get_entry(struct process *proc) 
{
    struct procfs_entry *i = procfs_root->e_entries;
    while(i) {
        if(i->e_proc == proc) {
            return i;
        }
        i = i->next;
    } 
    return NULL;
}

static int procfs_read_ino(struct device *dev, struct inode *ino, const char *path)
{
    struct procfs_entry *ent = procfs_get_directory(procfs_root, path);
    if(ent != NULL) {
        memcpy(ino, &ent->e_ino, sizeof(struct inode));
        return 0;
    }
    return -1;
}

static int procfs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent)
{
    struct procfs_entry *dir = (struct procfs_entry*)ino;
    int j = 0;
    struct procfs_entry *i = dir->e_entries;
    while(i) {
        if(j == d) {
            dent->d_type = i->e_type;
            dent->d_ino = i;
            memcpy(dent->d_name, i->e_name, 256);
            return 0;
        }
        j++;
        i = i->next;
    }
    return -1;
}

static struct procfs_entry *procfs_get_directory(struct procfs_entry *parent, const char *path)
{
    if(path[0] == 0) {
        return parent;
    }
    
    int has_sep = contains(path, '/');
    
    struct procfs_entry *i = parent->e_entries;
    while(i) {
        if(has_sep && strncmp(path, i->e_name, strindx(path, '/')) == 0) {
            return procfs_get_directory(i, strrchr(path, '/') + 1);
        } else if (!has_sep) {
            if (strcmp(i->e_name, path) == 0) {
                return i;
            }
        }
        i = i->next;
    }
    return NULL;
}

static int procfs_fstat(struct device *dev, ino_t ino, struct stat *st)
{
    struct procfs_entry *dir = (struct procfs_entry*)ino;
    st->st_mode = 0644;//dir->e_ino.i_mode;
    return 0;
}

static int procfs_read_status(struct file *f, char *buf, off_t off, size_t len)
{
    struct procfs_entry *dir = (struct procfs_entry*)f->f_ino->i_ino;
    struct process *proc = (struct process*)dir->e_proc;
    char tmp_buf[512];
    memset(tmp_buf, 0, 512);
    sprintf(tmp_buf,    "Name:   %s\n"
                        "Pid:    %d\n"
                        "Uid:    %d\n"
                        "Gid:    %d\n",
                        proc->p_name, proc->p_id, proc->p_uid, proc->p_gid);
                        
    int slen = strlen(tmp_buf);
    int i = 0;
    for(i = 0; i < len && i + off < slen; i++) {
        buf[i] = tmp_buf[i + off];
    }
    return i;
}


static int procfs_unlink(struct device *dev, const char *path)
{
    return -1;
}
