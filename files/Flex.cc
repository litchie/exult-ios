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

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#define	DEBUGFLEX 0

#include "Flex.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <fstream>
#include <iostream>
#include "exceptions.h"
#include "utils.h"

using std::cerr;
using std::endl;
using std::FILE;
using std::memset;
using std::size_t;
using std::string;
using std::strncpy;


Flex::Flex(const string &n) : U7file(n)
{
	IndexFlexFile();
}

void	Flex::IndexFlexFile(void)
{
	FILE	*fp;
	fp=U7open(filename.c_str(),"rb");
	std::fread(title,sizeof(title),1,fp);
	magic1 = Read4(fp);
	count = Read4(fp);
	magic2 = Read4(fp);
	if(magic1!=0xffff1a00UL)
		throw wrong_file_type_exception(filename,"FLEX");	// Not a flex file

	for(int i=0;i<9;i++)
		padding[i] = Read4(fp);
#if DEBUGFLEX
	cout << "Title: " << title << endl;
	cout << "Count: " << count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(uint32 c=0;c<count;c++)
	{
		Flex::Reference f;
		f.offset = Read4(fp);
		f.size = Read4(fp);
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

/*
 *	Write out a FLEX header.  Note that this is a STATIC method.
 */

void Flex::write_header
	(
	std::ostream& out,			// File to write to.
	const char *title,
	int count			// # entries.
	)
	{
	char titlebuf[0x50];		// Use savename for title.
	memset(titlebuf, 0, sizeof(titlebuf));
	strncpy(titlebuf, title, sizeof(titlebuf) - 1);
	out.write(titlebuf, sizeof(titlebuf));
	Write4(out, 0xFFFF1A00);	// Magic number.
	Write4(out, count);
	Write4(out, 0x000000CC);	// 2nd magic number.
	long pos = out.tellp();		// Fill to data (past table at 0x80).
	long fill = 0x80 + 8*count - pos;
	while (fill--)
		out.put((char) 0);
	}

/*
 *	Verify if a file is a FLEX.  Note that this is a STATIC method.
 */

bool Flex::is_flex(std::istream& in)
{
	long pos = in.tellg();		// Fill to data (past table at 0x80).
	in.seekg(0x50);
	uint32 magic = Read4(in);
	in.seekg(pos);

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

	if (in.good()) is = is_flex(in);

	in.close();
	return is;
}

/*
 *	Start writing out a new Flex file.
 */

Flex_writer::Flex_writer
	(
	ofstream& o,			// Where to write.
	const char *title,		// Flex title.
	int cnt				// #entries we'll write.
	) : out(&o), count(cnt), index(0)
	{
					// Write out header.
	Flex::write_header(*out, title, count);
					// Create table.
	tptr = table = new uint8[2*count*4];
	cur_start = out->tellp();	// Store start of 1st entry.
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
	long pos = out->tellp();	// Location past end of section.
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
	out->seekp(0x80, ios::beg);	// Write table.
	out->write(reinterpret_cast<char*>(table), 2*count*4);
	out->flush();
	bool ok = out->good();
	out->close();
	delete table;
	table = 0;
	return ok;
	}

#endif // PENTAGRAM