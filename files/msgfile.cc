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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <vector>
#include "utils.h"

using std::istream;
using std::cerr;
using std::endl;
using std::vector;

/*
 *	Read in text, where each line is of the form "nnn:sssss", where nnn is
 *	to be the Flex entry #, and anything after the ':' is the string to
 *	store.  
 *	NOTES:	Entry #'s may be skipped, and may be given in hex (0xnnn)
 *			or decimal.
 *		Max. text length is 1024.
 *		A line beginning with a '#' is a comment.
 *	Output:	# of first message (i.e., lowest-numbered msg), or -1 if
 *		error.
 */

int Read_text_msg_file
	(
	istream& in,
	vector<char *>& strings		// Strings returned here, each
					//   allocated on heap.
	)
	{
	strings.resize(0);		// Initialize.
	strings.reserve(2000);
	char buf[1024];
	int linenum = 0;
	unsigned long first = 0xffffffff;// Index of first one found.
	while (!in.eof())
		{
		++linenum;
		in.get(buf, sizeof(buf));
		char delim;		// Check for end-of-line.
		in.get(delim);
		if (delim != '\n' && !in.eof())
			{
			cerr << "Line #" << linenum << " is too long" << endl;
			return -1;
			}
		if (!buf[0])
			continue;	// Empty line.
		char *ptr = &buf[0];
		char *endptr;		// Get line# in decimal, hex, or oct.
		long index = strtol(ptr, &endptr, 0);
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
		if (index >= strings.size())
			strings.resize(index + 1);
		strings[index] = newstrdup(endptr + 1);
		if (index < first)
			first = index;
		}
	return first;
	}

