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
 * open.c
 * Provides functions for creating file streams ontop of the virtual filesystem
 */

#include <stddef.h>
#include <infinity/kernel.h>
#include <infinity/dirent.h>
#include <infinity/heap.h>
#include <infinity/virtfs.h>


static struct file_table_entry *file_table = NULL;

static void add_to_file_table(struct file *file);
static int remove_from_file_table(struct file *file);

/*
 * Opens a file returning a file struct
 * @param path			The path to the file
 * @param oflags		Flags to open the file with
 */
struct file *fopen(const char *path, int oflags)
{
	printk(KERN_INFO "DEBUG: Open %s\n", path);
	struct file *new_file = (struct file *)kalloc(sizeof(struct file));
	memset(new_file, 0, sizeof(struct file));
	int res = virtfs_open(new_file, path, oflags);
	if (res == 0) {
		add_to_file_table(new_file);
		return new_file;
	}
	printk(KERN_ERR "ERROR: virtfs_open() failed!\n");

	kfree(new_file);

	return NULL;
}

/*
 * Closes a file and removes it from the global OS file
 * table
 * @param f				The file to close
 */
int fclose(struct file *f)
{
	return remove_from_file_table(f);
}

/*
 * Adds a file struct to the global OS file table
 */
static void add_to_file_table(struct file *nfile)
{
	struct file_table_entry *entry = (struct file_table_entry *)kalloc(sizeof(struct file_table_entry));

	entry->next = NULL;
	entry->file_entry = nfile;
	if (!file_table) {
		file_table = entry;
	} else {
		struct file_table_entry *i = file_table;
		while (i->next)
			i = i->next;
		i->next = entry;
	}
}

/*
 * Removes a file from the global OS file table
 */
static int remove_from_file_table(struct file *file)
{
	struct file_table_entry *i = file_table;
	struct file_table_entry *last = NULL;

	while (i) {
		if (i->file_entry == file) {
			if (last)
				last->next = i->next;
			else
				file_table = i->next;
			kfree(i->file_entry);
			kfree(i);
			return 0;
		}

		last = i;
		i = i->next;
	}
	return -1;
}
