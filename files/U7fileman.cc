/*
 *  Copyright (C) 2008  The Exult Team
 *
 *  Based on code by Dancer A.L Vesperman
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>
#include <iostream>
#include <cstdlib>

#include "U7fileman.h"
#include "U7obj.h"
#include "U7file.h"
#include "Flex.h"
#include "IFF.h"
#include "Table.h"
#include "Flat.h"
#include "utils.h"

using std::string;
using std::map;

static U7FileManager filemanager;
U7FileManager *U7FileManager::self = &filemanager;

/**
 *  Searches for the desired "file". If it is not already open,
 *  it is opened by the creation of an appropriate reader class.
 *  @param s    Unique identifier.
 *  @param allow_errors If false, this function will throw an
 *  exception if the file cannot be found. If true, it will return
 *  a null file pointer instead.
 *  @return Pointer to data reading class.
 */
U7file *U7FileManager::get_file_object(const File_spec &s, bool allow_errors) {
	if (file_list.count(s))
		return file_list[s];

	// Not in our cache. Attempt to figure it out.
	U7file *uf = 0;
	if (s.index >= 0) {
		U7object from(s.name, s.index);
		std::size_t size;
		char *buffer = from.retrieve(size);
		BufferDataSource *data = new BufferDataSource(buffer, size);
		if (IFF::is_iff(data))
			uf = new IFFBuffer(s, data);
		else if (Flex::is_flex(data))
			uf = new FlexBuffer(s, data);
		else if (Table::is_table(data))
			uf = new TableBuffer(s, data);
		else if (Flat::is_flat(data))
			uf = new FlatBuffer(s, data);
		else {
			// All other cases manage this, so we don't have to.
			delete data;
			delete [] buffer;
		}
	} else {
		if (IFF::is_iff(s.name))
			uf = new IFFFile(s.name);
		else if (Flex::is_flex(s.name))
			uf = new FlexFile(s.name);
		else if (Table::is_table(s.name))
			uf = new TableFile(s.name);
		else if (Flat::is_flat(s.name))
			uf = new FlatFile(s.name);
	}

	// Failed
	if (!uf) {
		if (!allow_errors)
			throw(file_open_exception(s.name));
		return 0;
	}

	file_list[s] = uf;
	return uf;
}

/**
 *  Cleans the whole file list.
 */
void U7FileManager::reset() {
	for (map<File_spec, U7file *>::iterator it = file_list.begin();
	        it != file_list.end(); ++it)
		delete it->second;

	file_list.clear();
}
