/*
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

#ifndef SHAPEID_H
#define SHAPEID_H	1

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */

class Shape_frame;

enum ShapeFile {
	SF_SHAPES_VGA,		// <STATIC>/shapes.vga
	SF_GUMPS_VGA,		// <STATIC>/gumps.vga
	SF_PAPERDOL_VGA,	// <STATIC>/paperdol.vga
	SF_SPRITES_VGA,		// <STATIC>/sprites.vga
	SF_FACES_VGA,		// <STATIC>/faces.vga
	SF_EXULT_FLX,		// <DATA>/exult.flx
	SF_GAME_FLX,		// <DATA>/bg_data.flx or <DATA>/si_data.flx
	SF_BG_SIGUMP_FLX,	// BG only for Paperdolls
	// Not yet
	//SF_FONTS_VGA,		// <STATIC>/fonts.vga

	SF_OTHER		// Other unknown FLX
};

class ShapeID
	{
	short shapenum;			// Shape #.
	unsigned char framenum;		// Frame # within shape.
	unsigned char has_trans;
	ShapeFile shapefile;
	Shape_frame *shape;

	Shape_frame* cache_shape();

public:
					// Create from map data.
	ShapeID(unsigned char l, unsigned char h) 
		: shapenum(l + 256*(h&0x3)), framenum(h >> 2), has_trans(false),
			shapefile(SF_SHAPES_VGA), shape(0)
		{  }
					// Read from buffer & incr. ptr.
	ShapeID(unsigned char *& data)
		: has_trans(false), shapefile(SF_SHAPES_VGA), shape(0)
		{
		unsigned char l = *data++;
		unsigned char h = *data++;
		shapenum = l + 256*(h&0x3);
		framenum = h >> 2;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : shapenum(-1), has_trans(false), shapefile(SF_SHAPES_VGA), shape(0)
		{  }

					// End-of-list or invalid?
	int is_invalid() const
		{ return shapenum == -1; }
	int is_eol() const
		{ return is_invalid(); }

	inline int get_shapenum() const
		{ return shapenum; }
	inline int get_framenum() const
		{ return framenum; }
	inline ShapeFile get_shapefile() const
		{ return shapefile; }
	inline Shape_frame *get_shape()
		{ return (shape!=0)?shape:cache_shape(); }
	inline void set_translucent(int trans)
		{ has_trans=trans; }
	inline bool is_translucent()
		{ if (shape==0) cache_shape(); return has_trans!=0; }
					// Set to given shape.
	void set_shape(int shnum, int frnum)
		{
		shapenum = shnum;
		framenum = frnum;
		shape = 0;
		}
	ShapeID(int shnum, int frnum, ShapeFile shfile = SF_SHAPES_VGA) :
		shapenum(shnum), framenum(frnum), shapefile(shfile), shape(0)
		{  }

	void set_shape(int shnum)	// Set shape, but keep old frame #.
		{ shapenum = shnum; shape = 0; }
	void set_frame(int frnum)	// Set to new frame.
		{ framenum = frnum; shape = 0; }
	void set_file(ShapeFile shfile)	// Set to new flex
		{ shapefile = shfile; shape = 0; }


	 
	};

#endif
