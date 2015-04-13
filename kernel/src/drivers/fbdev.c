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
 * fbdev.c
 * Device driver for accessing the VBE frame buffer
 */

#include <stdint.h>
#include <mboot.h>
#include <infinity/heap.h>
#include <infinity/kernel.h>
#include <infinity/device.h>
#include <infinity/paging.h>
#include <infinity/virtfs.h>
#include <infinity/fcntl.h>
#include <infinity/drivers/serial.h>
#include <infinity/drivers/framebuffer.h>

struct device *fb_dev;

static size_t fb_write(void *tag, const char *data, size_t s, uint32_t add);
static size_t fb_read(void *tag, const char *data, size_t s, uint32_t add);

static void sexify_framebuffer(struct fb_info *info, char *fb);

void init_fb(vbe_info_t *info)
{
    struct fb_info *tag = (struct fb_info*)kalloc(sizeof(struct fb_info));
    tag->res_x = info->Xres;
    tag->res_y = info->Yres;
    tag->pitch = info->pitch;
    tag->depth = info->pitch / info->Xres;
    tag->frame_buffer = (char*)info->physbase;
    tag->frame_buffer_length = info->Yres * info->pitch;
    fb_dev = device_create(CHAR_DEVICE, "lfb");
    fb_dev->write = fb_write;
    fb_dev->read = fb_read;
    fb_dev->dev_tag = tag;
    
    extern struct page_directory *current_directory;
    for(int i = 0; i < tag->frame_buffer_length; i += 0x1000) {
        page_alloc(current_directory, tag->frame_buffer + i, tag->frame_buffer + i, 1, 1);
    }
    init_fbtty(tag);
    //sexify_framebuffer(tag, tag->frame_buffer);
}

static inline void fb_set_pixel(struct fb_info *info, int x, int y, int c)
{
    int pos = x * info->depth + y * info->pitch;
    char *screen = (char*)info->frame_buffer;
    screen[pos] = c & 255;
    screen[pos + 1] = (c >> 8) & 255;
    screen[pos + 2] = (c >> 16) & 255;
}

/*
 * I got bored...
 */
static void sexify_framebuffer(struct fb_info *info, char *fb)
{
    int bg = open("/lib/infinity/avril.raster", O_RDWR);
    if(bg != -1) {
        int *zbuf = (int*)kalloc(800 * 600 * 4);
        
        read(bg, zbuf, 800 * 600 * 4);
        int i = 0 ;
        for(int y = 0; y < 600; y++) {
            for(int x = 0; x < 800; x++) {
                int col = zbuf[i++];
                fb_set_pixel(info, x, y, col);
            }
        }
        kfree(zbuf);
        close(bg);
    } else {
        printk(KERN_WARN "Booting WITHOUT sexy photo!\n");
    }
    
}
 
static size_t fb_write(void *tag, const char *data, size_t size, uint32_t addr)
{
    struct fb_info *info = (struct fb_info*)tag;
    int i;
    for(i = 0; i < size; i++) {
        int p = (addr + i) % info->frame_buffer_length;
        info->frame_buffer[p] = ((char*)data)[i];
    }
    return i;
}
 
static size_t fb_read(void *tag, const char *data, size_t size, uint32_t addr)
{
        struct fb_info *info = (struct fb_info*)tag;
        int i;
        for(i = 0; i < size; i++) {
                int p = (addr + i) % info->frame_buffer_length;
                *((char*)data + i) = info->frame_buffer[p];
        }
        return i;
}
