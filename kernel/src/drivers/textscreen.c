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
#include <infinity/drivers/textscreen.h>


/*
 * We will make this device available to outside 
 * parties
 */
struct device *hardware_textscreen;

uint16_t *video_memory = (uint16_t *)0xB8000;

static int foreground_color = 0x0F;
static int background_color = 0x00;
static int terminal_pos = 0;

/* Static method prototypes */
static void textscreen_newline();
static void textscreen_scroll();
static void textscreen_putc(const char);
static size_t textscreen_write(void *tag, const char *data, size_t s, uint32_t add);

/*
 * Will initialize the device for the hardware textscreen
 * and clear the screen
 */
void init_textscreen()
{
    hardware_textscreen = device_create(CHAR_DEVICE, "hwcon");
    hardware_textscreen->write = textscreen_write;
    klog_output(hardware_textscreen);
    clrscr();
}


/*
 * Clears the screen
 */
void clrscr()
{
    for (int i = 0; i < 2000; i++)
        video_memory[i] = 0;
    terminal_pos = 0;
}

/*
 * Create a new line on the screen
 */
static void textscreen_newline()
{
    if (terminal_pos % 80 == 0) {
        terminal_pos += 80;
        return;
    }
    while (terminal_pos % 80) terminal_pos++;
}

/*
 * Perform a scroll
 */
static void textscreen_scroll()
{
    memcpy(video_memory, video_memory + 80, (80 * 25) - 80);
    for (int i = 80; i < (80 * 25); i++)
        video_memory[i - 80] = video_memory[i];
    for (int i = 0; i < 80; i++)
        video_memory[((80 * 25) - 80) + i] = 0;
    terminal_pos = (80 * 25) - 80;
}

/*
 * Writes a single character to the screen, and will
 * regonize control characters
 */
static void textscreen_putc(const char c)
{
    if (c == '\n') {
        textscreen_newline();
    } else if (c == '\b') {
        terminal_pos--;
        textscreen_putc(' ');
        terminal_pos--;
    } else {
        if (terminal_pos >= 2000)
            textscreen_scroll();
        int attr = (((background_color) << 4) | (foreground_color & 0x0F)) << 8;
        video_memory[terminal_pos++] = c | attr;
    }
}

/*
 * Device call back to actually write to the textscreen
 */
static size_t textscreen_write(void *tag, const char *data, size_t s, uint32_t add)
{
    for (int i = 0; i < s; i++)
        textscreen_putc(data[i]);
    return s;
}

