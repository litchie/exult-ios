/**	-*-tab-width: 8; -*-
 **
 **	Tiles.h - Tile coords.
 **
 **	Written: 5/5/2000 - JSF
 **/

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

#ifndef INCL_TILES
#define INCL_TILES	1

/*
 *	A 3D tile coordinate:
 */
class Tile_coord
	{
public:
	short tx, ty, tz;		// Coords. within world. tz=lift.
	Tile_coord(int x, int y, int z) : tx(x), ty(y), tz(z)
		{  }
	Tile_coord() { }
	int operator==(Tile_coord t2)
		{ return t2.tx == tx && t2.ty == ty && t2.tz == tz; }
	int distance(Tile_coord t2)	// Distance to another tile?
		{
		int dy = t2.ty - ty;
		int dx = t2.tx - tx;
		if (dy < 0)		// Just take longer abs. value.
			dy = -dy;
		if (dx < 0)
			dx = -dx;
		return (dy > dx ? dy : dx);
		}
	};
					// Add two coords.
inline Tile_coord operator+(Tile_coord a, Tile_coord b)
	{ return Tile_coord(a.tx + b.tx, a.ty + b.ty, a.tz + b.tz); }

#endif
