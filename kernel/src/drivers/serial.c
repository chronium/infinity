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
 * serial.c
 * Driver for communicating with serial devices
 */

#include <stddef.h>
#include <infinity/device.h>
#include <infinity/portio.h>
#include <infinity/drivers/serial.h>

#define COM_PORT                        0x3F8

struct device *serial_dev1 = NULL;
static struct device *serial_dev2 = NULL;
static struct device *serial_dev3 = NULL;
static struct device *serial_dev4 = NULL;

static int serial_is_empty();
static void serial_putc(char c);
static size_t serial_write_com1(void *tag, const char *data, size_t s, uint32_t add);

void init_serial()
{
    serial_dev1 = device_create(CHAR_DEVICE, "ttyS0");
    serial_dev1->write = serial_write_com1;

    outb(COM_PORT + 1, 0x00);
    outb(COM_PORT + 3, 0x80);
    outb(COM_PORT + 0, 0x03);
    outb(COM_PORT + 1, 0x00);
    outb(COM_PORT + 3, 0x03);
    outb(COM_PORT + 2, 0xC7);
    outb(COM_PORT + 4, 0x0B);
}

/*
 * Devices file call back
 */
static size_t serial_write_com1(void *tag, const char *data, size_t s, uint32_t add)
{
    for (int i = 0; i < s; i++)
        serial_putc(data[i]);
    return s;
}

/*
 * Writes a signal character to the serial port
 */
static void serial_putc(char c)
{
    while (!serial_is_empty());
    outb(COM_PORT, c);
}

/*
 * Does the serial controller have any data for us to read?
 */
static int serial_is_empty()
{
    return inb(COM_PORT + 5) & 0x20;
}
