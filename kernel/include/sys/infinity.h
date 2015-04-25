#ifndef INFINITY_SYS_H
#define INFINITY_SYS_H

#include <stdint.h>

struct infinity_dirent {
    uint32_t    d_ino;
    uint8_t     d_type;
    char        d_name[256];
};

void sys_exit(int status);
int sys_open(char *path, int mode);
int sys_close(int fd);
int sys_read(int fd, void *buf, int n);
int sys_write(int fd, void *buf, int n);
void sys_exit(int status);
int sys_readdir(int fd, int i, struct infinity_dirent *buf);

#endif
