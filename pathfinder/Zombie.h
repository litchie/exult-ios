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

#ifndef	ZOMBIE_H
#define	ZOMBIE_H


#include "PathFinder.h"


class	Zombie: public virtual PathFinder
	{
	int major_distance;		// Distance in tiles to go.
	int major_frame_incr;		// # steps to take in faster dir.
	Tile_coord cur;			// Current pos. within world.
					// ->'s to cur.tx, cur.ty.
	short *major_coord, *minor_coord;
	int major_dir, minor_dir;	// 1 or -1 for dir. along each axis.
	int major_delta, minor_delta;	// For each tile we move along major
					//   axis, we add 'minor_delta'.  When
					//   the sum >= 'major_delta', we move
					//   1 tile along minor axis, and
					//   subtract 'major_delta' from sum.
	int sum;			// Sum of 'minor_delta''s.
public:
	Zombie(int major_incr = 1) : major_distance(0), 
					major_frame_incr(major_incr)
		{  }
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord s, Tile_coord d,
					Pathfinder_client *client);

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord& n);
	virtual int get_num_steps()	// # of steps left to take.
		{ return major_distance/major_frame_incr; }
	virtual ~Zombie();
	};

#endif
