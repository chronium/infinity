#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

struct cat_options {
    char    file[128];
};

static int parse_options(char **argv, int argc, struct cat_options *options);
static int set_options(char *str, struct cat_options *options);
static void cat_file(struct cat_options *options);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    struct cat_options options;
    memset(&options, 0, sizeof(struct cat_options));
    
    if(parse_options(argv, argc, &options) == 0) {
        cat_file(&options);
        sys_exit(0);
    } else {
        sys_exit(-1);
    }
}

static int parse_options(char **argv, int argc, struct cat_options *options) 
{
    int file_set = 0;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(set_options(argv[i], options))
                return -1;
        } else {
            strcpy(options->file, argv[i]);
            file_set = 1;
        }
    }
    if(file_set == 0) {
        printf("cat: no file specified!\n");
        return -1;
    } else {
        return 0;
    }
}

static int set_options(char *str, struct cat_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        printf("cat: invalid option -- '%c'\n", str[i]);
        return -1;
    }
    return 0;
}

static void cat_file(struct cat_options *options)
{
    int fd = sys_open(options->file, O_RDONLY);
    if(fd != -1) {
        char c = NULL;
        while(read(fd, &c, 1)) {
            printf("%c", c);
        }
        fflush(stdout);
        sys_close(fd);
    } else {
        printf("cat: no such file '%s'\n", options->file);
    }
}

