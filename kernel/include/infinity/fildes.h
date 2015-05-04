#ifndef INFINITY_FILDES_H
#define INFINITY_FILDES_H

#include <stddef.h>

int open(const char *path, int mode);
int close(int fd);
size_t write(int fd, const void *buf, size_t n);
size_t read(int fd, void *buf, size_t n);
int readdir(int fd, int i, struct dirent *buf);
int create_fildes(const char *path, struct file *f);
int pipe(int pipedes[2]);

#endif
