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

/*
 * textscreen.c
 * Simple driver for hardware textmode
 */
 
#include <stddef.h>
#include <stdint.h>
#include <infinity/common.h>
#include <infinity/device.h>
#include <infinity/textscreen.h>

int foreground_color;
int background_color;
device_t textscreen_device;

static uint32_t dummy;
static uint16_t* vram = (uint16_t*)0xB8000;
static int pos;

static void textscreen_newline();
static void textscreen_scroll();
static void textscreen_putc(const char);
static size_t textscreen_write (const char* data, size_t s, uint32_t add);

static void textscreen_newline()
{
	if(pos % 80 == 0)
	{
		pos += 80;
		return;
	}
	while(pos % 80 != 0)
		pos++;
}


static void textscreen_scroll()
{
	memcpy(vram, vram + 80, (80 * 25) - 80);
	for(int i = 80; i < (80 * 25); i++)
		vram[i - 80] = vram[i];
	for(int i = 0; i < 80; i++)
		vram[((80 * 25) - 80) + i] = 0;
	pos = (80 * 25) - 80;
}


static void textscreen_putc(const char c)
{
	if(c == '\n')
		textscreen_newline();
	else if (c == '\b')
	{
		pos--;
		textscreen_putc(' ');
		pos--;
	}
	else
	{
		if(pos >= 2000)
			 textscreen_scroll();
		int attr = (((background_color)<< 4) | (foreground_color & 0x0F)) << 8;
		vram[pos++] = c | attr;
		
	}
}

void set_attributes(char fg, char bg)
{
	foreground_color = fg;
	background_color = bg;
}

void clrscr()
{
	for(int i = 0; i < 2000; i++)
		vram[i] = 0;
	pos = 0;
}


void init_textscreen()
{
	set_attributes(15, 0);
	clrscr();
	textscreen_device.device_name = "textscreen";
	textscreen_device.device_type = DEV_CHAR_DEVICE;
	textscreen_device.write = textscreen_write;
	register_device(&textscreen_device);
}

static size_t textscreen_write (const char* data, size_t s, uint32_t add)
{
	for(int i = 0; i < s; i++)
		textscreen_putc(data[i]);
	return s;
}
