#include <infinity/kernel.h>
#include <infinity/interrupt.h>
#include <infinity/fildes.h>
#include <infinity/syscalls.h>
#include <infinity/sched.h>


int syscall_table[] = {
  NULL, 
  NULL,
  NULL,
  read,  
  write, 
  open, 
  close, 
};

static void syscall_handler(struct regs *r);

void init_syscalls()
{
    request_isr(0x80, syscall_handler);
}


extern struct process *current_proc;
static int scount = 0;

static void syscall_handler(struct regs *r)
{
    r->eax = ((int (*)(int, int, int, int, int))(void*)syscall_table[r->eax])(r->ebx, r->ecx, r->edx, r->esi, r->edi);
    scount++;
    if(scount == 9) {
       
        scount = 0;
        exit(0);
    }
}
