#include <stdint.h>
#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/virtfs.h>
#include <infinity/dirent.h>
#include <infinity/fcntl.h>


struct debug_command {
    int                     key;
    int                     (*interpret) (const char *args[], int argc);
    struct debug_command *  next;
};

struct token {
    const char  *           value;
    struct token *          next;
};

struct tokenizer {
    int                     position;
    char    *               source;
    int                     tokens;
    struct token *          token_list;  
    
};

struct debug_command *dbg_cmd_list = NULL;
static char current_directory[1024];

static struct file *stdin;
static struct file *stdout;
static void gets(char *buf);
static void printf(const char *fmt, ...);
static void exec_command(const char *args[], int argc);
static struct debug_command *get_command(const char *cmd);
static void add_command(const char *cmd, void *callback);
static void init_tokenizer(struct tokenizer *scanner, const char *source);
static void uninit_tokenizer(struct tokenizer *scanner);
static int tokenize(struct tokenizer *context);
static int scanner_next(struct tokenizer *context);
static void scanner_scan_ident(struct tokenizer *context);
static void canonicalize_path(const char *path, char *buf);
static void cmd_cd(const char *args[], int argc);
static void cmd_help(const char *args[], int argc);
static void cmd_ls(const char *args[], int argc);
static void cmd_dmesg(const char *args[], int argc);
static void cmd_clear(const char *args[], int argc);
static void cmd_exec(const char *args[], int argc);

void init_debug_shell()
{
    strcpy(current_directory, "/lib/infinity");
    
    add_command("help", cmd_help);
    add_command("ls", cmd_ls);
    add_command("cd", cmd_cd);
    add_command("dmesg", cmd_dmesg);
    add_command("clear", cmd_clear);
    add_command("exec", cmd_exec);
    
    stdin = fopen("/dev/tty0", O_RDWR);
    stdout = fopen("/dev/tty0", O_RDWR);
    printf("Infinity debug shell enabled. Only basic commands are supported (IE: ls, insmod, dmesg, ect). For help type 'help'\n");
    
    struct tokenizer scanner;
    char *args[64];
    while(1) {
        char input[512];
        printf("\x1B[32minfinity \x1B[34m%s \x1B[1;32m# \x1B[0m", current_directory);
        gets(input);
        init_tokenizer(&scanner, input);
        tokenize(&scanner);
        struct token *i = scanner.token_list;
        int j = 0;
        while(i) {
            args[j++] = i->value;
            i = i->next;
        }
        uninit_tokenizer(&scanner);
        
        if(j) {
            exec_command(args, j);
        }
    }    
}

static void gets(char *buf)
{
    fread(stdin, buf, 0, 256);
}

static void printf(const char *format, ...)
{
    char tmp[512];
    va_list argp;
    va_start(argp, format);
    char tmp_buff[512];
    memset(tmp_buff, 0, 512);
    vsprintf(tmp_buff, format, argp);
    va_end(argp);
    fwrite(stdout, tmp_buff, 0, strlen(tmp_buff));
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


static void exec_command(const char *args[], int argc)
{
    const char *cmd = args[0];
    struct debug_command *c = get_command(cmd);
    if(c) {
        c->interpret(args, argc);
    } else {
        printf("infinity: Command not found!\n");
    }
}


static struct debug_command *get_command(const char *cmd)
{
    struct debug_command *i = dbg_cmd_list;
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
    struct debug_command *cmd = (struct debug_command*)kalloc(sizeof(struct debug_command));
    cmd->key = hash_string(key);
    cmd->interpret = callback;
    cmd->next = NULL;
    struct debug_command *i = dbg_cmd_list;
    if(i) {
        while(i->next) {
            i = i->next;
        }
        i->next = cmd;
    } else {
        dbg_cmd_list = cmd;
    }
}

static void init_tokenizer(struct tokenizer *scanner, const char *source)
{
    memset(scanner, 0, sizeof(struct tokenizer));
    scanner->source = source;
}


static void uninit_tokenizer(struct tokenizer *scanner)
{
    struct token *tok = scanner->token_list;
    while(tok) {
        struct token *t = tok;
        tok = tok->next;
        kfree(t->value);
        kfree(t);
    }
}

static int tokenize(struct tokenizer *context)
{
    int i = 0;
    while(context->source[context->position]) {
        while(context->source[context->position] == ' ')
            context->position++;
        if(context->source[context->position]) {
            scanner_scan_ident(context);
        }
    }
    return context->tokens;
}

static int scanner_next(struct tokenizer *context)
{
    if(context->source[context->position])
        return (int)context->source[context->position++];
    else
        return -1;
}

static void scanner_scan_ident(struct tokenizer *context) 
{
    char *buf = (char*)kalloc(128);
    int i = 0;
    int c = scanner_next(context);
    while(c != -1 && c != ' ') {
        buf[i++] = (char)c;
        c = scanner_next(context);
    }
    struct token *tok = (struct token*)kalloc(sizeof(struct token));
    tok->value = buf;
    if(context->token_list) {
        struct token *n = context->token_list;
        while(n->next)
            n = n->next;
        n->next = tok;
    } else {
        context->token_list = tok;
    }
    context->tokens++;
}

static void canonicalize_path(const char *path, char *buf) 
{
    buf[0] = 0;
    if(path[0] == '/') {
        strcpy(buf, path);
    } else {
        sprintf(buf, "%s/%s", current_directory, path);
    }
}

static void cmd_cd(const char *args[], int argc)
{
    if(argc != 2) {
        printf("cd does not take %d arguments!\n", argc);
    } else {
        char path[256];
        canonicalize_path(args[1], path);
        strcpy(current_directory, path);
    }
}

static void cmd_help(const char *args[], int argc)
{
    printf( "Infinity debug shell supports the following commands\n"
            "insmod - insert a kernel module\n"
            "ls     - show a directory listing\n"
            "cd     - change the current directory\n"
            "cat    - display the contents of a file\n"
            "dmesg  - display logged kernel messages\n"
            "clear  - clear the screen\n");
    
}

static void cmd_ls(const char *args[], int argc)
{
    char *dir = argc == 1 ? current_directory : args[1];
    struct file *d = fopen(dir, O_RDWR);
    
    if(d) {
        struct dirent ent;
        for(int i = 0; readdir(d, i, &ent) == 0; i++) {
            switch(ent.d_type) {
                case DT_DIR:
                    printf("\x1B[1;34m%s\x1B[0m\n", ent.d_name);
                    break;
                case DT_CHR:
                    printf("\x1B[33;40m%s\x1B[0m\n", ent.d_name);
                    break;
                case DT_BLK:
                    printf("\x1B[1;33;2;40m%s\x1B[0m\n", ent.d_name);
                    break;
                default:
                    printf("%s\n", ent.d_name);
                    break;
            }
        }
        fclose(d);
    } else {
        printf("ls: Could not find directory '%s'!\n", dir);
    }
}

static void cmd_dmesg(const char *args[], int argc)
{
    struct kernel_msg *msg_buf = (struct kernel_msg*)kalloc(sizeof(struct kernel_msg)* 512);
    
    flush_klog(msg_buf, sizeof(struct kernel_msg) * 512);
    for(int i = 0; i < 10; i++) {
        printf("[%d%d.%d] %s", msg_buf[0].msg_tm.tm_hour, msg_buf[0].msg_tm.tm_min, msg_buf[0].msg_tm.tm_sec, msg_buf[i].msg_string);
    }
    
    kfree(msg_buf);
}

static void cmd_clear(const char *args[], int argc)
{
    printf("\x1B");
    printf("c");
}

static void cmd_exec(const char *args[], int argc)
{
    char path[256];
    canonicalize_path(args[1], path);
    spawnve(0, path, 0, 0, 0);
    execvpe(path, NULL, NULL);
}
