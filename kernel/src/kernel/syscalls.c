#include <infinity/kernel.h>
#include <infinity/interrupt.h>
#include <infinity/fildes.h>
#include <infinity/syscalls.h>
#include <infinity/fcntl.h>
#include <infinity/sched.h>


int syscall_table[] = {
  NULL,  // 0
  exit,  // 1
  sbrk,  // 2
  read,  // 3
  write, // 4
  open,  // 5
  close, // 6
  lseek, // 7
  fstat  // 8
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
    asm("sti");
    r->eax = ((int (*)(int, int, int, int, int))(void*)syscall_table[r->eax])(r->ebx, r->ecx, r->edx, r->esi, r->edi);
}
