/**
 **	Handle access to to a data file consisting of segments.
 **
 **	Written: 2/25/00 - JSF
 **/

#ifndef INCL_SEGFILE
#define INCL_SEGFILE	1

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

#include <fstream.h>

/*
 *	A class for accessing any 'segmented' file:
 */
class Segment_file
	{
	ifstream file;			// For reading.
	int num_segments;
public:
	Segment_file(char *nm);
	int get_num_segments()
		{ return num_segments; }
	int is_good()
		{ return (num_segments != 0); }
					// Return allocated buffer with data.
	int read_segment(int index, char *&data, int& len);
	};

#endif
