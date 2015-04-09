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
    struct procfs_entry *   next;
};


static struct procfs_entry *procfs_root;

static void procfs_new_proc(uint32_t e, void *args);
static void procfs_exit_proc(uint32_t e, void *args);

static void procfs_add_child(struct procfs_entry *parent, struct procfs_entry *child);
static void procfs_remove_child(struct procfs_entry *parent, struct procfs_entry *child);
static struct procfs_entry *procfs_get_entry(struct process *proc);
static int procfs_open(struct device *dev, struct file *f, const char *path, int oflag);
static int procfs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent);

struct filesystem procfs = {
    .open = procfs_open,
    .readdir = procfs_read_dir
};


void init_procfs()
{
    event_subscribe(PROCESS_CREATE, procfs_new_proc);
    event_subscribe(PROCESS_DESTROY, procfs_exit_proc);
    
    procfs_root = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    int res = virtfs_mount(NULL, &procfs, "/proc");
    printk(KERN_DEBUG, "Mounting procfs to /proc\n");
}


static void procfs_new_proc(uint32_t e, void *args)
{
    struct process *proc = (struct process*)args;
    struct procfs_entry *entry = (struct procfs_entry*)kalloc(sizeof(struct procfs_entry));
    entry->e_proc = proc;
    sprintf(entry->e_name, "%d", proc->p_id);
    procfs_add_child(procfs_root, entry);
}

static void procfs_exit_proc(uint32_t e, void *args)
{
    struct process *proc = (struct process*)args;
    struct procfs_entry *ent = procfs_get_entry(proc);
    procfs_remove_child(procfs_root, ent);
}

static void procfs_add_child(struct procfs_entry *parent, struct procfs_entry *child)
{
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
    struct procfs_entry *i = parent->e_entries;
    if(i) {
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
    kfree(child);
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
static int procfs_open(struct device *dev, struct file *f, const char *path, int oflag)
{
    if(path[0] == 0) {
        f->f_ino = (ino_t)procfs_root;
        f->f_fs = &procfs;
        f->f_flags |= F_SUPPORT_READ;
        return 0;
    }
}

static int procfs_read_dir(struct device *dev, ino_t ino, int d, struct dirent *dent)
{
    struct procfs_entry *dir = (struct procfs_entry*)ino;
    
    int j = 0;
    struct procfs_entry *i = dir->e_entries;
    while(i) {
        if(j == d) {
            dent->d_type = DT_DIR;
            memcpy(dent->d_name, i->e_name, 256);
            return 0;
        }
        j++;
        i = i->next;
    }
    return -1;
}
