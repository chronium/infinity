#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

static void get_pos(int in, int *x, int *y);

int main(char **argv, char **environ)
{
    
    int input = open("/dev/tty0", O_RDONLY);
    printf("\x1B[12h");
    char c = NULL;
    while(read(0, &c, 1) > 0) {
        printf("%c", c);
        int x, y;
        get_pos(input, &x, &y);
        if(y == 24) {
            printf("\x1B[0;24r");
            printf("\x1B[0;24f\x1B[1;47;2;30m--- More ---");
            printf("\x1B[8m");
            fflush(stdout);
            char c;
            read(input, &c, 1);
            printf("\x1B[0m");
            printf("\x1B[%d;23f", x);
            fflush(stdout);
        }
    }
    printf("\x1B[12l\n");
    fflush(stdout);
    sys_exit(0);
}


static void get_pos(int in, int *x, int *y)
{
    printf("\x1B[12h\x1B[6n");
    fflush(stdout);
    char response[16];
    char c;
    int i = 0;
    while(read(in, &c, 1) && c != 'R' && i < 16) {
        response[i++] = c;
    }
    char *p1 = response + 2;
    char *p2 = strchr(response + 1, ';') + 1;
    for(int i = 0; i < 16; i++) {
        if(response[i] == ';' || response == 'R')
            response[i] = 0;
    }
    *x = atoi(p1);
    *y = atoi(p2);
}
