/*
Copyright (C) 2000  Dancer A.L Vesperman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PENTAGRAM // DONT'T INCLUDE THIS IN PENTAGRAM!

#define	DEBUGFLEX 0

#include "Flex.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <fstream>
#include <iostream>
#include "exceptions.h"
#include "utils.h"
#include "databuf.h"

#ifndef UNDER_CE
using std::cerr;
using std::endl;
using std::FILE;
using std::memset;
using std::size_t;
using std::string;
using std::strncpy;
using std::ofstream;
using std::ios;
#endif

Flex::Flex(const string &n) : U7file(n)
{
	IndexFlexFile();
}

Flex::Flex_vers Flex::get_vers() const
{
	return (magic2&~0xff) == EXULT_FLEX_MAGIC2 ? 
		(Flex_vers) (magic2&0xff) : orig;
}

void	Flex::IndexFlexFile(void)
{
	FILE	*fp;
	fp=U7open(filename.c_str(),"rb");
	FileDataSource flex(fp);
	flex.read(title, sizeof(title));
	magic1 = flex.read4();
	count = flex.read4();
	magic2 = flex.read4();
	if(magic1!=0xffff1a00UL) {
		std::fclose (fp);
		throw wrong_file_type_exception(filename,"FLEX");	// Not a flex file
	}

	for(int i=0;i<9;i++)
		padding[i] = flex.read4();
#if DEBUGFLEX
	cout << "Title: " << title << endl;
	cout << "Count: " << count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(uint32 c=0;c<count;c++)
	{
		Flex::Reference f;
		f.offset = flex.read4();
		f.size = flex.read4();
#if DEBUGFLEX
		cout << "Item " << c << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		object_list.push_back(f);
	}
	fclose(fp);
}

char *	Flex::retrieve(uint32 objnum, size_t &len)
{
	FILE	*fp;
	char	*buffer;

	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in Flex::retrieve()");

	fp = U7open(filename.c_str(), "rb");
	fseek(fp, object_list[objnum].offset, SEEK_SET);
	len = object_list[objnum].size;
	buffer = new char[len];
	std::fread(buffer, len, 1, fp);
	fclose(fp);
	
	return buffer;
}

uint32 	Flex::get_entry_info(uint32 objnum, size_t &len)
{
	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in Flex::retrieve()");
	len = object_list[objnum].size;
	return object_list[objnum].offset;
}

/*
 *	Write out a FLEX header.  Note that this is a STATIC method.
 */

void Flex::write_header
	(
	DataSource* out,			// File to write to.
	const char *title,
	int count,			// # entries.
	Flex_vers vers
	)
	{
	char titlebuf[0x50];		// Use savename for title.
	memset(titlebuf, 0, sizeof(titlebuf));
	strncpy(titlebuf, title, sizeof(titlebuf) - 1);
	out->write(titlebuf, sizeof(titlebuf));
	out->write4(0xFFFF1A00);	// Magic number.
	out->write4(count);
	if (vers == orig)		// 2nd magic #:  use for version.
		out->write4(0x000000CC);
	else
		out->write4(EXULT_FLEX_MAGIC2 + (int) vers);
	long pos = out->getPos();		// Fill to data (past table at 0x80).
	long fill = 0x80 + 8*count - pos;
	while (fill--)
		out->write1((char) 0);
	}

/*
 *	Verify if a file is a FLEX.  Note that this is a STATIC method.
 */

bool Flex::is_flex(DataSource *in)
{
	long pos = in->getPos();		// Fill to data (past table at 0x80).
	long len = in->getSize() - pos;	// Check length.
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

bool Flex::is_flex(const char *fname)
{
	bool is = false;
	std::ifstream in;
	U7open (in, fname);
	StreamDataSource ds(&in);

	if (in.good()) is = is_flex(&ds);

	in.close();
	return is;
}

/*
 *	Start writing out a new Flex file.
 */

Flex_writer::Flex_writer
	(
	std::ofstream& o,			// Where to write.
	const char *title,			// Flex title.
	int cnt,				// #entries we'll write.
	Flex::Flex_vers vers
	) : out(&o), dout(0), count(cnt), index(0)
	{
					// Write out header.
	StreamDataSource ds(out);
	Flex::write_header(&ds, title, count, vers);
					// Create table.
	tptr = table = new uint8[2*count*4];
	cur_start = out->tellp();	// Store start of 1st entry.
	}

Flex_writer::Flex_writer
	(
	DataSource *o,				// Where to write.
	const char *title,			// Flex title.
	int cnt,				// #entries we'll write.
	Flex::Flex_vers vers
	) : out(0), dout(o), count(cnt), index(0)
	{
					// Write out header.
	Flex::write_header(dout, title, count, vers);
					// Create table.
	tptr = table = new uint8[2*count*4];
	cur_start = dout->getPos();	// Store start of 1st entry.
	}

/*
 *	Clean up.
 */

Flex_writer::~Flex_writer
	(
	)
	{
	close();
	}

/*
 *	Call this when done writing out a section.
 */

void Flex_writer::mark_section_done
	(
	)
	{
					// Location past end of section.
	long pos = out ? (long) out->tellp() : (long) dout->getPos();
	Write4(tptr, cur_start);	// Store start of section.
	Write4(tptr, pos - cur_start);	// Store length.
	cur_start = pos;
	}

/*
 *	All done.
 *
 *	Output:	False if error.
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
	delete table;
	table = 0;
	return ok;
	}

#endif // PENTAGRAM
