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

#ifndef MODULE_H
#define MODULE_H


#include <stdint.h>

#define MODULE_MAGIC	0xCB01180E

typedef int (*mod_init_t)();
typedef int (*mod_uninit_t)();

struct module
{
	uint32_t 			mod_magic;
	char 				mod_name[128];
	mod_init_t 			mod_init;
	mod_uninit_t 		mod_uninit;
	struct module* 		next_module;
};

#endif
