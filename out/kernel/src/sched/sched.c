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
 * sched.c
 * Provides functions for scheduling threads and tcontext switching
 */

#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/sched.h>


extern struct regs *pit_irq_regs;

struct thread *thread_list = NULL;
struct thread *current_thread = NULL;

static void thread_alloc(struct thread *t);
static struct thread *thread_get_next();

void init_sched()
{
}

/*
 * Performs a context switch, switching to a new thread
 * @param regs			The processor registers
 */
void perform_context_switch(struct regs *state)
{
	BEGIN_CRITICAL_REGION;

	int orgds = state->ds;
	int orgcs = state->cs;
	int orgss = state->ss;

	if (thread_list) {
		if (current_thread)
			memcpy(current_thread->t_regs, state, sizeof(struct regs));

		current_thread = thread_get_next();
		memcpy(state, current_thread->t_regs, sizeof(struct regs));
	}

	state->ds = orgds;
	state->cs = orgcs;
	state->ss = orgss;

	END_CRITICAL_REGION;
}

/*
 * Allocates a thread placing it on the queue
 */
static void thread_alloc(struct thread *new_thread)
{
	new_thread->next = NULL;

	struct thread *t = thread_list;

	if (thread_list) {
		struct thread *i = thread_list;
		while (i->next)
			i = i->next;
		i->next = new_thread;
	} else {
		thread_list = new_thread;
	}
}


/*
 * Gets the next thread in the queue to be executed
 */
static struct thread *thread_get_next()
{
	struct thread *i = thread_list;

	while (i) {
		if (i == current_thread) {
			if (i->next)
				return i->next;
			else
				return thread_list;
		}
		i = i->next;
	}
	return thread_list;
}

/*
 * Creates a thread
 * @param target		The entry point
 * @param args			Arguments to be passed to the entry point
 */
void thread_create(void *target, void *arg)
{
	BEGIN_CRITICAL_REGION;

	struct thread *t = (struct thread *)kalloc(sizeof(struct thread));
	t->t_regs = kalloc(sizeof(struct regs));
	memcpy(t->t_regs, pit_irq_regs, sizeof(struct regs));
	t->t_regs->eip = target;
	void *stack = kalloc(0xFFFFF);
	t->t_regs->esp = stack + 0xFFFFF;
	t->t_regs->ebp = stack + 0xFFFFF;

	thread_alloc(t);

	END_CRITICAL_REGION;
}

/*
 * Gives up resource time and switches to the next thread
 */
void thread_yield()
{
	asm ("hlt");
}
