#include <infinity/kernel.h>
#include <infinity/heap.h>
#include <infinity/tty.h>
#include <infinity/common.h>
#include <infinity/device.h>
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
    char                escaped;
    char                escape_buf[64];
    char                escape_pos;
    struct fb_info *    f_info;
};
const int fb_pallete[] = {0x000000, 0xAA0000, 0x00AA00, 0xAA5500, 0x0000AA, 0xAA00AA, 0x00AAAA, 0xAAAAAA};

static void fb_putc(struct fb_tty_info *info, char c);
static void fb_putc_esc(struct fb_tty_info *info, char c);
static void fb_set_attrs(struct fb_tty_info *info, const char *attr);
static int fb_set_attr(struct fb_tty_info *info, int i, int *attributes);
static void fb_reset(struct fb_tty_info *info);
static void fb_newline(struct fb_tty_info *info);
static void fb_scroll(struct fb_tty_info *info);
static size_t fb_tty_write(void *tag, const char *msg, size_t len, int addr);
static void fb_drawc(struct fb_tty_info *info, int x, int y, int val, uint32_t fg, uint32_t bg);

void init_fbtty(struct fb_info *info)
{
    struct fb_tty_info *t_info = (struct fb_tty_info*)kalloc(sizeof(struct fb_tty_info));
    memset(t_info, 0, sizeof(struct fb_tty_info));
    t_info->position = 0;
    t_info->width = info->res_x / FONT_WIDTH;
    t_info->height = info->res_y / FONT_HEIGHT;
    t_info->f_info = info;
    t_info->fg_color = 0xFFFFFF;
    struct tty *fb_tty = tty_create();
    fb_tty->t_device->dev_tag = t_info;
    fb_tty->t_device->write = fb_tty_write;

    klog_output(fb_tty->t_device);
}

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

static void fb_putc_esc(struct fb_tty_info *info, char c)
{
    if(info->escape_pos == 0 && c == 'c') {
        // Clear the screen
    } else if(c == 'm' && info->escape_buf[0] == '[') { 
        fb_set_attrs(info, info->escape_buf);
        info->escaped = 0;
    }
    info->escape_buf[info->escape_pos] = c;
    info->escape_pos++;
    if(!info->escaped || info->escape_pos > 64) {
        memset(info->escape_buf, 0, 64);
        info->escape_pos = 0;
    }
}

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

static int fb_set_attr(struct fb_tty_info *info, int i, int *attributes)
{
    int a = attributes[i];
    if(a >= 30 && a <= 37) {
        int col = 37 - a;
        info->fg_color = fb_pallete[col];
        return 1;
    } else if(a >= 40 && a <= 47) {
        int col = 47 - a;
        info->bg_color = fb_pallete[col];
        return 1;
    }
    
    switch(a) {
        case 0:
            fb_reset(info);
            return 1;
    }
    return 0;
}

static void fb_reset(struct fb_tty_info *info)
{
}

static void fb_newline(struct fb_tty_info *info)
{
    if (info->position % info->width == 0) {
        info->position += info->width;
        return;
    }
    while (info->position % info->width) info->position++;
}

static void fb_scroll(struct fb_tty_info *t_info)
{
    struct fb_info *info = t_info->f_info;
    char *fb = info->frame_buffer;
    int rlen = info->pitch * FONT_HEIGHT;
    memcpy(fb, fb + rlen, info->frame_buffer_length - rlen);
    memset(fb + info->frame_buffer_length - rlen, 0, rlen); 
    t_info->position = t_info->width * (t_info->height - 1);
}

static size_t fb_tty_write(void *tag, const char *msg, size_t len, int addr)
{
    struct fb_tty_info *info = (struct fb_tty_info*)tag;
    for(int i = 0; i < len; i++) {
        fb_putc(info, msg[i]);      
    }
    return len;
}

static inline void fb_set_pixel(struct fb_info *info, int x, int y, int c)
{
    int pos = x * 3 + y * info->pitch;
    char *screen = (char*)info->frame_buffer;
    screen[pos] = c & 255;
    screen[pos + 1] = (c >> 8) & 255;
    screen[pos + 2] = (c >> 16) & 255;
}

static void fb_drawc(struct fb_tty_info *info, int x, int y, int val, uint32_t fg, uint32_t bg)
{
    if (val > 128) {
        val = 4;
    }

    uint8_t * c = number_font[val];
    for (uint8_t i = 0; i < FONT_HEIGHT; ++i) {
        for (uint8_t j = 0; j < FONT_WIDTH; ++j) {
            if (c[i] & (1 << (8-j))) {
                fb_set_pixel(info->f_info, x + j, y + i, fg);
            } else
                fb_set_pixel(info->f_info, x + j, y + i, bg);
        }
    }

}
