/**
 **	Shapevga.h - Handle the 'shapes.vga' file and associated info.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifndef INCL_SHAPEVGA
#define INCL_SHAPEVGA	1

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

#include <fstream>
#include <iostream>
#ifdef MACOS
  #include <cassert>
#endif
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
public:
	Shapes_vga_file() : info()
		{  }
	void init() { load(SHAPES_VGA); info.set_size(num_shapes); }
	virtual ~Shapes_vga_file();
	void read_info();		// Read additional data files.
	Shape_info& get_info(int shapenum)
	{ return shapenum>=1024&&shapenum%2?info[989]:
		 shapenum>=1024?info[721]:info[shapenum]; }
	};

#endif
