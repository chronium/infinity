#ifndef INFINITY_SCHED_H
#define INFINITY_SCHED_H

#include <stddef.h>
#include <infinity/types.h>
#include <infinity/interrupt.h>
#include <infinity/virtfs.h>

#define BEGIN_CRITICAL_REGION	asm("cli"); 
#define END_CRITICAL_REGION		asm("sti"); 

struct process {
	char					p_name[64];
	pid_t					p_id;
	uid_t					p_uid;
	gid_t					p_gid;
	struct fildes			*p_file_descriptor_table;
	struct process			*next;
};

struct thread {
	pid_t					t_id;
	struct regs				*t_regs;
	struct process			*t_proc;
	struct thread			*next;
};

struct task {
	void (*task_handler)		(void *data);
	void						*data;
	struct task					*next;
};

extern struct process *current_proc;

void init_sched();
void schedule_task(struct task *task);
void thread_create(void *target, void *arg);
void thread_yield();

pid_t getpid();
uid_t getuid();

#endif
