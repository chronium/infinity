#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    for(int i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    sys_exit(0);
}
