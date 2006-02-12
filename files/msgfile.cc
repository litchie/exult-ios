/**
 **	Msgfile.cc - Read in text message file.
 **
 **	Written: 6/25/03
 **/

/*
 *  Copyright (C) 2002-2003  The Exult Team
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

#include <iostream>
#include <vector>
#include <ctype.h>
#include <iomanip>
#include "utils.h"
#include "databuf.h"

using std::istream;
using std::ostream;
using std::cerr;
using std::endl;
using std::hex;
using std::vector;

/*
 *	Read in text, where each line is of the form "nnn:sssss", where nnn is
 *	to be the Flex entry #, and anything after the ':' is the string to
 *	store.  
 *	NOTES:	Entry #'s may be skipped, and may be given in hex (0xnnn)
 *			or decimal.
 *		Max. text length is 1024.
 *		A line beginning with a '#' is a comment.
 *		A 'section' can be marked:
 *			%%section shapes
 *				....
 *			%%endsection
 *	Output:	# of first message (i.e., lowest-numbered msg), or -1 if
 *		error.
 */

int Read_text_msg_file(DataSource* in, vector<char *>& strings, char* section)
{
	strings.resize(0);		// Initialize.
	strings.reserve(1000);
	const char* buf;
	int linenum = 0;
#define NONEFOUND 0xffffffff
	unsigned long first = NONEFOUND;// Index of first one found.
	long next_index = 0;// For auto-indexing of lines
	while (!in->eof())
	{
		++linenum;
		std::string s;
		in->readline(s);
		if (s.empty())
			continue;	// Empty line.
		unsigned int len = s.size();
		buf = s.c_str();

		const char *ptr = &buf[0];
		if (section)
		{
			if (buf[0] != '%' || 
					strncmp(ptr + 1, "%section", 8) != 0)
				continue;
			for (ptr = &buf[9]; isspace(*ptr); ++ptr)
				;
			if (strncmp(ptr, section, strlen(ptr)) == 0)
				{	// Found the section.
				section = 0;
				continue;
				}
			cerr << "Line #" << linenum << 
				" has the wrong section name: " << newstrdup(ptr) << " != " << section << endl;
			return -1;
		}
		if (buf[0] == '%' && strncmp(ptr + 1, "%endsection", 11) == 0)
			break;
		char *endptr;
		long index;
		
		if (buf[0] == ':')
		{			// Auto-index lines missing an index.
			index = next_index++;
			endptr = const_cast<char *>(&buf[0]);
		}
		else
		{			// Get line# in decimal, hex, or oct.
			index = strtol(ptr, &endptr, 0);
			if (endptr == ptr)	// No #?
			{
				if (*ptr == '#')
					continue;
				cerr << "Line " << linenum <<
						" doesn't start with a number" << endl;
				return -1;
			}
			if (*endptr != ':')
			{
				cerr << "Missing ':' in line " << linenum << 
					".  Ignoring line" << endl;
				continue;
			}
		}
		if (index >= strings.size())
			strings.resize(index + 1);
		strings[index] = newstrdup(endptr + 1);
		if (index < first)
			first = index;
	}
	return first == NONEFOUND ? -1 : (int) first;
}


int Read_text_msg_file
	(
	istream& in,
	vector<char *>& strings,	// Strings returned here, each
					//   allocated on heap.
	char *section			// Section name, or NULL.  If given
					//   the section must be next infile.
	)
{
	StreamDataSource ds(&in);
	return Read_text_msg_file(&ds, strings, section);
}

/*
 *	Same as above, but store in given list, and set given count.
 */

int Read_text_msg_file
	(
	istream& in,
	char **& strings,		// Strings returned here, each
					//   allocated on heap.
	int& count,
	char *section
	)
	{
	StreamDataSource ds(&in);
	vector<char *> txtlist;
	int first = Read_text_msg_file(&ds, txtlist, section);
	count = txtlist.size();
	strings = new char *[count];
	for (int i = 0; i < count; ++i)
		strings[i] = txtlist[i];
	return first;
	}

/*
 *	Write one section.
 */

void Write_msg_file_section
	(
	ostream& out, 
	char *section, 
	char **items, 
	int num_items
	)
	{
	out << "%%section " << section << endl;
	for (int i = 0; i < num_items; ++i)
		if (items[i])
			out << hex << "0x" << i << ':' << items[i] << endl;
	out << "%%endsection " << section << endl;
	}

