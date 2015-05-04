#ifndef INTEPRETER_H
#define INTEPRETER_H

struct builtin_command {
    int                     key;
    int                     (*interpret) (const char **args, int argc);
    struct builtin_command *next;
};

struct interpreter_context {
    
};

void init_interpreter(struct interpreter_context *context);
void interpret(struct interpreter_context *context, const char *cmd);
void add_command(const char *key, void *callback);
struct builtin_command *get_command(const char *cmd);

#endif
