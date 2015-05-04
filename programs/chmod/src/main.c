#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/infinity.h>

struct chmod_options {
    char    mode[32];
    char    path[128];
    char    recursive;
};

static int parse_options(char **argv, int argc, struct chmod_options *options);
static int set_options(char *str, struct chmod_options *options);
static int change_mode(struct chmod_options *options);
static int change_mode_r(const char *path, mode_t mode);
static char *get_errmsg(int err);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    
    struct chmod_options options;
    memset(&options, 0, sizeof(struct chmod_options));
    
    if(parse_options(argv, argc, &options) == 0 && change_mode(&options) == 0) {
        sys_exit(0);
    }
    sys_exit(-1);
    
}

static int parse_options(char **argv, int argc, struct chmod_options *options) 
{
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(set_options(argv[i], options))
                return -1;
        } else if (!options->mode[0]) {
            strcpy(options->mode, argv[i]);
        } else if (!options->path[0]) {
            strcpy(options->path, argv[i]);
        } else if (argv[i]) {
            printf("chmod: invalid option -- '%c'\n", argv[i]);
            return -1;
        }
    }
    return 0;
}

static int set_options(char *str, struct chmod_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        if(str[i] == 'r') {
            options->recursive = 1;
        } else {
            printf("chmod: invalid option -- '%c'\n", str[i]);
            return -1;
        }
    }
    return 0;
}

static int change_mode(struct chmod_options *options)
{
    if(options->path[0] == 0 || options->mode[0] == 0) {
        printf("chmod: missing operand!\n");
        return -1;
    }
    
    int mode = strtol(options->mode, NULL, 8);
    
    if(options->recursive) {
        if(change_mode_r(options->path, mode))
            return -1;
    }
    
    int res = sys_chmod(options->path, mode);
    
    if(res != 0) {
        printf("chmod: can not access %s : %s\n", options->path, get_errmsg(res));
        return -1;
    }
    
    return 0;
    
}

static int change_mode_r(const char *path, mode_t mode)
{
    int ret = 0;
    struct infinity_dirent dent;
    int fd = open(path, O_RDONLY);
    char buf[256];
    int d = 0;
    while(sys_readdir(fd, d++, &dent) == 0) {
        if(path[0] == '/' && path[1] == 0)
            sprintf(buf, "/%s", dent.d_name);
        else
            sprintf(buf, "%s/%s", path, dent.d_name);
        if(dent.d_type == 0x02)
            ret = change_mode_r(buf, mode);
        int res = sys_chmod(buf, mode);
        if(res != 0) {
            printf("chmod: can not access %s : %s\n", buf, get_errmsg(res));
            ret = res;
        }
    }
    return ret;
}

static char *get_errmsg(int err)
{
    switch(err) {
        case EACCES:
            return "Access is denied";
        case ENOENT:
            return "No such file or directory";
    }
    return "Unspecified error";
}
