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
 * sync.c
 * Provides methods for basic thread synchronization
 */

#include <infinity/types.h>
#define atomic_xadd(P, V) __sync_fetch_and_add((P), (V))

static int release_all = 0;

/*
 * Release all locls
 */
 
void release_all_locks()
{
    release_all = 1;
}
 
/*
 * Will lock a spinlock_t
 * @param lock  The lock to lock
 */
void spin_lock(spinlock_t *lock)
{
	while (*lock && !release_all)
		asm ("pause");
	__sync_lock_test_and_set(lock, 1);
}

/*
 * Will unlock a spinlock_t
 * @param lock  The lock to unlock
 */
void spin_unlock(spinlock_t *lock)
{
	asm volatile ("" : : : "memory");
	__sync_lock_release(lock, 0);
}

/*
 * Will lock a mutex_t
 * @param lock  The lock to lock
 */
void mutex_lock(mutex_t *lock)
{
	while (*lock) asm ("hlt");
	*lock = 1;
}

/*
 * Will unlock a mutex_t
 * @param lock  The lock to unlock
 */
void mutex_unlock(mutex_t *lock)
{
	*lock = 0;
}
