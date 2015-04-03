#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/kernel.h>
#include <infinity/device.h>
#include <infinity/tty.h>

static int next_tty = 0;
static struct tty *tty_list = NULL;

static struct tty *tty_add(struct tty *t);

struct tty *tty_create()
{
    char name[64];
    sprintf(name, "tty%d", next_tty++);
    struct device *ttyd = device_create(CHAR_DEVICE, name);
    struct tty *ret = (struct tty*)kalloc(sizeof(struct tty));
    ret->t_device = ttyd;
    ret->next = NULL;
    return tty_add(ret);
    
}

void set_tty(struct tty *t)
{
    
}

struct tty *get_tty()
{
    return NULL;
}

static struct tty *tty_add(struct tty *t)
{
    struct tty *i = tty_list;
    if(!i) {
        tty_list = t;
    } else {
        while(i) {
            i = i->next;
        }
        i->next = t;
    }
    return t;
}

