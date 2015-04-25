#ifndef INFINITY_SYS_H
#define INFINITY_SYS_H

#include <stdint.h>

#define SYS_EXIT    0x01
#define SYS_SBRK    0x02
#define SYS_READ    0x03
#define SYS_WRITE   0x04
#define SYS_OPEN    0x05
#define SYS_CLOSE   0x06
#define SYS_LSEEK   0x07
#define SYS_FSTAT   0x08
#define SYS_READDIR 0x09
#define SYS_SPAWNVE 0x0A
#define SYS_GETUID  0x0B
#define SYS_SETUID  0x0C
#define SYS_GETWD   0x0D
#define SYS_SETWD   0x0E
#define SYS_PIPE    0x0F
#define SYS_FCNTL   0x10
#define SYS_MKFIFO  0x11
#define SYS_MKNODE  0x12
#define SYS_MKDIR   0x13
#define SYS_CREAT   0x14
#define SYS_UNLINK  0x15
#define SYS_RMDIR   0x16

#define P_WAIT      0        
#define P_NOWAIT    1
#define P_DETACH    4
#define P_SUSPEND   8
#define P_CHILD     16

struct infinity_dirent {
    uint32_t    d_ino;
    uint8_t     d_type;
    char        d_name[256];
};

void sys_exit(int status);
int sys_open(const char *path, int mode);
int sys_close(int fd);
int sys_read(int fd, void *buf, int n);
int sys_write(int fd, void *buf, int n);
void sys_exit(int status);
int sys_readdir(int fd, int i, struct infinity_dirent *buf);
int sys_spawnve(int mode, char *path, char **argv, char **envp);
char *sys_getwd(char *buf);
int sys_setwd(char *buf);
int sys_getuid();
int sys_setuid(int uid);
int sys_pipe(int pipedes[2]);
int sys_fcntl(int fd, int arg1, int arg2, int arg3);
int sys_mkdir(const char *dir);
int sys_rmdir(const char *dir);
int sys_unlink(const char *dir);

#endif
