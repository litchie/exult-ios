/**
 **	Ordinfo.h - Ordering information.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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

#ifndef INCL_ORDINFO
#define INCL_ORDINFO	1

/*
 *	Information about an object used during render-order comparison (lt()):
 */
class Ordering_info
	{
public:
	Rectangle area;			// Area (pixels) rel. to screen.
	Shape_info& info;		// Info. about shape.
	int tx, ty, tz;			// Absolute tile coords.
	int xs, ys, zs;			// Tile dimensions.
#if 1	/* For experimental compare(). */
	int xleft, xright, ynear, yfar, zbot, ztop;
#endif
private:
	void init(const Game_object *obj)
		{
		Tile_coord t = obj->get_tile();
		tx = t.tx; ty = t.ty; tz = t.tz;
		int frnum = obj->get_framenum();
		xs = info.get_3d_xtiles(frnum);
		ys = info.get_3d_ytiles(frnum);
		zs = info.get_3d_height();
#if 1	/* For experimental compare(). */
		xleft = tx - xs + 1;
		xright = tx;
		yfar = ty - ys + 1;
		ynear = ty;
		ztop = tz + zs - 1;
		zbot = tz;
		if (!zs)		// Flat?
			zbot--;
#endif
		}
public:
	friend class Game_object;
	friend class Map_chunk;
					// Create from scratch.
	Ordering_info(Game_window *gwin, Game_object *obj)
		: area(gwin->get_shape_rect(obj)),
		  info(gwin->get_shapes().get_info(obj->get_shapenum()))
		{ init(obj); }
	Ordering_info(Game_window *gwin, Game_object *obj, Rectangle& a)
		: area(a),
		  info(gwin->get_shapes().get_info(obj->get_shapenum()))
		{ init(obj); }
	};

#endif
