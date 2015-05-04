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
 * panic.c
 * Brings the system down in the case of a fatal error or
 * exception
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <infinity/common.h>
#include <infinity/arch/pic.h>
#include <infinity/kernel.h>

static void display_registers(struct regs *regs);
static void display_message(const char *format, va_list argp);
static void shut_it_down_charlie_brown();

/*
 * Brings the kernel a swift halt displaying a 
 * message
 * @param format    The format of the message to display
 */
void panic(const char *format, ...)
{
    asm("cli");
    release_all_locks();
    va_list argp;
    va_start(argp, format);
    display_message(format, argp);
    va_end(argp);
    
    shut_it_down_charlie_brown();
}

/*
 * Brings the kernel a swift halt displaying a 
 * message AND CPU registers as an added bonus
 * @param format    The format of the message to display
 */
void panic_cpu(struct regs *r, const char *format, ...)
{
    release_all_locks();
    va_list argp;
    va_start(argp, format);
    display_message(format, argp);
    va_end(argp);
    display_registers(r);
    shut_it_down_charlie_brown();
}

static void display_message(const char *format, va_list argp)
{
    klog(0); // Disable the kernel log because the heap could be corrupt
    char msg_buff[512];
    memset(msg_buff, 0, 512);
    vsprintf(msg_buff, format, argp);
    printk(KERN_EMERG "kernel panic: %s\n\n", msg_buff);
}

static void display_registers(struct regs *regs)
{
    char bad[64];
    int offset = rresolve_ksym(regs->eip, bad);
    if(offset != -1) {
        printk(KERN_EMERG "EIP is at <%s+0x%x>\n", bad, offset);
    } else {
        printk(KERN_EMERG "EIP is at <0x%x>\n", regs->eip);
    }
    printk(KERN_EMERG "EAX: %p EBX: %p ECX: %p EDX: %p\n", regs->eax, regs->ebx, regs->ecx, regs->edx); 
    printk(KERN_EMERG "ESI: %p EDI: %p EBP: %p ESP: %p\n\n", regs->esi, regs->edi, regs->ebp, regs->esp); 
}

static void shut_it_down_charlie_brown()
{
    printk(KERN_EMERG "The system is HALTED!\n");
    while(1) {
        asm volatile ("cli");
        asm volatile ("hlt");
    }
}
