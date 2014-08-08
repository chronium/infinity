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

#ifndef PROCESS_H
#define PROCESS_H


#include <infinity/types.h>
#include <infinity/paging.h>
#include <infinity/interrupt.h>

typedef struct process_image_t process_image_t;
typedef struct process_t process_t;



struct process_image_t
{
	void* sig_handlers[256];
	page_directory_t* page_directory;
	uid_t uid;
	gid_t gid;
	uint32_t kernel_stack;
	uint32_t stack_base;
	uint32_t image_base;
	uint32_t image_brk;
	uint32_t paged;
	process_image_t* next_image;
	registers_t previous_state;
};

struct process_t
{
	pid_t pid;
	pid_t parent_pid;
	process_image_t image;
	registers_t register_context;
	process_t* next_proc;

};
#endif
