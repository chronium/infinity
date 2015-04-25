#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/infinity.h>

struct rm_options {
    char    recursive;
    char    prompt;
    char    force;
};

struct rm_ent {
  char              path[128];
  struct rm_ent *   next;  
};

static int parse_options(char **argv, int argc, struct rm_options *options);
static int set_options(char *str, struct rm_options *options);
static int remove_item(char *path, struct rm_options *options);
static int remove_dir(char *path, struct rm_options *options);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    struct rm_options options;
    
    if(parse_options(argv, argc, &options) == 0) {
        sys_exit(0);
    } else {
        printf("rm: could not remove file!\n");
    }
    sys_exit(-1);
}

static int parse_options(char **argv, int argc, struct rm_options *options) 
{
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(set_options(argv[i], options))
                return -1;
        } else {
            if(remove_item(argv[i], options)) {
                return -1;
            }
        }
    }
    return 0;
}

static int set_options(char *str, struct rm_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        if(str[i] == 'f') {
            options->force = 1;
        } else if (str[i] == 'r') {
            options->recursive = 1;
        }  else if (str[i] == 'i') {
            options->prompt = 1;
        } else {
            printf("rm: invalid option -- '%c'\n", str[i]);
            return -1;
        }
    }
    return 0;
}

static int remove_item(char *path, struct rm_options *options)
{
    if(options->recursive) {
        return sys_rmdir(path);
    } else {
        return sys_unlink(path);
    }
}


