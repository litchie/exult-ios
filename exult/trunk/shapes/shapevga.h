/*
 *	shapevga.h - Handle the 'shapes.vga' file and associated info.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef INCL_SHAPEVGA
#define INCL_SHAPEVGA	1

#include <fstream>
#include <iostream>
#ifdef MACOS
  #include <cassert>
#endif
#include "autoarray.h"
#include "fnames.h"
#include "imagebuf.h"
#include "vgafile.h"
#include "shapeinf.h"

/*
 *	The "shapes.vga" file:
 */
class Shapes_vga_file : public Vga_file
{
	autoarray<Shape_info> info;	// Extra info. about each shape.
	Shape_info zinfo;		// A fake one (all 0's).
	bool info_read;			// True when info is set.
public:
	Shapes_vga_file() : info(), info_read(false) {  }
	Shapes_vga_file(const char *nm, int u7drag = -1, const char *nm2 = 0);
	void init();
	virtual ~Shapes_vga_file();
					// Read additional data files.
	void read_info(bool bg, bool editing = false);
	void write_info(bool bg);	// Write them back out.
	virtual Shape *new_shape(int shapenum);	
	Shape_info& get_info(int shapenum)
	{
#if 0	/* This can go away.  Done in Shape_manager::read_shape_info().	*/
		// Shapes 1024 -> 1035 in SI are alternative player chars.
		// Odd are female, even are male.
		return shapenum>=1024&&shapenum<=1035&&shapenum%2 ? info[989]:
			shapenum>=1024&&shapenum<=1035 ? info[721]:
			info[shapenum];
#else
		return info[shapenum];
#endif
	}
};

#endif
