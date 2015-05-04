#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

int main(char **argv, char **environ)
{
    int fd = open("/pipe", O_RDONLY);
    
    char c = NULL;
    
    write(fd, "Hello, World", 12);
    
    sys_close(fd);
    
    sys_exit(0);
}
