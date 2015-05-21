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
static int interpret_cmd(struct interpreter_context *context, struct gsh_command *node, int flags);
static void interpret_rdir(struct interpreter_context *context, struct ast_node *node);
static void interpret_pipe(struct interpreter_context *context, struct ast_node *node);
static int attempt_exec(const char *cmd, const char **args, int argc, int flags);

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
            interpret_cmd(context, (struct gsh_command*)node, P_WAIT);
            break;
        case NODE_RDIR:
            interpret_rdir(context, node);
            break;
        case NODE_PIPE:
            interpret_pipe(context, node);
            break;
    }
}


static int interpret_cmd(struct interpreter_context *context, struct gsh_command *node, int flags)
{
    char **args = node->c_args;
    const char *cmd = args[0];
    int argc = 0;
    int i = 0;
    while(cmd[i++]) argc++;
    struct builtin_command *c = get_command(cmd);
    if(c) {
        c->interpret(args, argc);
        return 0;
    } else {
        char full_path[64];
        int ret = 0;
        if((ret = attempt_exec(cmd, args, argc, flags)) == -1) {
            printf("gsh: command not found!\n");
            return -1;
        }
        return ret;
    }
}

static void interpret_rdir(struct interpreter_context *context, struct ast_node *node)
{
    struct gsh_command *file = node->n_children->next;
    int f = sys_open(file->c_args[0], O_WRONLY | O_CREAT);
    int out = sys_fcntl(1, 0, 0, 0);
    int err = sys_fcntl(2, 0, 0, 0);
    close(1);
    close(2);
    sys_fcntl(f, 0, 1, 0);
    sys_fcntl(f, 0, 2, 0);
    interpret_cmd(context, node->n_children, P_WAIT);
    close(f);
    close(1);
    close(2);
    sys_fcntl(out, 0, 1, 0);
    sys_fcntl(err, 0, 2, 0);
    close(out);
    close(err);
}

static void interpret_pipe(struct interpreter_context *context, struct ast_node *node)
{
    struct gsh_command *cmd1 = node->n_children;
    struct gsh_command *cmd2 = node->n_children->next;
    
    int in = sys_fcntl(0, 0, 0, 0);
    int out = sys_fcntl(1, 0, 0, 0);
    int err = sys_fcntl(2, 0, 0, 0);
    
    int pipe_des[2];
    sys_pipe(pipe_des);
    
    close(0);
    sys_fcntl(pipe_des[0], 0, 0, 0);
    int pid = interpret_cmd(context, cmd2, P_NOWAIT);
    
    close(0);
    close(1);
    close(2);
    
    sys_fcntl(in, 0, 0, 0);
    sys_fcntl(pipe_des[1], 0, 1, 0);
    sys_fcntl(pipe_des[1], 0, 2, 0);
    
    interpret_cmd(context, cmd1, P_WAIT);
    
    
    close(1);
    close(2);
    
    
    sys_fcntl(out, 0, 1, 0);
    sys_fcntl(err, 0, 2, 0);
    

    close(pipe_des[0]);
    close(pipe_des[1]);
    
    int status;
    sys_waitpid(pid, &status);
    
    close(out);
    close(err);
}

static int attempt_exec(const char *cmd, const char **args, int argc, int flags)
{
    char *path[] = {"/bin", "/sbin", NULL};
    int i = 0;
    while(path[i]) {
        char fullpath[128];
        sprintf(fullpath, "'%s/%s'", path[i++], cmd);
        int fd = 12;
        if((fd = open(fullpath, O_RDWR)) != -1) {
            close(fd);
            return sys_spawnve(flags, fullpath, args, argc); 
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
