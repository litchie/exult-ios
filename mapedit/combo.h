/**
 **	Combo.h - A combination of multiple objects.
 **
 **	Written: 4/26/02 - JSF
 **/

#ifndef INCL_COMBO_H
#define INCL_COMBO_H	1

/*
Copyright (C) 2002 The Exult Team

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

#include <vector>
#include "objbrowse.h"

class Shape_draw;

/*
 *	A single object:
 */
class Combo_member
	{
	int tx, ty, tz;			// Location (tile) rel. to top-left.
	int shapenum, framenum;		// Object in shapes.vga.
public:
	friend class Combo;
	Combo_member(int x, int y, int z, int sh, int fr)
		: tx(x), ty(y), tz(z), shapenum(sh), framenum(fr)
		{  }
	};

/*
 *	A combination of objects.
 */
class Combo
	{
	vector<Combo_member *> members;	// Members of this combination.
	int hot_index;			// Index of obj. whose 'hot spot' we'll
					//   use.
	int tx, ty;			// Offset represented by top-left.
	int xtiles, ytiles, ztiles;	// Dimensions.
public:
	Combo();
	~Combo();
					// Add a new object.
	void add(int tx, int ty, int tz, int shnum, int frnum);
	void remove(int i);		// Remove object #i.
	void draw(Shape_draw *draw);	// Paint shapes in drawing area.
	};

#endif	/* INCL_COMBO_H */
