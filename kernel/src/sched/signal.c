#include <stdint.h>
#include <infinity/sched.h>
#include <infinity/signal.h>


static void really_kill_process(pid_t id);

sighandler_t signal(int sig, sighandler_t handler)
{
    return NULL;
}

int kill(pid_t id, int sig)
{
    struct process *proc = get_process(id);
    if(proc->p_uid == getuid() || getuid() == 0) {
        if(sig == SIGKILL) {
            really_kill_process(id);
        }
    } else {
        return -1;
    }
}

static void really_kill_process(pid_t id)
{
    struct process *proc = get_process(id);
    proc->p_kill = 1;
    proc->p_ttl = 2;
    proc->p_status = -1;
}
