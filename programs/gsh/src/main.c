#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/infinity.h>
#include "interpreter.h"

int main(char **argc, char **environ)
{
    char input[1024];
    char cd[64];
    struct interpreter_context context;
    init_interpreter(&context);
    while(1) {
        char input[512];
        sys_getwd(cd);
        printf("\x1B[32mgsh \x1B[34m%s \x1B[1;32m# \x1B[0m", cd);
        fflush(stdout);
        gets(input);
        fflush(stdin);
        
        interpret(&context, input);
        memset(input, 0, 512);
    }
}
