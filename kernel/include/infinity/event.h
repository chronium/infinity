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


#ifndef INFINITY_EVENT_H
#define INFINITY_EVENT_H

#include <stdint.h>

typedef void (*event_handler_t)(int32_t e, void *args);

enum events {
    KERNEL_INIT =       0,
    KERNEL_SHUTDOWN =   1,
    PROCESS_CREATE =    2,
    PROCESS_DESTROY =   3,
    FILDES_OPEN =       4,
    FILDES_CLOSE =      5,
};

void event_subscribe(uint32_t e, event_handler_t handler);
void event_unsubscribe(event_handler_t handler);
void event_dispatch(uint32_t e, void *args);

#endif
