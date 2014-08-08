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


#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

extern int strlen(const char* str);
extern char* strrchr (const char* str, char ch);
extern char* strlchr (const char* str, char ch);
extern char* strcpy (char * dest, const char* src);
extern char* strncpy (char* dest, const char* src, size_t len);
extern int strncmp(const char* str1, const char* str2, size_t len);
extern int strcmp(const char* str1, const char* str2);
extern char* strcat(char* dest, const char* dat);
extern char* strncat(char* dest, const char* dat, size_t dlen);
extern void reverse(char* rv);
extern void itoa(char* s,int i);
extern int atoi(char* s);
extern void itox(char* s, unsigned int i);
extern void vsprintf(char* dest, const char* format, va_list argp);
extern void sprintf(char* dest, const char* format, ...);
extern int strindx (const char* str, char ch);
extern int strtol(char* s, int base);
extern void* memcpy(void* dest, void* src, size_t size);
extern void* memset(void* source, uint8_t b, size_t size);

#endif
