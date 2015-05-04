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
 * resolve_ksym.c
 * Provides functions for loading and resolving kernel symbols
 */

#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/kernel.h>
#include <infinity/fcntl.h>
#include <infinity/fs.h>
#include <infinity/types.h>

struct kernel_symbol {
    char                    s_name[64];
    caddr_t                 s_addr;
    struct kernel_symbol *  next;
};

static struct kernel_symbol *symbol_list = NULL;
static void ksym_parse(const char *line);
static int ksym_read_line(int fd, char *buf);
static void ksym_add_to_list(struct kernel_symbol *ksym);

/*
 * Begins parsing the kernel symbol file and constructs a
 * linked list containing every symbol
 */
void parse_symbol_file()
{
    int fd = open("/infinity.map", O_RDWR);

    if (fd == -1) {
        printk(KERN_WARN "Could not load kernel symbols!\n");
    } else {
        char line[512];
        ksym_read_line(fd, line);
        while (ksym_read_line(fd, line)) {
            ksym_parse(line);
        }
        close(fd);
    }
    
}

/*
 * Parse a line in the linker map file
 */
static void ksym_parse(const char *line)
{
    char address[9];
    memset(address, 0, 9);
    memcpy(address, line, 8);
    caddr_t addr = strtol(address, 16);
    char type = line[9];
    char *name = &line[11];
    struct kernel_symbol *ksym = (struct kernel_symbol *)kalloc(sizeof(struct kernel_symbol));
    memcpy(ksym->s_name, name, strlen(name));
    ksym->s_addr = addr;
    ksym->next = NULL;
    ksym_add_to_list(ksym);
}

/*
 * Read a single line from the linker map file
 */ 
static int ksym_read_line(int fd, char *buf)
{
    int i = 0;
    char dat = 0;
    while(read(fd, &dat, 1) && dat && dat != '\n') {
        buf[i++] = dat;
    }
    buf[i] = 0;
    return i;
}

/*
 * Resolves the value for a specific symbol
 */
void *resolve_ksym(const char *name)
{
    struct kernel_symbol *sym = symbol_list;

    while (sym) {
        if (strcmp(name, sym->s_name) == 0)
            return (void *)sym->s_addr;
        sym = sym->next;
    }
    return NULL;
}

/*
 * Gets the name of a symbol based on its 
 * address in memory
 * @param addr      The address in question
 * @buf             A buffer to store the closest
 *                  symbol
 */
int rresolve_ksym(int addr, char *buf)
{
    struct kernel_symbol *sym = symbol_list;
    struct kernel_symbol *closest = NULL;
    int distc;
    
    while (sym) {
        if(sym->s_addr <= addr) {
            if(closest) {
                int disti = addr - sym->s_addr; 
                distc = addr - closest->s_addr;
                if(disti < distc) {
                    closest = sym;
                }
            } else {
                closest = sym;
            }
        }
        sym = sym->next;
    }
    
    if(closest) {
        strcpy(buf, closest->s_name);
        return distc;
    } else {
        return -1;
    }
}

/*
 * Adds a struct kernel_symbol to the symbol list
 */
static void ksym_add_to_list(struct kernel_symbol *ksym)
{
    if (symbol_list == NULL) {
        symbol_list = ksym;
    } else {
        struct kernel_symbol *sym = symbol_list;
        while (sym->next)
            sym = sym->next;
        sym->next = ksym;
    }
}
