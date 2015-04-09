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
 * fbtty.c
 * Driver for a kernel terminal emulator using the VBE frame buffer
 */


#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/tty.h>
#include <infinity/common.h>
#include <infinity/device.h>
#include <infinity/fcntl.h>
#include <infinity/drivers/framebuffer.h>
#include "fb_font.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 12

struct fb_tty_info {
    int                 position;
    int                 width;
    int                 height;
    int                 fg_color;
    int                 bg_color;
    int                 transparency_index;
    char                underline;
    char                hidden;
    char                bright;
    char                escaped;
    char                escape_buf[64];
    char                escape_pos;
    char                input_buf[128];
    char                input_pos;
    char *              z_buf;
    int *               wallpaper;
    struct fb_info *    f_info;
};
const int fb_pallete[] = {0x000000, 0xAA0000, 0x00AA00, 0xAA5500, 0x0000AA, 0xAA00AA, 0x00AAAA, 0xAAAAAA};

static void fb_putc(struct fb_tty_info *info, char c);
static void fb_putc_esc(struct fb_tty_info *info, char c);
static void fb_set_attrs(struct fb_tty_info *info, const char *attr);
static int fb_set_attr(struct fb_tty_info *info, int i, int *attributes);
static void fb_set_home(struct fb_tty_info *info, const char *attr);
static void fb_reset(struct fb_tty_info *info);
static void fb_newline(struct fb_tty_info *info);
static void fb_scroll(struct fb_tty_info *info);
static void fb_clear(struct fb_tty_info *info);
static size_t fb_tty_write(void *tag, const char *msg, size_t len, int addr);
static size_t fb_tty_read(void *tag, char *buf, size_t len, int addr);
static void fb_tty_recieve(struct tty *t, char c);
static char fb_tty_getc(struct fb_tty_info *info);
static void fb_drawc(struct fb_tty_info *info, int x, int y, int val, int fg, int bg);

/*
 * Initialize the terminal driver
 */
void init_fbtty(struct fb_info *info)
{
    struct fb_tty_info *t_info = (struct fb_tty_info*)kalloc(sizeof(struct fb_tty_info));
    memset(t_info, 0, sizeof(struct fb_tty_info));
    t_info->width = info->res_x / FONT_WIDTH;
    t_info->height = info->res_y / FONT_HEIGHT;
    t_info->f_info = info;
    struct tty *fb_tty = tty_create();
    fb_tty->t_device->dev_tag = t_info;
    fb_tty->t_device->write = fb_tty_write;
    fb_tty->t_device->read = fb_tty_read;
    fb_tty->writec = fb_tty_recieve;
    fb_reset(t_info);
    set_tty(fb_tty);
    klog_output(fb_tty->t_device);
    t_info->wallpaper = kalloc(800 * 600 * 4);
    struct file *bg = fopen("/lib/infinity/avril.raster", O_RDWR);
  
    fread(bg, t_info->wallpaper, 0, 800 * 600 * 4);
    
    fclose(bg);
}

/*
 * Set a pixel
 */ 
static inline void fb_set_pixel(struct fb_tty_info *info, int x, int y, int c)
{
    struct fb_info *f_info = info->f_info;
    int pos = x * f_info->depth + y * f_info->pitch;
    char *screen = (char*)f_info->frame_buffer;
    if(c == 0xFF000000) {
        c = info->wallpaper[y * f_info->res_x + x];
    }
    screen[pos] = c & 255;
    screen[pos + 1] = (c >> 8) & 255;
    screen[pos + 2] = (c >> 16) & 255;
}

/*
 * Will write a signal character to the terminal 
 */
static void fb_putc(struct fb_tty_info *info, char c)
{
    int x = info->position % info->width * FONT_WIDTH;
    int y = info->position / info->width * FONT_HEIGHT;
    if(!info->escaped) {
        switch(c) {
            case '\n':
                fb_newline(info);
                break;
            case '\b':
                info->position--;
                fb_putc(info, ' ');
                info->position--;
                break;
            case '\x1b':
                info->escaped = 1;
                break;
            default:
                fb_drawc(info, x, y, c, info->fg_color, info->bg_color);
                info->position++;
                break;
        }
    } else {
        fb_putc_esc(info, c);
    }
    
    if(info->position >= info->height * info->width) {
        fb_scroll(info);
    }
}

/*
 * Writes a character to a buffer containing an escape 
 * sequence
 */ 
static void fb_putc_esc(struct fb_tty_info *info, char c)
{
    if(info->escape_pos == 0 && c == 'c') {
        fb_reset(info);
        fb_clear(info);
        info->escaped = 0;
    } else if(c == 'm' && info->escape_buf[0] == '[') { 
        fb_set_attrs(info, info->escape_buf);
        info->escaped = 0;
    } else if ((c == 'H' || c == 'f') && info->escape_pos == 0) {
        fb_set_home(info, info->escape_buf);
        info->escaped = 0;
    }
    info->escape_buf[info->escape_pos] = c;
    info->escape_pos++;
    if(!info->escaped || info->escape_pos > 64) {
        memset(info->escape_buf, 0, 64);
        info->escape_pos = 0;
    }
}

/*
 * Parses a list of attributes
 */
static void fb_set_attrs(struct fb_tty_info *info, const char *attr)
{
    int attributes[32];
    int k = 0;
    int i = 1;
    while(i < info->escape_pos) {
        char tmp[64];
        int j = 0;
        while(i < info->escape_pos && info->escape_buf[i] != ';') {
            tmp[j++] = info->escape_buf[i++];
        }
        i++;
        tmp[j] = 0;
        attributes[k++] = strtol(tmp, 10);
    }
    i = 0;
    while(i < k) {
        i += fb_set_attr(info, i, attributes);
    }
}

/*
 * Sets a signal attribute, some attributes require arguments so we 
 * will return the amount of int's read from the attribute list
 */
static int fb_set_attr(struct fb_tty_info *info, int i, int *attributes)
{
    int a = attributes[i];
    if(a >= 30 && a <= 37) {
        int col = a - 30;
        info->fg_color = fb_pallete[col];
        if(info->bright)
            info->fg_color += 0x555555;
        return 1;
    } else if(a >= 40 && a <= 47) {
        int col = a - 40;
        info->bg_color = fb_pallete[col];
        if(info->bright)
            info->bg_color += 0x555555;
        return 1;
    }
    
    switch(a) {
        case 0:
            fb_reset(info);
            return 1;
        case 1:
            info->bright = 1;
            return 1;
        case 2:
            info->bright = 0;
            return 1;
        case 4:
            info->underline = 1;
            return 1;
        case 8:
            info->hidden = 1;
            return 1;
    }
    return 0;
}

/*
 * Sets the cursor position, if attr is empty it will default to 
 * 0, 0
 */
static void fb_set_home(struct fb_tty_info *info, const char *attr)
{
    int coords[2] = {0, 0};
    int k = 0;
    int i = 1;
    while(i < info->escape_pos && k < 2) {
        char tmp[64];
        int j = 0;
        while(i < info->escape_pos && info->escape_buf[i] != ';') {
            tmp[j++] = info->escape_buf[i++];
        }
        i++;
        tmp[j] = 0;
        coords[k++] = strtol(tmp, 10);
    }
    i = 0;
    info->position = info->width * coords[1] + coords[0];
}

/*
 * Resets the terminal to default settings
 */
static void fb_reset(struct fb_tty_info *info)
{
    info->bright = 0;
    info->underline = 0;
    info->hidden = 0;
    info->fg_color = fb_pallete[7];
    info->bg_color = 0xFF000000;
    info->transparency_index = 0xFF000000;
}

/*
 * Creates a new line
 */
static void fb_newline(struct fb_tty_info *info)
{
    if (info->position % info->width == 0) {
        info->position += info->width;
        return;
    }
    while (info->position % info->width) info->position++;
}

/*
 * Scrolls up one row
 */
static void fb_scroll(struct fb_tty_info *t_info)
{
    struct fb_info *info = t_info->f_info;
    char *fb = info->frame_buffer;
    int rlen = info->pitch * FONT_HEIGHT;
    memcpy(fb, fb + rlen, info->frame_buffer_length - rlen);
    memset(fb + info->frame_buffer_length - rlen, 0, rlen); 
    t_info->position = t_info->width * (t_info->height - 1);
}

/*
 * Clears the screen
 */ 
static void fb_clear(struct fb_tty_info *info)
{
    info->position = 0;
    int i = 0;
    for(int y = 0; y < 600; y++) {
        for(int x = 0; x < 800; x++) {
            fb_set_pixel(info, x, y, info->wallpaper[i++]);
        }
    }
}

static size_t fb_tty_write(void *tag, const char *msg, size_t len, int addr)
{
    struct fb_tty_info *info = (struct fb_tty_info*)tag;
    for(int i = 0; i < len; i++) {
        fb_putc(info, msg[i]);      
    }
    return len;
}

/*
 * Reads from the input buffer
 */
static size_t fb_tty_read(void *tag, char *buf, size_t len, int addr)
{
    struct fb_tty_info *info = (struct fb_tty_info*)tag;
    int i = 0;
    while(i < len) {
        char c = fb_tty_getc(info);
        if(c == '\n') {
            break;
        } else if (c == '\b') {
          buf[--i] = 0;  
        } else {
            buf[i++] = c;
        }
    }
    buf[i++] = 0;
    return len;
}

/*
 * Recieve a character from stdin
 */
static void fb_tty_recieve(struct tty *t, char c)
{
    struct fb_tty_info *info = (struct fb_tty_info*)t->t_device->dev_tag;
    if(info->input_pos < 128) {
        fb_putc(info, c);
        info->input_buf[info->input_pos++] = c;
    }
}

/*
 * Read signal character from the input stream
 */
static char fb_tty_getc(struct fb_tty_info *info)
{
    while(info->input_pos == 0)
        thread_yield();
    return info->input_buf[--info->input_pos];
}

static void fb_drawc(struct fb_tty_info *info, int x, int y, int val, int fg, int bg)
{

    if (val > 128) {
        val = 4;
    }
    
    uint8_t *c = number_font[val];
    for (uint8_t i = 0; i < FONT_WIDTH; i++) {
        for (uint8_t j = 0; j < FONT_HEIGHT; j++) {
            if (c[j] & (1 << (8-i))) {
                fb_set_pixel(info, x + i, y + j, fg);
            } else {
                fb_set_pixel(info, x + i, y + j, bg);
            }
        }
        if(info->underline) {
            fb_set_pixel(info, x + i, y + FONT_HEIGHT - 1, fg);
        }
    }

}
