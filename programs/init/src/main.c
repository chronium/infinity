#include <stdio.h>
#include <fcntl.h>
#include <sys/infinity.h>

static void open_fildes();

int main(char **argc, char **environ)
{
    open_fildes();
    printf("\x1B");
    printf("c");
    printf("Welcome to GruntyOS Infinity!\n");
    sys_spawnve(P_NOWAIT, "/sbin/login", NULL, NULL);
    while(1);
}


static void open_fildes()
{
    open("/dev/tty0", O_RDONLY);
    open("/dev/tty0", O_WRONLY);
    open("/dev/tty0", O_WRONLY);
    printf("fildes opened!\n");
}
