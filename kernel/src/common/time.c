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
 * time.c
 * Implements several methods found in time.h
 * Note: the implemention of time() is found in
 * hardware/rtc.c
 */

#include <stdint.h>
#include <stddef.h>
#include <infinity/types.h>
#include <infinity/time.h>


struct tm *gmtime(const time_t *timep)
{
}

struct tm *gmtime_r(const time_t *timep, struct tm *buf)
{
	time_t t = time(NULL);

	buf->tm_hour = (t / 3600) % 24;
	buf->tm_min = (t / 60) % 60;
	buf->tm_sec = t % 60;
	return buf;
}
