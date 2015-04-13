/* Copyright (C) 2014 - GruntTheDivine (Sloan Crandell)
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

#ifndef INFINITY_TTY_H
#define INFINITY_TTY_H

#include <stdint.h>
#include <infinity/device.h>

#define TIOCGWINSZ  1

struct tty {
    struct device * t_device;
    void            (*writec) (struct tty *t, char c);
    struct tty *    next;
};

struct tty_size {
    uint16_t        t_width;
    uint16_t        t_height;
    uint16_t        t_xpixel;
    uint16_t        t_ypixel;
};

struct tty *create_tty();
void set_tty(struct tty *t);
struct tty *get_tty();
void tty_writec(char c);
char tty_readc();

#endif
