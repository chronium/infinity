#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/infinity.h>
#include "scanner.h"

struct builtin_command {
    int                     key;
    int                     (*interpret) (const char **args, int argc);
    struct builtin_command *next;
};


struct builtin_command *cmd_list = NULL;

static void interpret(const char *cmd);
static void exec_command(const char **args, int argc);

static int attempt_exec(const char *cmd, const char **args, int argc);

static int hash_string(const char *key);
static struct builtin_command *get_command(const char *cmd);
static void add_command(const char *key, void *callback);

static void cmd_cd(const char **args, int argc);


int main(char **argc, char **environ)
{
    add_command("cd", cmd_cd);
    char input[1024];
    char cd[64];
    while(1) {
        char input[512];
        sys_getwd(cd);
        printf("\x1B[32mgsh \x1B[34m%s \x1B[1;32m# \x1B[0m", cd);
        fflush(stdout);
        gets(input);
        fflush(stdin);
        interpret(input);
        memset(input, 0, 512);
    }
}

static int hash_string(const char *key)
{
    size_t len = strlen(key);
    int hash = 0;
    int i = 0;
    
    for(hash = i = 0; i < len; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static struct builtin_command *get_command(const char *cmd)
{
    struct builtin_command *i = cmd_list;
    int hash = hash_string(cmd);
    while(i) {
        if(i->key == hash)
            return i;
        i = i->next;
    }
    return NULL;
}

static void add_command(const char *key, void *callback)
{
    struct builtin_command *cmd = (struct builtin_command*)malloc(sizeof(struct builtin_command));
    cmd->key = hash_string(key);
    cmd->interpret = callback;
    cmd->next = NULL;
    struct builtin_command *i = cmd_list;
    if(i) {
        while(i->next) {
            i = i->next;
        }
        i->next = cmd;
    } else {
        cmd_list = cmd;
    }
}

static void interpret(const char *cmd)
{
    char **args[64];
    struct tokenizer scanner;
    init_tokenizer(&scanner, cmd);
    tokenize(&scanner);
    struct token *i = scanner.token_list;
    int j = 0;
    while(i) {
        args[j++] = i->value;
        i = i->next;
    }
    args[j] = NULL;
    exec_command(args, j);
    uninit_tokenizer(&scanner);
}

static void exec_command(const char **args, int argc)
{
    const char *cmd = args[0];
    struct builtin_command *c = get_command(cmd);
    if(c) {
        c->interpret(args, argc);
    } else {
        char full_path[64];
        
        if(attempt_exec(cmd, args, argc) == -1) {
            printf("gsh: command not found!\n");
        }
    }
}

static int attempt_exec(const char *cmd, const char **args, int argc)
{
    char *path[] = {"/bin", "/sbin", NULL};
    int i = 0;
    while(path[i]) {
        char fullpath[128];
        sprintf(fullpath, "'%s/%s'", path[i++], cmd);
        int fd;
        if(fd = open(fullpath, O_RDWR) != -1) {
            close(fd);
            sys_spawnve(P_WAIT, fullpath, args, argc); 
            return 0;
        }
    }
    return -1;
}

static void cmd_cd(const char **args, int argc)
{
    if(sys_setwd(args[1]) != 0) {
        printf("cd: %s: no such directory!\n", args[1]);
    }
}
