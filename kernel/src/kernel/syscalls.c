#include <infinity/kernel.h>
#include <infinity/arch/pic.h>
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
  fstat,  // 8
  readdir,  // 9
  spawnve, // 10
  getuid, // 11
  setuid, // 12
  getwd, // 13
  setwd, // 14
  pipe, // 15
  fcntl, // 16 (0x10)
  mkfifo, // 17 (0x11)
  NULL, // 0x12
  mkdir, // 0x13
  NULL, // 0x14 
  unlink,
  rmdir,
  symlink,
  readlink,
  lstat,
  chmod,
  NULL, // chown
  waitpid
  
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
