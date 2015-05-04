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
 * kmain.c
 * Kernel entry point and initialization
 */

#include <mboot.h>
#include <stdint.h>
#include <infinity/elf32.h>
#include <infinity/fcntl.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/paging.h>
#include <infinity/fs.h>
#include <infinity/arch/idt.h>
#include <infinity/arch/gdt.h>
#include <infinity/sched.h>
#include <infinity/sync.h>
#include <infinity/module.h>
#include <infinity/event.h>
#include <infinity/syscalls.h>
#include <infinity/procfs.h>
#include <infinity/drivers/pit.h>
#include <infinity/drivers/textscreen.h>
#include <infinity/drivers/serial.h>

static void kthread_main();

static void init_term(multiboot_info_t *mbootinfo);

/*
 * Start of kernel execution, we are transferred here
 * from boot.asm
 */
void kmain(multiboot_info_t *mbootinfo)
{
    void *heap_start = *(uint32_t *)(mbootinfo->mods_addr + 4);
    void *module_start = *(uint32_t *)(mbootinfo->mods_addr);
    
    init_kheap(heap_start);
    init_gdt();
    init_idt();
    init_paging();
    klog(1);
    init_serial();
    init_pit(50);
    klog_output(serial_dev1);
    init_ramdisk(module_start, 0);
    init_procfs();
    init_devfs();
    init_fb((vbe_info_t*)mbootinfo->vbe_mode_info);
    init_sched(kthread_main, mbootinfo);
    
    while (1);
}

/*
 * Entry point for the kernel thread (Once we have the
 * schedular enabled)
 */
static void kthread_main(multiboot_info_t *mbootinfo)
{
    parse_symbol_file();
    init_syscalls();
    printk(KERN_DEBUG "Kernel thread initialized\n");
    init_boot_modules();
    printk(KERN_DEBUG "Infinity kernel initialization complete. Going idle NOW!\n");
    event_dispatch(KERNEL_INIT, NULL);
    
    spawnve(P_DETACH, "/sbin/init", NULL, NULL);
    
    while (1) {
        asm ("hlt"); // We are done. Stay here
    }
}


static void init_term(multiboot_info_t *mbootinfo)
{
}
