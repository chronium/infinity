#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

int main(char **argv, char **environ)
{
    putc('\x1B', stdout);
    putc('c', stdout);
    fflush(stdout);
    sys_exit(0);
}
