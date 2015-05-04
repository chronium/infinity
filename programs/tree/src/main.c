#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/infinity.h>

struct tree_options {
    char    dir_only;
    char    directory[128];
};

static int parse_options(char **argv, int argc, struct tree_options *options);
static int set_options(char *str, struct tree_options *options);
static void tree_r(int ident, const char *dir, struct tree_options *options);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    struct tree_options options;
    memset(&options, 0, sizeof(struct tree_options));
    sys_getwd(options.directory);
    if(parse_options(argv, argc, &options) == 0) {
        tree_r(0, options.directory, &options);
        sys_exit(0);
    } else {
        sys_exit(-1);
    }
}

static int parse_options(char **argv, int argc, struct tree_options *options) 
{
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(set_options(argv[i], options))
                return -1;
        } else {
            strcpy(options->directory, argv[i]);
        }
    }
    return 0;
}

static int set_options(char *str, struct tree_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        if(str[i] == 'd') {
            options->dir_only = 1;
        } else {
            printf("tree: invalid option -- '%c'\n", str[i]);
            return -1;
        }
    }
    return 0;
}

static void tree_r(int ident, const char *dir, struct tree_options *options)
{
    char idents[32];
    char tmp[128];
    memset(idents, ' ', ident * 4);
    idents[ident * 4] = 0;
    
    int fd = sys_open(dir, 0xFF);
    if(fd != -1) {
        struct infinity_dirent dent;
        int d = 0;
        
        while(sys_readdir(fd, d++, &dent) != -1) {
            if(options->dir_only && dent.d_type != 0x02)
                continue;
            printf("%s|-- %s\n", idents, dent.d_name);
            if(dir[1] == 0) 
                sprintf(tmp, "/%s", dent.d_name);
            else 
                sprintf(tmp, "%s/%s", dir, dent.d_name);
            if(dent.d_type == 0x02) {
                tree_r(ident + 1, tmp, options);
            }
        }
        
        sys_close(fd);
    } else {
        printf("tree: could not access directory '%s'\n", options->directory);
    }
}
