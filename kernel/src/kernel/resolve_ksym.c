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
#include <infinity/virtfs.h>
#include <infinity/types.h>

struct kernel_symbol {
	char						s_name[64];
	caddr_t						s_addr;
	struct kernel_symbol		*next;
};

static struct kernel_symbol *symbol_list = NULL;
static void ksym_parse(const char *line);
static void ksym_read_line(struct file *f, char *buf);
static void ksym_add_to_list(struct kernel_symbol *ksym);

/*
 * Begins parsing the kernel symbol file and constructs a 
 * linked list containing every symbol
 */
void parse_symbol_file()
{
	struct file *f = fopen("/infinity.map", O_RDWR);
	
	if(f == NULL) {
		printk(KERN_WARN "[WARNING] Could not load kernel symbols!\n");
	} else {
		char line[512];
		while(f->f_len > f->f_pos) {
			ksym_read_line(f, line);
			ksym_parse(line);
		}
		fclose(f);
	}
}

static void ksym_parse(const char *line)
{
	char address[9];
	memset(address, 0, 9);
	memcpy(address, line, 8);
	caddr_t addr = strtol(address, 16);
	char type = line[9];
	char *name = &line[11];
	struct kernel_symbol *ksym = (struct kernel_symbol*)kalloc(sizeof(struct kernel_symbol));
	memcpy(ksym->s_name, name, strlen(name));
	ksym->s_addr = addr;
	ksym->next = NULL;
	ksym_add_to_list(ksym);
}

static void ksym_read_line(struct file *f, char *buf)
{
	int i = 0;
	char dat = 0;
	while(dat != '\n' && f->f_len > f->f_pos) {
		if(dat) buf[i++] = dat;
		virtfs_read(f, &dat, 0, 1);
	}
	buf[i] = 0; 	
}

void *resolve_ksym(const char *name)
{
	struct kernel_symbol *sym = symbol_list;
	while(sym) {
		if(strcmp(name, sym->s_name) == 0) 
			return (void*)sym->s_addr;
		sym = sym->next;
	}
	return NULL;
}


static void ksym_add_to_list(struct kernel_symbol *ksym)
{
	if(symbol_list == NULL) 
		symbol_list = ksym;
	else {
		struct kernel_symbol *sym = symbol_list;
		while(sym->next) {
			sym = sym->next;
		}
		sym->next = ksym;
	}
}
