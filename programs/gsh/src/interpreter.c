#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/infinity.h>
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"

struct builtin_command *cmd_list = NULL;

static int hash_string(const char *key);
static void interpret_node(struct interpreter_context *context, struct ast_node *node);
static void interpret_cmd(struct interpreter_context *context, struct gsh_command *node);
static void interpret_rdir(struct interpreter_context *context, struct ast_node *node);
static int attempt_exec(const char *cmd, const char **args, int argc);

static void cmd_cd(const char **args, int argc);

void init_interpreter(struct interpreter_context *context)
{
    add_command("cd", cmd_cd);
    
}


struct builtin_command *get_command(const char *cmd)
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

void add_command(const char *key, void *callback)
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

void interpret(struct interpreter_context *context, const char *cmd)
{
    struct tokenizer scanner;
    struct parser_context parser;
    init_tokenizer(&scanner, cmd);
    tokenize(&scanner);
    init_parser(&parser, scanner.token_list);
    struct ast_node *n = parse(&parser);
    interpret_node(context, n);
    uninit_parser(&parser);
    uninit_tokenizer(&scanner);
}


static void interpret_node(struct interpreter_context *context, struct ast_node *node)
{
    switch(node->n_type) {
        case NODE_CMD:
            interpret_cmd(context, (struct gsh_command*)node);
            break;
        case NODE_RDIR:
            interpret_rdir(context, node);
            break;
    }
}


static void interpret_cmd(struct interpreter_context *context, struct gsh_command *node)
{
    char **args = node->c_args;
    const char *cmd = args[0];
    int argc = 0;
    int i = 0;
    while(cmd[i++]) argc++;
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

static void interpret_rdir(struct interpreter_context *context, struct ast_node *node)
{
    struct gsh_command *file = node->n_children->next;
    int f = sys_open(file->c_args[0], O_WRONLY | O_CREAT);
    sys_fcntl(f, 0, 1, 0);
    sys_fcntl(f, 0, 2, 0);
    interpret_cmd(context, node->n_children);
    close(f);
    int out = sys_open("/dev/tty0", 0xFF);
    sys_fcntl(out, 0, 1, 0);
    sys_fcntl(out, 0, 2, 0);
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

static void cmd_cd(const char **args, int argc)
{
    if(sys_setwd(args[1]) != 0) {
        printf("cd: %s: no such directory!\n", args[1]);
    }
}
