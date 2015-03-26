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
 * module.c
 * Provides functions loading and unloading kernel modules
 */
  

#include <stddef.h>
#include <infinity/heap.h>
#include <infinity/kernel.h>
#include <infinity/elf32.h>
#include <infinity/virtfs.h>
#include <infinity/fcntl.h>
#include <infinity/module.h>

static struct module *module_list = NULL;

static void add_module(struct module *mod);

/*
 * Loads a module and returns a struct module
 * Note, the module will not be initialized
 * @param path			The path to the kernel module
 */
struct module *load_module(const char *path)
{
    printk(KERN_INFO "INFO: Loading kernel module '%s'\n", path);
    void *exe = elf_open(path);
    if(exe) {
        struct module *mod = (struct module*)kalloc(sizeof(struct module));
        mod->mod_init = elf_sym(exe, "mod_init");
        mod->mod_uninit = elf_sym(exe, "mod_uninit");
        
        if(mod->mod_init == NULL || mod->mod_uninit == NULL) {
            printk(KERN_ERR "ERROR: Kernel module missing mod_init or mod_uninit, invalid kernel module! (File: %s)\n", path);
            return NULL;
        }
        return mod;
        
    } else {
        return NULL;
    }
}

/*
 * Inserts a kernel module into the linked list of
 * active modules and initializes it
 * @param mod			The module to initialize
 */
void insert_mod(struct module *mod)
{
    add_module(mod);
    
    int res = mod->mod_init();
    if(res) {
        printk(KERN_ERR "ERROR: Failed to initialize module %s, mod_init() returned %d\n", mod->mod_name, res);
    } else {
        printk(KERN_INFO "DEBUG: Returned %d\n", res);
    }
}

void unload_mod(struct module *mod)
{
    
}

/*
 * Loads all modules needed to boot infinity
 */
void init_boot_modules()
{
    struct file *f = fopen("/lib/infinity/modules", O_RDONLY);
    if(f) {

        struct dirent entry;
        int i = 0;
        while(virtfs_readdir(f, i, &entry) == 0) {
            char file_name[256];
            memset(file_name, 0, 256);
            strcat(file_name, "/lib/infinity/modules/");
            strcat(file_name, entry.d_name);
            
            struct module *mod = load_module(file_name);
            if(mod) {
                insert_mod(mod);
            } else {
                printk(KERN_ERR "ERROR: Could not load module %s\n", entry.d_name);
            }
            i++;
        }
        
        fclose(f);
    } else {
        printk(KERN_WARN "WARNING: Could not open up /lib/infinity/modules! Booting NO kernel modules!\n");
    }
}

/*
 * Adds a module to the linked list
 */
static void add_module(struct module *mod)
{
    if(module_list) {
        struct module *i = module_list;
        while(i->next) {
            i = i->next;
        }
        i->next = mod;
    } else {
        module_list = mod;
    }
}
