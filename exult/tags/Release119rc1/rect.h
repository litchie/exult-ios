/*
 *	rect.h - Rectangles.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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

#ifndef RECT_H
#define RECT_H	1

#ifdef WIN32
#include <windows.h>
#define Rectangle RECTX
#endif

/*
 *	A rectangle:
 */
class Rectangle
	{
public:					// Let's make it all public.
	int x, y;			// Position.
	int w, h;			// Dimensions.
	Rectangle(int xin, int yin, int win, int hin)
			: x(xin), y(yin), w(win), h(hin)
		{  }
	Rectangle() { }			// An uninitialized one.
					// Is this point in it?
	int has_point(int px, int py) const
		{ return (px >= x && px < x + w && py >= y && py < y + h); }
					// Add another to this one to get
					//  a rect. that encloses both.
	Rectangle add(Rectangle& r2) const
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x < r2.x ? x : r2.x;
		r.y = y < r2.y ? y : r2.y;
		r.w = (xend > xend2 ? xend : xend2) - r.x;
		r.h = (yend > yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Intersect another with this.
	Rectangle intersect(Rectangle& r2) const
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x >= r2.x ? x : r2.x;
		r.y = y >= r2.y ? y : r2.y;
		r.w = (xend <= xend2 ? xend : xend2) - r.x;
		r.h = (yend <= yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Does it intersect another?
	int intersects(Rectangle r2) const
		{
		return (x >= r2.x + r2.w ? 0 : r2.x >= x + w ? 0 :
			y >= r2.y + r2.h ? 0 : r2.y >= y + h ? 0 : 1);
		}
	void shift(int deltax, int deltay)
		{
		x += deltax;
		y += deltay;
		}		
	Rectangle& enlarge(int delta)	// Add delta in each dir.
		{
		x -= delta; y -= delta; w += 2*delta; h += 2*delta; 
		return *this;
		}
	int distance(int px, int py)	// Get distance from a point (max.
					//   dist. along x or y coord.)
		{
		int xdist = px <= x ? (x - px) : (px - x - w + 1);
		int ydist = py <= y ? (y - py) : (py - y - h + 1);
		int dist = xdist > ydist ? xdist : ydist;
		return dist < 0 ? 0 : dist;
		}
	};

#endif
