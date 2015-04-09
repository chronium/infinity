#include <infinity/heap.h>
#include <infinity/event.h>
#include <infinity/kernel.h>


struct handler_entry {
    event_handler_t         e_handler;
    uint32_t                e_event;
    struct handler_entry *  next;
};


struct handler_entry *event_handlers = NULL;

/*
 * Subscribe to a kernel event on the kernel's
 * event bus
 * @param e         The event to subscribe to
 * @param handler   Event handler callback
 */
void event_subscribe(uint32_t e, event_handler_t handler)
{
    struct handler_entry *entry = (struct handler_entry*)kalloc(sizeof(struct handler_entry));
    entry->e_handler = handler;
    entry->e_event = e;
    struct handler_entry *i = event_handlers;
    if(i) {
        while(i->next) {
            i = i->next;
        }
        i->next = entry;
    } else {
        event_handlers = entry;
    }
}

/*
 * Unsubscribe an event handler
 * @param handler   Event handler callback
 */
void event_unsubscribe(event_handler_t handler)
{
}

void event_dispatch(uint32_t e, void *args)
{
    struct handler_entry *i = event_handlers;
    while(i) {
        if(i->e_event == e) {
            i->e_handler(e, args);
        }
        i = i->next;
    }
}
