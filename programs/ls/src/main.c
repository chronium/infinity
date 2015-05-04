#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/infinity.h>

struct ls_options {
    char    list_all;
    char    long_list;
    char    directory[128];
};

static int parse_options(char **argv, int argc, struct ls_options *options);
static int set_options(char *str, struct ls_options *options);
static void ls_dir(struct ls_options *options);
static void ls_ent(struct ls_options *options, struct infinity_dirent *dent);
static void print_mode(int mode);

int main(char **argv, char **environ)
{
    int argc = 0;
    for(int i = 0; argv[i]; i++) argc++;
    struct ls_options options;
    memset(&options, 0, sizeof(struct ls_options));
    sys_getwd(options.directory);
    if(parse_options(argv, argc, &options) == 0) {
        ls_dir(&options);
        sys_exit(0);
    } else {
        sys_exit(-1);
    }
}

static int parse_options(char **argv, int argc, struct ls_options *options) 
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

static int set_options(char *str, struct ls_options *options)
{
    int len = strlen(str);
    for(int i = 1; i < len; i++) {
        if(str[i] == 'a') {
            options->list_all = 1;
        } else if (str[i] == 'l') {
            options->long_list = 1;
        } else {
            printf("ls: invalid option -- '%c'\n", str[i]);
            return -1;
        }
    }
    return 0;
}

static void ls_dir(struct ls_options *options)
{
    int fd = sys_open(options->directory, 0xFF);
    if(fd != -1) {
        struct infinity_dirent dent;
        int d = 0;
        
        while(sys_readdir(fd, d++, &dent) != -1) {
            ls_ent(options, &dent);
        }
        
        sys_close(fd);
    } else {
        printf("ls: could not access directory '%s'\n", options->directory);
    }
}


static void ls_ent(struct ls_options *options, struct infinity_dirent *dent)
{
    char full_path[128];
    sprintf(full_path, "%s/%s", options->directory, dent->d_name);
    struct stat st;
    if(options->long_list) {
        if(sys_lstat(full_path, &st) == 0) {
            print_mode(st.st_mode);
            struct passwd *pw = getpwuid(st.st_uid);
            printf("\t%s", pw->pw_name);
            printf("\t%d\t", st.st_uid);
            printf("\t%s", dent->d_name);
            if(dent->d_type == 0x07) {
                char tmp[128];
                sys_readlink(full_path, tmp, 128);
                printf(" -> %s", tmp);
            }
            printf("\n");
        }
    } else {
        printf("%s\n", dent->d_name);
    }
}

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

static void print_mode(int mode)
{
    for(int i = 8; i >= 0; i -= 3) {
        putc((CHECK_BIT(mode, i)) ? 'r' : '-', stdout); 
        putc((CHECK_BIT(mode, i - 1)) ? 'w' : '-', stdout); 
        putc((CHECK_BIT(mode, i - 2)) ? 'x' : '-', stdout); 
    }
}
