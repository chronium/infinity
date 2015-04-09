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


#ifndef INFINITY_INFINITY_H
#define INFINITY_INFINITY_H

#include <infinity/time.h>
#include <infinity/device.h>

#define KERN_EMERG      "<0>"
#define KERN_ALERT      "<1>"
#define KERN_ERR        "<2>"
#define KERN_WARN       "<3>"
#define KERN_INFO       "<4>"
#define KERN_DEBUG      "<5>"

typedef enum {
    LOG_KERN_DEBUG =    0,
    LOG_KERN_INFO =     1,
    LOG_KERN_WARN =     2,
    LOG_KERN_ERR =      3,
    LOG_KERN_ALERT =    4,
    LOG_KERN_EMERG =    5
} loglevel_t;

struct kernel_msg;

struct kernel_msg {
    loglevel_t          log_level;
    struct tm           msg_tm;
    char                msg_string[512];
    struct kernel_msg * next;
};

void parse_symbol_file();
void klog(int log);
void klog_output(struct device *dev);
void printk(const char *format, ...);
void panic(const char *format, ...);
void panic_cpu(struct regs *r, const char *format, ...);
void *resolve_ksym(const char *name);
int rresolve_ksym(int addr, char *buf);
int execvpe(const char *path, const char *argv[], const char *envp[]);

#endif
