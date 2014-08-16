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
#include <infinity/virtfs.h>

struct process_image;
struct process;

struct process_image
{
	void* 					sig_handlers[256];
	struct page_directory* 	page_directory;
	uid_t 					uid;
	gid_t 					gid;
	uint32_t 				kernel_stack;
	uint32_t 				stack_base;
	uint32_t 				image_base;
	uint32_t 				image_brk;
	uint32_t 				paged;
	struct process_image* 	next_image;
	
};

struct process
{
	pid_t 					pid;
	pid_t 					parent_pid;
	struct process_image 	image;
	struct file_descriptor* file_descriptors;
	struct regs 			register_context;
	struct process* 		next_proc;

};


extern pid_t fork();
#endif
