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
 * textscreen.c
 * Driver for hardware VGA text screen
 */

#include <infinity/heap.h>
#include <infinity/common.h>
#include <infinity/device.h>
#include <infinity/kernel.h>
#include <infinity/tty.h>
#include <infinity/sync.h>
#include <infinity/fifobuf.h>
#include <infinity/arch/portio.h>
#include <infinity/drivers/textscreen.h>

struct tty_info {
    int                 position;
    int                 prev_position;
    int                 width;
    int                 height;
    int                 fg_color;
    int                 bg_color;
    char                underline;
    char                hidden;
    char                bright;
    char                echo;
    char                escaped;
    char                escape_buf[64];
    char                escape_pos;
    spinlock_t          lock;
    struct fifo_buffer  input_buf;
    
};

/*
 * We will make this device available to outside
 * parties
 */
struct device *hardware_textscreen;

uint16_t *video_memory = (uint16_t *)0xB8000;

static int vga_pallete[] = {0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15};

static char hw_tty_getc(struct tty_info *info);
static void hw_tty_putc(struct tty_info *info, char c);
static void hw_tty_putc_esc(struct tty_info *info, char c);
static void hw_tty_set_attrs(struct tty_info *info, const char *attr);
static int hw_tty_set_attr(struct tty_info *info, int i, int *attributes);
static void hw_tty_set_home(struct tty_info *info, const char *attr);
static void hw_tty_scroll_region(struct tty_info *info, const char *attr);
static void hw_tty_request(struct tty_info *info, const char *attr);
static void hw_tty_clear(struct tty_info *info);
static void hw_tty_reset(struct tty_info *info);
static void hw_tty_newline(struct tty_info *info);
static void hw_tty_scroll(struct tty_info *info);
static void hw_tty_recieve(struct tty *t, char c);
static void hw_tty_setcurse(int position);
static void hw_drawc(struct tty_info *info, int x, int y, char c, int fg, int bg);
static size_t textscreen_write(void *tag, const char *msg, size_t len, int addr);
static size_t hw_tty_read(void *tag, char *buf, size_t len, int addr);

/*
 * Will initialize the device for the hardware textscreen
 * and clear the screen
 */
void init_textscreen()
{
    
    struct tty_info *t_info = (struct tty_info*)kalloc(sizeof(struct tty_info));
    t_info->width = 80;
    t_info->height = 25;
    struct tty *hwcon = tty_create();
    
    hwcon->t_device->dev_tag = t_info;
    hwcon->t_device->write = textscreen_write;
    hwcon->t_device->read = hw_tty_read;
    
    hwcon->writec = hw_tty_recieve;
    
    fifo_init(&t_info->input_buf, 4 * 4096);
    set_tty(hwcon);
    //klog_output(hwcon->t_device);
    hw_tty_clear(t_info);
    hw_tty_reset(t_info);
}

static char hw_tty_getc(struct tty_info *info)
{
    return fifo_readb(&info->input_buf);
}

static void hw_tty_putc(struct tty_info *info, char c)
{
    int x = info->position % info->width;
    int y = info->position / info->width;
    if(!info->escaped) {
        switch(c) {
            case '\n':
                if(!info->hidden)
                    hw_tty_newline(info);
                break;
            case '\b':
                info->position--;
                hw_tty_putc(info, ' ');
                info->position--;
                break;
            case '\t':
                info->position += 4 - (info->position % 4);
                break;
            case '\x1b':
                info->escaped = 1;
                break;
            default:
                if(!info->hidden)
                    hw_drawc(info, x, y, c, info->fg_color, info->bg_color);
                info->position++;
                hw_tty_setcurse(info->position - 1);
                break;
        }
    } else {
        hw_tty_putc_esc(info, c);
    }
    
    if(info->position >= info->height * info->width) {
        hw_tty_scroll(info);
    }
}


static void hw_tty_putc_esc(struct tty_info *info, char c)
{
    if(info->escape_pos == 0 && c == 'c') {
        hw_tty_reset(info);
        hw_tty_clear(info);
        info->escaped = 0;
    } else if(c == 'm' && info->escape_buf[0] == '[') { 
        hw_tty_set_attrs(info, info->escape_buf);
        info->escaped = 0;
    } else if(c == 's' && info->escape_buf[0] == '[') {
        info->prev_position = info->position;
        info->escaped = 0;
    } else if(c == 'u' && info->escape_buf[0] == '[') {
        info->position = info->prev_position;
        info->escaped = 0;
    } else if((c == 'H' || c == 'f') && info->escape_buf[0] == '[') {
        hw_tty_set_home(info, info->escape_buf);
        info->escaped = 0;
    } else if(c == 'n' && info->escape_buf[0] == '[') {
        info->escaped = 0;
        hw_tty_request(info, info->escape_buf);
    } else if(c == 'r' && info->escape_buf[0] == '[') {
        info->escaped = 0;
        hw_tty_scroll_region(info, info->escape_buf);
    } else if(c == 'l' && strncmp(info->escape_buf, "[12", 3) == 0) {
        info->escaped = 0;
        info->echo = 1;
    } else if(c == 'h' && strncmp(info->escape_buf, "[12", 3) == 0) {
        info->escaped = 0;
        info->echo = 0;
    }
    info->escape_buf[info->escape_pos] = c;
    info->escape_pos++;
    if(!info->escaped || info->escape_pos > 64) {
        memset(info->escape_buf, 0, 64);
        info->escape_pos = 0;
    }
}

/*
 * Sets terminal attributes
 */
static void hw_tty_set_attrs(struct tty_info *info, const char *attr)
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
        i += hw_tty_set_attr(info, i, attributes);
    }
}

static int hw_tty_set_attr(struct tty_info *info, int i, int *attributes)
{
    int a = attributes[i];
    if(a >= 30 && a <= 37) {
        int col = a - 30;
        info->fg_color = info->bright ? vga_pallete[col] : vga_pallete[col];
        return 1;
    } else if(a >= 40 && a <= 47) {
        int col = a - 40;
        info->bg_color = info->bright ? vga_pallete[col] : vga_pallete[col];
        return 1;
    }
    
    switch(a) {
        case 0:
            hw_tty_reset(info);
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
 * Clears the screen
 */
static void hw_tty_clear(struct tty_info *info)
{
    for (int i = 0; i < 2000; i++)
        video_memory[i] = 0;
    info->position = 0;
    info->echo = 1;
}

/*
 * Reset terminal to default setings
 */
static void hw_tty_reset(struct tty_info *info)
{
    info->bright = 0;
    info->hidden = 0;
    info->fg_color = 0x0F;
    info->bg_color = 0x00;
}

/*
 * Sets the cursor position, if attr is empty it will default to 
 * 0, 0
 */
static void hw_tty_set_home(struct tty_info *info, const char *attr)
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
    hw_tty_setcurse(info->position);
}

/*
 * Scrolls over an entire region
 */
static void hw_tty_scroll_region(struct tty_info *info, const char *attr)
{
    int rows[2] = {0, 0};
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
        rows[k++] = strtol(tmp, 10);
    }
    
    int start = rows[0] * info->width;
    int end = rows[1] * info->width;
    int amount = (end - start);
    for (int i = start + 80; i < amount; i++)
        video_memory[i - 80] = video_memory[i];
    for (int i = 0; i < 80; i++)
        video_memory[((end) - 80) + i] = 0;
}


/*
 * Handle a request
 */ 
static void hw_tty_request(struct tty_info *info, const char *attr)
{
    char response[16];
    memset(response, 0, 16);
    switch(attr[1]) {
        case '6': 
            sprintf(response, "\x1B[%d;%dR", info->position % info->width, info->position / info->width);
            break;
        default:
            return;
    }
    
    for(int i = 0; response[i] != 0; i++) {
        char c = response[i];
        if(response[i] != 0x1B && info->echo)
            hw_tty_putc(info, c);
        else if(info->echo) {
            hw_tty_putc(info, '^');
            hw_tty_putc(info, '[');
        }
        fifo_writeb(&info->input_buf, c);
    }
    
}

/*
 * Create a new line on the screen
 */ 
static void hw_tty_newline(struct tty_info *info)
{
    if (info->position % info->width == 0) {
        info->position += info->width;
        return;
    }
    while (info->position % info->width) info->position++;
}

/*
 * Perform a scroll
 */
static void hw_tty_scroll(struct tty_info *info)
{
    for (int i = 80; i < (80 * 25); i++)
        video_memory[i - 80] = video_memory[i];
    for (int i = 0; i < 80; i++)
        video_memory[((80 * 25) - 80) + i] = 0;
    info->position = info->width * (info->height - 1);
}

/*
 * Draws a single character (Ignores escape sequences
 * and control characters)
 */
static void hw_drawc(struct tty_info *info, int x, int y, char c, int fg, int bg)
{
    int attr = (((bg) << 4) | (fg & 0x0F)) << 8;
    video_memory[info->position] = c | attr;
}

/*
 * Updates the hardware cursor position
 */
static void hw_tty_setcurse(int position)
{
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

/*
 * Recieve a character from stdin
 */
static void hw_tty_recieve(struct tty *t, char c)
{
    struct tty_info *info = (struct tty_info*)t->t_device->dev_tag;
    if(info->echo)
        hw_tty_putc(info, c);
    fifo_writeb(&info->input_buf, c);
}

/*
 * Device call back to actually write to the textscreen
 */
static size_t textscreen_write(void *tag, const char *msg, size_t len, int addr)
{
    struct tty_info *info = (struct tty_info*)tag;
    spin_lock(&info->lock);
    for(int i = 0; i < len; i++) {
        hw_tty_putc(info, msg[i]);      
    }
    spin_unlock(&info->lock);
    return len;
}

static size_t hw_tty_read(void *tag, char *buf, size_t len, int addr)
{
    struct tty_info *info = (struct tty_info*)tag;
    int i = 0;
    while(i < len) {
        char c = hw_tty_getc(info);
        if(c == '\n') {
            buf[i++] = c;
            break;
        } else if (c == '\b') {
            if(i != 0)
                buf[--i] = 0;  
        } else{
            buf[i++] = c;
        }
    }
    buf[i++] = 0;
    
    return i;
}
