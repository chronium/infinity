/* Copyright (C) 2015 - GruntTheDivine (Sloan Crandell)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
/*
 * event.c
 * This is the infinity kernel's primary event bus. I don't
 * know if I'll keep this or not, just a wild idea I had.
 */
 
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
