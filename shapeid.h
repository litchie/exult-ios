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

#include "shapevga.h"

class Shape_frame;
class Shape_info;

enum ShapeFile {
	SF_SHAPES_VGA = 0,	// <STATIC>/shapes.vga.  MUST be first.
	SF_GUMPS_VGA,		// <STATIC>/gumps.vga
	SF_PAPERDOL_VGA,	// <STATIC>/paperdol.vga
	SF_SPRITES_VGA,		// <STATIC>/sprites.vga
	SF_FACES_VGA,		// <STATIC>/faces.vga
	SF_EXULT_FLX,		// <DATA>/exult.flx
	SF_GAME_FLX,		// <DATA>/bg_data.flx or <DATA>/si_data.flx
	SF_BG_SIGUMP_FLX,	// BG only for Paperdolls
	SF_BG_SISHAPES_VGA,	// BG only for Multiracial
	// Not yet
	//SF_FONTS_VGA,		// <STATIC>/fonts.vga

	SF_OTHER,		// Other unknown FLX
	SF_COUNT		// # of preceding entries.
};

/*
 *	Manage the set of shape files.
 */
class Shape_manager
	{
	static Shape_manager *instance;	// There shall be only one.
	Shapes_vga_file shapes;		// Main 'shapes.vga' file.
	Vga_file files[(int) SF_COUNT];	// The files we manage.
	bool bg_paperdolls_allowed;	// Set true if the SI paperdoll file 
					//   is found when playing BG
	bool bg_paperdolls;		// True if paperdolls are wanted in BG
	bool bg_multiracial_allowed;	// Set true if the SI shapes file 
					//   is found when playing BG
public:
	friend class ShapeID;
	Shape_manager();
	~Shape_manager();
	static Shape_manager *get_instance()
		{ return instance; }
	void load();			// Read in files.
	Vga_file& get_file(enum ShapeFile f)
		{ return files[(int) f]; };
	Shapes_vga_file& get_shapes()
		{ return shapes; }
	Shape_info& get_info(int shnum)	// Get info. about shape.
		{ return shapes.get_info(shnum); }
	// BG Only
	inline bool can_use_paperdolls() const
	{ return bg_paperdolls_allowed; }

	inline bool get_bg_paperdolls() const
	{ return bg_paperdolls; }

	inline void set_bg_paperdolls(bool p)
	{ bg_paperdolls = p; }

	inline bool can_use_multiracial() const
	{ return bg_multiracial_allowed; }
	};

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID
	{
	short shapenum;			// Shape #.
	char framenum;			// Frame # within shape.
	char has_trans;
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

    virtual ~ShapeID()
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

	int get_num_frames() const;
	Shape_info& get_info() const;	// Get info. about shape.
	};

#endif
