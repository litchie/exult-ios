/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  Original file by Dancer A.L Vesperman
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

#include "Table.h"

#include <iostream>
#include <cstdlib>
#include "exceptions.h"
#include "utils.h"

using std::string;
using std::vector;

using std::cout;
using std::cerr;
using std::endl;
using std::FILE;
using std::size_t;

void Table::index_file(void) {
	if (!data)
		throw file_read_exception(identifier.name);

	if (!is_table(data))    // Not a table file we recognise
		throw wrong_file_type_exception(identifier.name, "TABLE");

	unsigned int i = 0;
	while (true) {
		Table::Reference f;
		f.size = data->read2();

		if (f.size == 65535)
			break;

		f.offset = data->read4();

#if 0
		// We already guarded against this above.
		if (f.size > file_size || f.offset > file_size)
			throw wrong_file_type_exception(filename, "TABLE");
#endif
#if 0
		cout << "Item " << i << ": " << f.size << " @ " << f.offset << endl;
#endif
		object_list.push_back(f);
		i++;
	}
}

/**
 *  Reads the desired object from the table file.
 *  @param objnum   Number of object to read.
 *  @param len  Receives the length of the object, or zero in any failure.
 *  @return Buffer created with new[] containing the object data or
 *  null on any failure.
 */
char *Table::retrieve(uint32 objnum, size_t &len) {
	if (!data || objnum >= object_list.size()) {
		len = 0;
		return 0;
	}
#if 0
	// Trying to avoid exceptions.
	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in Flex::retrieve()");
#endif

	data->seek(object_list[objnum].offset);
	len = object_list[objnum].size;
	char *buffer = new char[len];
	data->read(buffer, len);

	return buffer;
}

/**
 *  Verify if a file is a table.  Note that this is a STATIC method.
 *  @param in   DataSource to verify.
 *  @return Whether or not the DataSource is a table file.
 */
bool Table::is_table(DataSource *in) {
	size_t pos = in->getPos();
	size_t file_size = in->getSize();

	in->seek(0);
	while (true) {
		uint16 size = in->read2();

		// End of table marker.
		if (size == 65535)
			break;

		uint32 offset = in->read4();
		if (size > file_size || offset > file_size) {
			in->seek(pos);
			return false;
		}
	}

	in->seek(pos);
	return true;
}

/**
 *  Verify if a file is a table.  Note that this is a STATIC method.
 *  @param fname    Name of file to verify.
 *  @return Whether or not the file is a table file. Returns false if
 *  the file does not exist.
 */
bool Table::is_table(const char *fname) {
	if (!U7exists(fname))
		return false;

	std::ifstream in;
	U7open(in, fname);
	StreamDataSource ds(&in);

	if (in.good())
		return is_table(&ds);

	in.close();
	return false;
}
