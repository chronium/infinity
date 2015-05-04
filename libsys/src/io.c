#include <sys/infinity.h>

void sys_exit(int status)
{
    asm volatile("int $0x80" : : "a"(SYS_EXIT), "b"(status));
}

int sys_open(const char *path, int mode)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_OPEN), "b"(path));
    return ret;
}

int sys_close(int fd)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_CLOSE), "b"(fd));
    return ret;
}

int sys_read(int fd, void *buf, int n)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_READ), "b"(fd), "c" (buf), "d" (n));
    return ret;
}

int sys_write(int fd, void *buf, int n)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_WRITE), "b"(fd), "c" (buf), "d" (n));
    return ret;
}

int sys_readdir(int fd, int i, struct infinity_dirent *buf)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_READDIR), "b"(fd), "c" (i), "d" (buf));
    return ret;
}

int sys_pipe(int pipedes[2])
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_PIPE), "b"(pipedes));
    return ret;
}

int sys_fcntl(int fd, int arg1, int arg2, int arg3)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_FCNTL), "b"(fd), "c" (arg1), "d" (arg2), "D" (arg3));
    return ret;
}

int sys_mkdir(const char *dir)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_MKDIR), "b"(dir));
    return ret;
}

int sys_mkfifo(const char *dir, mode_t mode)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_MKFIFO), "b"(dir), "c"(mode));
    return ret;
}

int sys_unlink(const char *dir)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_UNLINK), "b"(dir));
    return ret;
}

int sys_rmdir(const char *dir)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_RMDIR), "b"(dir));
    return ret;
}


int sys_readlink(const char *path, char *buf, int len)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_READLINK), "b"(path), "c" (buf), "d" (len));
    return ret;
}

int sys_symlink(const char *from, const char *to)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_SYMLINK), "b"(from), "c" (to));
    return ret;
}

int sys_lstat(const char *path, struct stat *st)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_LSTAT), "b"(path), "c" (st));
    return ret;
}

int sys_chmod(const char *path, mode_t mode)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_CHMOD), "b"(path), "c" (mode));
    return ret;
}
