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
 * memmanager.c
 * The kernel memory manager, managages page frame
 * allocation
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/paging.h>
#include <infinity/memmanager.h>

extern struct page_directory *current_directory;

static struct page_frame *free_frame_stack = NULL;
static struct page_frame *allocated_frame_stack = NULL;

static void *phys_free_address = 0x4000000;

static struct page_frame *pop_free_frame();

/*
 * Allocates a frame at a specified virtual address
 * and sets the flags.
 * @param vaddr		The address in virtual memory
 * @param flags		Page frame flags
 */
void *frame_alloc(void *vaddr, int flags)
{
	struct page_frame *frame = pop_free_frame();

	if (!frame) {
		frame = (struct page_frame *)kalloc(sizeof(struct page_frame));
		frame->phys_addr = phys_free_address;
		phys_free_address += 0x1000;
	}
	frame->ref_count = 1;
    printk(KERN_INFO "Map %x to %x\n", vaddr, frame->phys_addr);
	page_alloc(current_directory, vaddr, frame->phys_addr, flags & 1, (flags & 2) >> 1);
	frame->virt_addr = (uint32_t)vaddr;
	frame->last_frame = allocated_frame_stack;
	frame->page_directory = current_directory;
	allocated_frame_stack = frame;
}

/*
 * Frees a frame
 * @param vaddr		A virtual address inside the frame to free
 */
void frame_free(void *vaddr)
{
	vaddr = (uint32_t)vaddr;
	struct page_frame *curr = allocated_frame_stack;
	struct page_frame *prev = curr;
	while (curr) {
		if (curr->virt_addr == vaddr && curr->page_directory == current_directory) {
			curr->ref_count = 0;
			if (prev == curr)
				allocated_frame_stack = curr->last_frame;
			else
				prev->last_frame = curr->last_frame->last_frame = prev->last_frame;
			curr->last_frame = free_frame_stack;
			free_frame_stack = curr;
			page_free(current_directory, vaddr);
			return;
		}
		prev = curr;
		curr = curr->last_frame;
	}
}


static struct page_frame *pop_free_frame()
{
	struct page_frame *curr = free_frame_stack;

	if (!curr) {
		return NULL;
	} else {
		free_frame_stack = curr->last_frame;
		return curr;
	}
}
