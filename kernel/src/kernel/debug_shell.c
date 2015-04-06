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
    int                     (*interpret) (const char *args);
    struct debug_command *  next;
};

struct debug_command *dbg_cmd_list = NULL;
static char *current_directory = "/lib/infinity";

static struct file *stdin;
static struct file *stdout;
static void gets(char *buf);
static void printf(const char *fmt, ...);
static void exec_command(const char *cmd);
static struct debug_command *get_command(const char *cmd);
static void add_command(const char *cmd, void *callback);
static void cmd_help(const char *args);
static void cmd_ls(const char *args);
static void cmd_dmesg(const char *args);

void init_debug_shell()
{
    add_command("help", cmd_help);
    add_command("ls", cmd_ls);
    add_command("dmesg", cmd_dmesg);
    stdin = fopen("/dev/tty0", O_RDWR);
    stdout = fopen("/dev/tty0", O_RDWR);
    printf("Infinity debug shell enabled. Only basic commands are supported (IE: ls, insmod, dmesg, ect). For help type 'help'\n");
    while(1) {
        char input[512];
        printf("\x1B[32minfinity \x1B[34m%s \x1B[1;32m# \x1B[0m", current_directory);
        gets(input);
        exec_command(input);
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


static void exec_command(const char *cmd)
{
    struct debug_command *c = get_command(cmd);
    if(c) {
        c->interpret(cmd);
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


static void cmd_help(const char *args)
{
    printf( "Infinity debug shell supports the following commands\n"
            "insmod - insert a kernel module\n"
            "ls     - show a directory listing\n"
            "cd     - change the current directory\n"
            "cat    - display the contents of a file\n"
            "dmesg  - display logged kernel messages\n"
            "clear  - clear the screen\n");
    
}

static void cmd_ls(const char *args)
{
    struct file *d = fopen(current_directory, O_RDWR);
    struct dirent ent;
    for(int i = 0; readdir(d, i, &ent) == 0; i++) {
        switch(ent.d_type) {
            case DT_DIR:
                printf("\x1B[1;34m%s\x1B[0m\n", ent.d_name);
                break;
            default:
                printf("%s\n", ent.d_name);
                break;
        }
    }
    fclose(d);
}


static void cmd_dmesg(const char *args)
{
    struct kernel_msg *msg_buf = (struct kernel_msg*)kalloc(sizeof(struct kernel_msg)* 512);
    
    flush_klog(msg_buf, sizeof(struct kernel_msg) * 512);
    for(int i = 0; i < 10; i++) {
        printf("[%d%d.%d] %s", msg_buf[0].msg_tm.tm_hour, msg_buf[0].msg_tm.tm_min, msg_buf[0].msg_tm.tm_sec, msg_buf[i].msg_string);
    }
    
    kfree(msg_buf);
}
