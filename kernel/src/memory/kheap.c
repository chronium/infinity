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
 * kheap.c
 * The kernel heap. This is the probably the most important file as it controls
 * memory allocation.
 */
 
#include <stddef.h>
#include <stdint.h>
#include <infinity/kheap.h>


#define MAGIC           0x50BADA55
#define GUARD_1         0xCB0A0D0D
#define GUARD_2         0xCB05160E


struct mblock *free_top;
struct mblock *used_top;

void *free_address;

static void *kheap_alloc(size_t size);
static struct mblock *kheap_get_block(size_t size);

/*
 * Initiliazes the kernel heap using a custom placement address
 * @param i		The end of kernel memory, where to place the heap
 */
void init_kheap(uint32_t i)
{
	free_address = ((int)i % 8) + i;
	free_top = NULL;
	used_top = NULL;
}

/*
 * Allocates space on the kernel heap. Returns an 8 byte aligned
 * pointer
 * @param size	How many bytes to allocated
 * @returns		An 8 byte aligned pointer
 */ 
void *kalloc(size_t size)
{
	struct mblock *new_mb = kheap_get_block(size);
	if(!new_mb) {
		new_mb = kheap_alloc(sizeof(struct mblock));
		new_mb->memory = kheap_alloc(size);
	}
	
	new_mb->next_block = used_top;
	new_mb->size = size;
	used_top = new_mb;
	return new_mb->memory;

}

static void *kheap_alloc(size_t size)
{
	size_t sz_aligned = size + (size % 8) + 8;
	
	*((int*)free_address) = GUARD_1;
	*((int*)(free_address + size + 4))= GUARD_2;

	free_address += sz_aligned;
	return 4 + free_address - sz_aligned;
}

static struct mblock *kheap_get_block(size_t size)
{
	struct mblock *i = free_top;
	struct mblock *p = i;
	while(i) {
		if(i->size >= size) {
			if(p == i)
				free_top = i->next_block;
			else 
				p->next_block = i->next_block;
			return i;
		}
		p = i;
		i = i->next_block;
	}
	return NULL;
}
/*
 * Reallocates an existing block of memory, with a new size
 * @param ptr	The pointer of the original memory block
 * @param size	The size of the new memory block
 * 
 * @returns		A pointer to a new block of memory, NULL if failure
 */
void *realloc(void *ptr, size_t size)
{
	void *tmp = ptr;
	size_t sz = ksize(tmp);
	if(sz != 0) {
		kfree(tmp);
		void *rtn = kalloc(size);
		memcpy(rtn, tmp, sz);
		return rtn;
	} else
		return 0;
}

/*
 * Allocates a block of memory, returns page aligned
 * address
 * @param size	Size of the memory block
 * @returns		A page aligned pointer to the new memory block
 */
void *malloc_pa(size_t size)
{
	void *old_free = free_address;

	free_address = ((uint32_t)free_address & 0xFFFFF000) + 0x1000;

	void *ret = free_address;
	free_address += size;
	return ret;
}

/*
 * Frees a block of memory
 * @param ptr	A pointer to the memory to free
 */
void kfree(void *ptr)
{
	struct mblock *i = used_top;
	struct mblock *p = i;
	while(i) {
		if(i->memory == ptr) {
			if (*((int*)(i->memory - 4)) != GUARD_1 || *((int*)(i->memory + i->size)) != GUARD_2 )
				panic("kernel heap corruption!");
			if(p == i)
				used_top = i->next_block;
			else 
				p->next_block = i->next_block;
			i->next_block = free_top;
			free_top = i;
		}
		p = i;
		i = i->next_block;
	}
}

/*
 * Gets the size of a block of memory
 * @param ptr	Pointer to the block in question
 * @returns		The size of ptr
 */
size_t ksize(void *ptr)
{
	struct mblock *i = used_top;
	while(i) {
		if(i->memory == ptr) {
			return i->size;
		}
		i = i->next_block;
	}
	return 0;
}
