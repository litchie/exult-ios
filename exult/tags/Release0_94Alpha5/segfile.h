/*
 *	segfile.h - Handle access to to a data file consisting of segments.
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

#ifndef _SEGFILE_H_
#define _SEGFILE_H_

#include <fstream>
#include <string>

#include "exult_types.h"

/*
 *	A class for accessing any 'segmented' file:
 */
class Segment_file
{
	std::string		filename;
	std::ifstream	file;			// For reading.
	uint32			num_segments;
public:
	Segment_file(std::string nm);
	int get_num_segments()
		{ return num_segments; }
	int is_good()
		{ return (num_segments != 0); }
					// Return allocated buffer with data.
	char* retrieve(uint32 index, std::size_t& len);
};

#endif
