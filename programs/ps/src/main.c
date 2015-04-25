#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

struct ps_options {
    
};

static int parse_options(char **argv, int argc, struct ps_options *options);
static int set_options(char *str, struct ps_options *options);
static void proc_list(struct ps_options *options);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    struct ps_options options;
    memset(&options, 0, sizeof(struct ps_options));
    
    if(parse_options(argv, argc, &options) == 0) {
        proc_list(&options);
        sys_exit(0);
    } else {
        sys_exit(-1);
    }
}

static int parse_options(char **argv, int argc, struct ps_options *options) 
{
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(set_options(argv[i], options))
                return -1;
        } else if (argv[i][0]) {
            printf("ps: invalid option -- '%c'\n", argv[i]);
            return -1;
        }
    }
    return 0;
}

static int set_options(char *str, struct ps_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        printf("ps: invalid option -- '%c'\n", str[i]);
        return -1;
    }
    return 0;
}

static void proc_list(struct ps_options *options)
{
    int fd = sys_open("/proc", 0xFF);
    if(fd != -1) {
        struct infinity_dirent dent;
        int d = 0;
        printf("PID\tCMD\n");
        while(sys_readdir(fd, d++, &dent) != -1) {
            printf("%s\n", dent.d_name);
        }
        
        sys_close(fd);
    } else {
        printf("ps: could not access proc! This is strange!\n");
    }
}

