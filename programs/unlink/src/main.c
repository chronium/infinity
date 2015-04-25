#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>


int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    for(int i = 1; i < argc; i++) {
        if(sys_unlink(argv[i]) != 0) {
            printf("unlink: could not remove '%s'\n", argv[i]);
        }
    }
    sys_exit(0);
}

