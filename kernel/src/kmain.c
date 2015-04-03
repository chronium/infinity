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
#include <infinity/virtfs.h>
#include <infinity/idt.h>
#include <infinity/gdt.h>
#include <infinity/sched.h>
#include <infinity/sync.h>
#include <infinity/module.h>
#include <infinity/drivers/pit.h>
#include <infinity/drivers/textscreen.h>
#include <infinity/drivers/serial.h>

static void kthread_main();

/*
 * Start of kernel execution, we are transferred here
 * from boot.asm
 */
void kmain(multiboot_info_t *mbootinfo)
{
    void *heap_start = *(uint32_t *)(mbootinfo->mods_addr + 4);
    void *module_start = *(uint32_t *)(mbootinfo->mods_addr);

    init_kheap(heap_start);
    init_textscreen();
    init_serial();
    klog(1);
    klog_output(serial_dev1);
    init_fb((vbe_info_t*)mbootinfo->vbe_mode_info);
    init_ramdisk(module_start, 0);
    parse_symbol_file();
    init_gdt();
    init_idt();
    init_devfs();
    init_pit(50);
    // init_paging();
    init_sched();

    thread_create(kthread_main, NULL);

    while (1) ; // Wait for the scheduler to take over
}

/*
 * Entry point for the kernel thread (Once we have the
 * schedular enabled)
 */
static void kthread_main()
{
    struct file *test = fopen("/hello.txt", O_RDWR);


    printk(KERN_DEBUG "DEBUG: Kernel thread initialized\n");
    init_boot_modules();
    printk(KERN_DEBUG "DEBUG: Infinity kernel initialization complete. Going idle NOW!\n");
    while (1) asm ("hlt"); // We are done. Stay here
}
