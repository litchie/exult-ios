/*
 *	tiles.h - Tile coords.
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

#ifndef INCL_TILES
#define INCL_TILES	1

#include "exult_constants.h"

/*
 *	A 3D tile coordinate:
 */
class Tile_coord
	{
	static short neighbors[16];	// Neighboring tiles in each dir.
public:
	short tx, ty, tz;		// Coords. within world. tz=lift.
	Tile_coord(int x, int y, int z) : tx(x), ty(y), tz(z)
		{  }
	Tile_coord(): tx(0),ty(0),tz(0) { }
	int operator==(Tile_coord t2)
		{ return t2.tx == tx && t2.ty == ty && t2.tz == tz; }
	int operator!=(Tile_coord t2)
		{ return !(*this == t2); }
	int distance(Tile_coord t2)	// Distance to another tile?
		{			// Handle wrapping round the world.
		int dy = (t2.ty - ty + c_num_tiles)%c_num_tiles;
		int dx = (t2.tx - tx + c_num_tiles)%c_num_tiles;
		if (dy >= c_num_tiles/2)// World-wrapping.
			dy = c_num_tiles - dy;
		if (dx >= c_num_tiles/2)
			dx = c_num_tiles - dx;
					// Take larger abs. value.
		return (dy > dx ? dy : dx);
		}
					// Get neighbor in given dir (0-7).
	inline Tile_coord get_neighbor(int dir)
		{ return Tile_coord(
			(tx + neighbors[2*dir] + c_num_tiles)%c_num_tiles,
			(ty + neighbors[2*dir + 1] + c_num_tiles)%c_num_tiles,
								 tz); }
	static bool gte(int t1, int t2)	// Ret. t1 >= t2 with wrapping.
		{
		int diff = t1 - t2;
		return diff >= 0 ? (diff < c_num_tiles/2) :
						diff < -c_num_tiles/2;
		}
					// Ret. (to - from) with wrapping.
	static int delta(int from, int to)
		{
		int diff = to - from;
		return diff >= c_num_tiles/2 ? (diff - c_num_tiles) :
			(diff <= -c_num_tiles/2 ? (diff + c_num_tiles) :
								diff);
		}
	};
					// Add two coords.
inline Tile_coord operator+(Tile_coord a, Tile_coord b)
	{ return Tile_coord(a.tx + b.tx, a.ty + b.ty, a.tz + b.tz); }

#endif
