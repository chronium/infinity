#include <sys/infinity.h>


int sys_spawnve(int mode, char *path, char **argv, char **envp)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_SPAWNVE), "b"(mode), "c"(path), "d"(argv), "S"(envp));
    return ret;
}

int sys_waitpid(int pid, int *status)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_WAITPID), "b"(pid), "c"(status));
    return ret;
}

char *sys_getwd(char *buf)
{
    char *ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETWD), "b"(buf));
    return ret;
}

int sys_setwd(char *buf)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_SETWD), "b"(buf));
    return ret;
}

int sys_getuid()
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETUID));
    return ret;
}

int sys_setuid(int uid)
{
    int ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(SYS_SETUID), "b"(uid));
    return ret;
}
