#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

int main(char **argv, char **environ)
{
    int fd = open("/pipe", O_RDONLY);
    
    char c = NULL;
    while(read(fd, &c, 1)) {
        printf("\x1B[0;33m%c\x1B[0m", c);
        fflush(stdout);
    }
    fflush(stdout);
    sys_close(fd);
    
    sys_exit(0);
}
