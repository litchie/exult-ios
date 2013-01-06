/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *	Original file by Dancer A.L Vesperman
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


#include "IFF.h"

#include <iostream>
#include <fstream>
#include "exceptions.h"
#include "utils.h"
#include "databuf.h"

using std::string;
using std::vector;

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::cerr;
using std::endl;
using std::memcmp;
using std::size_t;
#endif

/**
 *	Reads the header from an IFF and builds an object index.
 */
void IFF::index_file(void)
	{
	if (!data)
		throw file_read_exception(identifier.name);

	if (!is_iff(data))	// Not an IFF file we recognise
		throw wrong_file_type_exception(identifier.name, "IFF");

#ifdef DEBUG
	cout << "Okay. It looks like an IFF file chunk" << endl;
#endif

	data->skip(4);	// Skip past identifier.
	size_t full_length = data->read4high();

#ifdef DEBUG
	cout << "length looks like: " << full_length << endl;
#endif

	// We don't really need to know what the general data type is
	data->skip(4);


/*
-the objects entries
	entry   = type, size, object, [even]
	type    = 4 chars representing the type of this object
	size    = reversed longint (size of the entry excluding the first 8 bytes)
	even    = 1 byte (set to 0) present only to get an even number of bytes
	(the objects found in U7 IFF files have the following format:)
	object  = name, data
	name    = 8 chars (filled with 0s)
	data    = the data of the object
*/

	while(data->getPos() < full_length)
		{
		Reference	r;
		char type[4];
		data->read(type, 4);

		r.size = data->read4high();	// 4 bytes for len
		r.offset = data->getPos();
		
		if(r.size == 0 || r.offset == 0)
			break;
		object_list.push_back(r);

		// Objects are word-aligned in IFF files.
		data->seek(r.offset + r.size + (r.size&1));
		}
	}

/**
 *	Reads the desired object from the iff.
 *	@param objnum	Number of object to read.
 *	@param len	Receives the length of the object, or zero in any failure.
 *	@return	Buffer created with new[] containing the object data or
 *	null on any failure.
 */
char *IFF::retrieve(uint32 objnum, size_t &len)
	{
	if (!data || objnum >= object_list.size())
		{
		len = 0;
		return 0;
		}
#if 0
	// Trying to avoid exceptions.
	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in IFF::retrieve()");
#endif

	data->seek(object_list[objnum].offset);
	len = object_list[objnum].size;
	char *buffer = new char[len];
	data->read(buffer, len);

	return buffer;
	}

/**
 *	Verify if a file is an IFF.  Note that this is a STATIC method.
 *	@param in	DataSource to verify.
 *	@return	Whether or not the DataSource is an IFF file.
 */
bool IFF::is_iff(DataSource *in)
	{
	char ckid[4];
	long pos = in->getPos();
	in->seek(0);
	in->read(ckid, 4);
	in->seek(pos);
	return !memcmp(ckid,"FORM",4);
	}

/**
 *	Verify if a file is an IFF.  Note that this is a STATIC method.
 *	@param fname	Name of file to verify.
 *	@return	Whether or not the file is an IFF file. Returns false if
 *	the file does not exist.
 */
bool IFF::is_iff(const char *fname)
	{
	if (!U7exists(fname))
		return false;

	std::ifstream in;
	U7open (in, fname);
	StreamDataSource ds(&in);

	if (in.good())
		return is_iff(&ds);

	in.close();
	return false;
	}
