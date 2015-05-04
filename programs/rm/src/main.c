#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
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
static char *get_errmsg(int err);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    struct rm_options options;
    
    if(parse_options(argv, argc, &options) == 0) {
        sys_exit(0);
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
            int status = remove_item(argv[i], options);
            if(status) {
                printf("rm: could not remove '%s' : %s\n", argv[i], get_errmsg(status));
                return status;
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
        remove_dir(path, options);
        return sys_rmdir(path);
    } else {
        return sys_unlink(path);
    }
}

static int remove_dir(char *path, struct rm_options *options)
{
    int fd = open(path, O_RDONLY);
    if(fd) {
        struct infinity_dirent dent;
        int files_removed;
        int files_failed;
        char buf[256];
        do {
            files_removed = 0;
            files_failed = 0;
            int d = 0;
            while(sys_readdir(fd, d++, &dent) == 0) {
                if(path[0] == '/' && path[1] == 0)
                    sprintf(buf, "/%s", dent.d_name);
                else
                    sprintf(buf, "%s/%s", path, dent.d_name);
                int res;
                if(dent.d_type == 0x02) {
                    remove_dir(buf, options);
                    res = sys_rmdir(buf);
                } else {
                    res = sys_unlink(buf);
                }
                if(res == 0) {
                    files_removed++;
                } else {
                    files_failed++;
                    printf("rm: could not remove %s: %s\n", buf, get_errmsg(res));
                }
            }
        } while(files_removed != 0 && files_failed == 0);
        
        close(fd);
        return 0;
    }
    return -1;
}

static char *get_errmsg(int err)
{
    switch(err) {
        case EACCES:
            return "Access is denied";
        case EISDIR:
            return "Is a directory";
        case ENOENT:
            return "No such file or directory";
    }
    return "Unspecified error";
}
