#ifndef INFINITY_SIGNAL_H
#define INFINITY_SIGNAL_H

#include <stdint.h>

#define SIGKILL	9

typedef void (*sighandler_t)(int signum);

struct signal_handler {
    sighandler_t               handler;
    struct signal_table        *next;
};

struct signal_table {
    uint32_t                    sig_mask;
    struct signal_handler *     handlers;
    
};

sighandler_t signal(int sig, sighandler_t handler);
int kill(pid_t id, int sig);

#endif
