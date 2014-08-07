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

#include <stddef.h>
#include <stdint.h>
#include <infinity/kheap.h>


#define MAGIC		0x50BADA55
#define GUARD_1		0xCB0A0D0D
#define GUARD_2 	0xCB05160E

/*
 * NOTE: Remind myself to rewrite this, the current implemenation 
 * uses a linked list which works; however finding a free block
 * is quite slow. If I use two seperate stacks (One for free blocks,
 * one for allocated blocks) speed should be that of O(1) (I think)
 */
 
mblock_t* mlist;			// Heap linked list

void* freeaddress;			// The next free address

static void* kheap_alloc(size_t size);
static mblock_t* kheap_get_block(size_t size);

/*
 * Initiliazes the kernel heap using a custom placement address
 */

void init_kheap(uint32_t i)
{
	freeaddress = ((int)i % 8 ) + i;
	mlist = NULL;
}



void* kalloc(size_t size)
{
	mblock_t* new_block = kheap_get_block(size);
	if(new_block->state == MBLOCK_ALLOCATED)
		new_block->memory = kheap_alloc(size);
	else
		new_block->state = MBLOCK_ALLOCATED;
	return new_block->memory;
}

/*
 * Allocates memory in the kernel heap
 */
static void* kheap_alloc(size_t size)
{

	size_t real_size = size + (size % 8) + 8;

	void* address = freeaddress;
	freeaddress += real_size;
	*((int*)address) = GUARD_1;
	*((int*)(address + real_size - 4)) = GUARD_2;
	return address + 4;
}


static mblock_t* kheap_get_block(size_t size)
{
	size = size + (size % 8);
	mblock_t* i = mlist;
	mblock_t* last = i;
	mblock_t* ret = NULL;
	uint32_t smallest = -1;
	while(i)
	{
		if(i->size >= size && i->size < smallest && i->state == MBLOCK_FREE)
		{
			smallest = i->size;
			ret = i;
		}
		last = i;
		i = i->next_block;
	}
	if(ret)
		return ret;
	else
	{
		mblock_t* new_block = (mblock_t*)kheap_alloc(sizeof(mblock_t));
		new_block->size = size;
		new_block->magic = MAGIC;
		new_block->state = MBLOCK_ALLOCATED;
		new_block->next_block = NULL;
		if(mlist == NULL)
			mlist = new_block;
		else
			last->next_block = new_block;
		return new_block;
	}
}
/*
 * Reallocates an existing block of memory, with 
 * a new size
 */
void* realloc(void* ptr, size_t size)
{
	size_t org = ksize(ptr);
	kfree(ptr);
	void* ret_val = kalloc(size);
	memcpy(ret_val, ptr, org);
	return ret_val;
}

/*
 * Allocates a block of memory, returns page aligned
 * address
 */
void* malloc_pa(size_t size)
{
	
	void* old_free = freeaddress;
	freeaddress = ((uint32_t)freeaddress & 0xFFFFF000) + 0x1000;
	
	void* ret = freeaddress;
	freeaddress += size ;
	return ret;
}

				static int lck = 0;
/*
 * Frees a block of memory
 */
void kfree(void* ptr)
{
	mblock_t* fb = mlist;
	while(fb != NULL)
	{
		if(ptr >= fb->memory && ptr < fb->memory + fb->size && fb->state == MBLOCK_ALLOCATED)
		{
			if(*((int*)(fb->memory - 4)) != GUARD_1 || *((int*)(fb->memory + fb->size)) != GUARD_2)
				return; //panic("kernel heap corruption!");
			fb->state = MBLOCK_FREE;
			return;
		}
		fb = fb->next_block;
	} 
}

/*
 * Gets the size of a block of memory
 */
size_t ksize (void* ptr)
{
	mblock_t* fb = mlist;
	while(fb)
	{
		if(ptr >= fb->memory && ptr < fb->memory + fb->size && fb->state == MBLOCK_ALLOCATED)
		{
			return fb->size;
		}
		fb = fb->next_block;
	} 
	return 0;
}
