/*
 *  Copyright (C) 2000-2008  The Exult Team
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

#define	DEBUGFLEX 0

#include "Flex.h"

#include <fstream>
#include <iostream>
#include "exceptions.h"
#include "utils.h"
#include "databuf.h"

#ifndef UNDER_EMBEDDED_CE
using std::endl;
using std::memset;
using std::size_t;
using std::string;
using std::strncpy;
using std::ofstream;
using std::ios;
#endif

/**
 *	Reads the header from a flex and builds an object index.
 */
void Flex::index_file()
	{
	if (!data)
		throw file_read_exception(identifier.name);

	data->read(title, sizeof(title));
	magic1 = data->read4();
	count = data->read4();
	magic2 = data->read4();

	if(magic1!=0xffff1a00UL)
			// Not a flex file.
		throw wrong_file_type_exception(identifier.name, "FLEX");

	for(int i=0;i<9;i++)
		padding[i] = data->read4();
#if DEBUGFLEX
	cout << "Title: " << title << endl;
	cout << "Count: " << count << endl;
#endif

	// We should already be there.
	data->seek(128);
	for(uint32 c = 0; c < count; c++)
		{
		Flex::Reference f;
		f.offset = data->read4();
		f.size = data->read4();
#if DEBUGFLEX
		cout << "Item " << c << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		object_list.push_back(f);
		}
	}

/**
 *	Reads the desired object from the flex.
 *	@param objnum	Number of object to read.
 *	@param len	Receives the length of the object, or zero in any failure.
 *	@return	Buffer created with new[] containing the object data or
 *	null on any failure.
 */
char *Flex::retrieve(uint32 objnum, size_t &len)
	{
	if (!data || objnum >= object_list.size())
		{
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
 *	Obtains information about an object from the flex.
 *	@param objnum	Number of object.
 *	@param len	Receives the length of the object, or zero in any failure.
 *	@return	Offset of object from the beginning of the file.
 */
uint32 	Flex::get_entry_info(uint32 objnum, size_t &len)
	{
	if (!data || objnum >= object_list.size())
		{
		len = 0;
		return 0;
		}
#if 0
	// Trying to avoid exceptions.
	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in Flex::retrieve()");
#endif
	len = object_list[objnum].size;
	return object_list[objnum].offset;
	}

/**
 *	Write out a FLEX header.  Note that this is a STATIC method.
 *	@param out	DataSource to which the data is written.
 *	@param title	Flex file title.
 *	@param count	Number of entries in the flex.
 *	@param vers	Version of the flex file.
 */

void Flex::write_header
	(
	DataSource* out,			// File to write to.
	const char *title,
	size_t count,			// # entries.
	Flex_vers vers
	)
	{
	char titlebuf[0x50];		// Use savename for title.
	memset(titlebuf, 0, sizeof(titlebuf));
	strncpy(titlebuf, title, sizeof(titlebuf) - 1);
	out->write(titlebuf, sizeof(titlebuf));
	out->write4(0xFFFF1A00);	// Magic number.
	out->write4(static_cast<uint32>(count));
	if (vers == orig)		// 2nd magic #:  use for version.
		out->write4(0x000000CC);
	else
		out->write4(EXULT_FLEX_MAGIC2 + static_cast<int>(vers));
	long pos = out->getPos();		// Fill to data (past table at 0x80).
	long fill = 0x80 + 8*count - pos;
	while (fill--)
		out->write1(0);
	}

/**
 *	Verify if a file is a FLEX.  Note that this is a STATIC method.
 *	@param in	DataSource to verify.
 *	@return	Whether or not the DataSource is a FLEX file.
 */
bool Flex::is_flex(DataSource *in)
	{
	long pos = in->getPos();		// Fill to data (past table at 0x80).
	long len = in->getSize();	// Check length.
	uint32 magic = 0;
	if (len >= 0x80)		// Has to be at least this long.
		{
		in->seek(0x50);
		magic = in->read4();
		}
	in->seek(pos);

	// Is a flex
	if(magic==0xffff1a00UL) return true;

	// Isn't a flex
	return false;
	}


/**
 *	Verify if a file is a FLEX.  Note that this is a STATIC method.
 *	@param fname	Name of file to verify.
 *	@return	Whether or not the file is a FLEX file. Returns false if
 *	the file does not exist.
 */
bool Flex::is_flex(const char *fname)
	{
	if (!U7exists(fname))
		return false;

	std::ifstream in;
	U7open (in, fname);
	StreamDataSource ds(&in);

	if (in.good())
		return is_flex(&ds);

	in.close();
	return false;
	}

/**
 *	Start writing out a new Flex file.
 */
Flex_writer::Flex_writer
	(
	std::ofstream& o,			///< Where to write.
	const char *title,			///< Flex title.
	size_t cnt,				///< Number of entries we'll write.
	Flex::Flex_vers vers	///< Version of flex file.
	) : out(&o), dout(0), count(cnt), index(0)
	{
					// Write out header.
	StreamDataSource ds(out);
	Flex::write_header(&ds, title, count, vers);
					// Create table.
	tptr = table = new uint8[2*count*4];
	cur_start = out->tellp();	// Store start of 1st entry.
	}

/**
 *	Start writing out a new Flex file.
 */
Flex_writer::Flex_writer
	(
	DataSource *o,				///< Where to write.
	const char *title,			///< Flex title.
	size_t cnt,				///< Number of entries we'll write.
	Flex::Flex_vers vers	///< Version of flex file.
	) : out(0), dout(o), count(cnt), index(0)
	{
					// Write out header.
	Flex::write_header(dout, title, count, vers);
					// Create table.
	tptr = table = new uint8[2*count*4];
	cur_start = dout->getPos();	// Store start of 1st entry.
	}

/**
 *	Clean up.
 */

Flex_writer::~Flex_writer
	(
	)
	{
	close();
	}

/**
 *	Call this when done writing out a section.
 */

void Flex_writer::mark_section_done
	(
	)
	{
					// Location past end of section.
	size_t pos = out ? static_cast<size_t>(out->tellp()) : dout->getPos();
	Write4(tptr, static_cast<uint32>(cur_start));	// Store start of section.
	Write4(tptr, static_cast<uint32>(pos - cur_start));	// Store length.
	cur_start = pos;
	}

/**
 *	All done.
 *
 *	@return	False if error.
 */

bool Flex_writer::close
	(
	)
	{
	if (!table)
		return true;		// Already done.
	bool ok;
	if (out)
		{
		out->seekp(0x80, ios::beg);	// Write table.
		out->write(reinterpret_cast<char*>(table), 2*count*4);
		out->flush();
		ok = out->good();
		out->close();
		}
	else
		{
		dout->seek(0x80);		// Write table.
		dout->write(reinterpret_cast<char*>(table), 2*count*4);
		dout->flush();
		ok = dout->good();
		}
	delete [] table;
	table = 0;
	return ok;
	}

