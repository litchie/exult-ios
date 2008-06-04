/**
 **	bodyinf.h - Body information from 'bodies.txt'.
 **
 **	Written: 06/01/2008 - Marzo
 **/

#ifndef INCL_BODYINF_H
#define INCL_BODYINF_H	1

/*
Copyright (C) 2008 The Exult Team

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

#include "baseinf.h"
using std::istream;

class Shape_info;

/*
 *	Information about body shape and frame.
 */
class Body_info : public Base_info
	{
	int		bshape;			// Body shape.
	int		bframe;			// Body frame.
public:
	friend class Shape_info;
	Body_info()
		: Base_info()
		{  }
		// Read in from file.
	int read(std::istream& in, int index, int version, bool bg);
					// Write out.
	void write(std::ostream& out, int shapenum, bool bg);
	void set(int s, int f)
		{
		if (bshape != s || bframe != f)
			{
			set_modified(true);
			bshape = s;
			bframe = f;
			}
		}
	int get_body_shape() const
		{ return bshape; }
	int get_body_frame() const
		{ return bframe; }
	static int get_info_flag()
		{ return 0x100; }
	static const int get_entry_size()
		{ return -1; }
	};

#endif
