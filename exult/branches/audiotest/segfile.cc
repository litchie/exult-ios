/*
 *	segfile.cc - Handle access to to a data file consisting of segments.
 *
 *  Copyright (C) 2000-2001  The Exult Team
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "exceptions.h"
#include "segfile.h"
#include "utils.h"

/*
 *	Open file.
 */

Segment_file::Segment_file
	(
	std::string nm			// Path to file.
	) : filename(nm), num_segments(0)
{
	U7open(file, filename.c_str());
	file.seekg(0x54);		// Get # of segments.
	num_segments = Read4(file);
	if( !file.good() )
		throw file_read_exception(filename);
}

/*
 *	Read in a given segment.
 *
 *	returns the data in a new allocated buffer so it should be freed by the caller.
 */

char*	Segment_file::retrieve
	(
	uint32 index,			// Number desired.
	std::size_t& len
	)
{
	char *buffer;
	if (index >= num_segments)
		throw exult_exception("objnum too large in read_object()");

	file.seekg(0x80 + 8*index);	// Get to info.
	long offset = Read4(file);	// Get offset, length.
	len = Read4(file);
	if (!len)
		throw file_read_exception(filename);

	file.seekg(offset);		// Get to data.
	buffer = new char[len];		// Allocate buffer.
	file.read(buffer, len);		// Read it.
	if( !file.good() )
		throw file_read_exception(filename);

	return buffer;
}

