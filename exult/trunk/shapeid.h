/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.h - Game objects.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef INCL_SHAPEID
#define INCL_SHAPEID	1

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID
	{
	short shapenum;			// Shape #.
	unsigned char framenum;		// Frame # within shape.
public:
					// Create from map data.
	ShapeID(unsigned char l, unsigned char h) 
		: shapenum(l + 256*(h&0x3)), framenum(h >> 2)
		{  }
	ShapeID(unsigned char *& data)	// Read from buffer & incr. ptr.
		{
		unsigned char l = *data++;
		unsigned char h = *data++;
		shapenum = l + 256*(h&0x3);
		framenum = h >> 2;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : shapenum(-1)
		{  }
	~ShapeID() {};
	int is_invalid() const		// End-of-list or invalid?
		{ return shapenum == -1; }
	int is_eol() const
		{ return is_invalid(); }
	int get_shapenum() const
		{ return shapenum; }
	int get_framenum() const
		{ return framenum; }
					// Set to given shape.
	void set_shape(int shnum, int frnum)
		{
		shapenum = shnum;
		framenum = frnum;
		}
	ShapeID(int shnum, int frnum) : shapenum(shnum), framenum(frnum)
		{  }
	void set_shape(int shnum)	// Set shape, but keep old frame #.
		{ shapenum = shnum; }
	void set_frame(int frnum)	// Set to new frame.
		{ framenum = frnum; }
	};

#endif
