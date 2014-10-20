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
 * module.c
 * Provides methods for loading kernel modules
 */

#include <stddef.h>
#include <infinity/device.h>
#include <infinity/module.h>

struct module *kmodule_list = NULL;

/*
 * Inserts a kernel module, returns 0 if success
 * @param mod	The module to insert
 */
int insert_module(struct module *mod)
{
	if (mod->mod_magic == MODULE_MAGIC) {
		mod->next_module = NULL;
		if (kmodule_list) {
			struct module *curr = kmodule_list;
			while (curr->next_module)
				curr = curr->next_module;
			curr->next_module = mod;
		} else {
			kmodule_list = mod;
		}
		return 0;
	} else {
		return -1;
	}
}

/*
 * Unitializes a kernel module
 * @param mod	The module to initialize
 */
int init_module(struct module *mod)
{
	return mod->mod_init();
}
