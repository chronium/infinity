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
 
#ifndef INFINITY_DRIVERS_FRAMEBUFFER_H
#define INFINITY_DRIVERS_FRAMEBUFFER_H

#include <mboot.h>

struct fb_info {
        uint32_t        res_x;
        uint32_t        res_y;
        uint16_t        pitch;
        uint8_t         depth;
        uint32_t        frame_buffer_length;
        char *          frame_buffer;
};
 
void init_fb(vbe_info_t *info);
void init_fbtty(struct fb_info *info);
#endif
