/**
 **	Citerate.h - Chunk iterator.
 **
 **	Written: 7/13/2000 - JSF
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

#ifndef INCL_CITERATE
#define INCL_CITERATE	1

/*
 *	Here's an iterator that takes a rectangle of tiles, and sequentially
 *	returns the interesection of that rectangle with each chunk that it
 *	touches.
 */
class Chunk_intersect_iterator
	{
	Rectangle tiles;		// Original rect.
					// Chunk #'s covered:
	int startcx, stopcx, stopcy;
	int curcx, curcy;		// Next chunk to return.
public:
	Chunk_intersect_iterator(Rectangle t) : tiles(t),
		  startcx(t.x/c_tiles_per_chunk),
		  stopcx(INCR_CHUNK((t.x + t.w - 1)/c_tiles_per_chunk)),
		  stopcy(INCR_CHUNK((t.y + t.h - 1)/c_tiles_per_chunk)),
		  curcy(t.y/c_tiles_per_chunk)
		{
		curcx = startcx;
		if (t.x < 0 || t.y < 0)
			{		// Empty to begin with.
			curcx = stopcx;
			curcy = stopcy;
			}
		}
					// Intersect is ranged within chunk.
	int get_next(Rectangle& intersect, int& cx, int& cy)
		{
		if (curcx == stopcx)	// End of row?
			if (curcy == stopcy)
				return (0);
			else
				{
				curcy = INCR_CHUNK(curcy);
				curcx = startcx;
				}
		Rectangle tmp = tiles;	// Shift area to chunk pos.
		tmp.shift(-curcx*c_tiles_per_chunk, -curcy*c_tiles_per_chunk);
		Rectangle cr(0, 0, c_tiles_per_chunk, c_tiles_per_chunk);
					// Intersect given rect. with chunk.
		intersect = cr.intersect(tmp);
		cx = curcx;
		cy = curcy;
		curcx = INCR_CHUNK(curcx);
		return (1);
		}
	};

#endif
