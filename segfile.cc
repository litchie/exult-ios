/**
 **	Handle access to to a data file consisting of segments.
 **
 **	Written: 2/25/00 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "segfile.h"
#include "utils.h"

/*
 *	Open file.
 */

Segment_file::Segment_file
	(
	char *nm			// Path to file.
	) : num_segments(0)
	{
	if (!U7open(file, nm))
		return;
	file.seekg(0x54);		// Get # of segments.
	num_segments = Read4(file);
	}

/*
 *	Read in a given segment.
 *
 *	Output:	1 if okay, with data containing what was read, len = length.
 *		0 if error.
 *	Note:	'data' is allocated, so it should be freed by the caller.
 */

int Segment_file::read_segment
	(
	int index,			// Number desired.
	char *&data, 			// ->data returned.
	int& len
	)
	{
	data = 0;
	len = 0;
	if (index >= num_segments)
		return (0);		// Past end.
	file.seekg(0x80 + 8*index);	// Get to info.
	long offset = Read4(file);	// Get offset, length.
	len = Read4(file);
	if (!len)
		return (0);
	file.seekg(offset);		// Get to data.
	data = new char[len];		// Allocate buffer.
	file.read(data, len);		// Read it.
	return (1);
	}

