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

#ifndef	ASTAR_H
#define	ASTAR_H

#include "PathFinder.h"
#include <vector>


class	Astar: public virtual PathFinder
	{
	std::vector<Tile_coord> path;		// Coords. to goal.
	int pathlen;			// Length of path.
	int dir;			// 1 or -1.
	int stop;			// Index to stop at.
	int next_index;			// Index of next tile to return.
public:
	Astar() : PathFinder(),path(), pathlen(0), dir(0),stop(0),next_index(0)
		{  }
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord s, Tile_coord d,
						Pathfinder_client *client);

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord& n);
	// Set to retrieve in opposite order.
	virtual int set_backwards();
	virtual int following_smart_path()	// Astar?
		{ return 1; }
	virtual ~Astar();
	};

#endif
