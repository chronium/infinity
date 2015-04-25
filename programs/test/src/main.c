#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/infinity.h>


int main(char **argv, char **environ)
{
    int pipedes[2];
    sys_pipe(pipedes);
    printf("Pipes created!\n");
    sys_fcntl(pipedes[1], 0, 1, 0);
    sys_spawnve(P_NOWAIT, "/bin/ls", NULL, NULL);
    int out = sys_open("/dev/tty0", 0xFF);
    sys_fcntl(out, 0, 1, 0);
    printf("we have control of stdout again\n");
    while(1) {
        char c;
        read(pipedes[0], &c, 1);
        printf("Child said %c!\n", c);
    }
}
