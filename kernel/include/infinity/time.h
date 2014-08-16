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
 
#ifndef TIME_H
#define TIME_H

#include <infinity/types.h>

struct tm {
	int 		tm_sec;
	int 		tm_min;
	int 		tm_hour;
	int 		tm_mday;
	int 		tm_mon;
	int 		tm_year;
	int 		tm_wday;
	int 		tm_yday;
	int 		tm_isdst;
};

extern time_t time(time_t* timer);
extern struct tm* gmtime(const time_t *timep);
extern struct tm* gmtime_r(const time_t *timep, struct tm* buf);

#endif
