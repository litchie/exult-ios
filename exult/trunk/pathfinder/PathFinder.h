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

#ifndef	PATHFINDER_H
#define	PATHFINDER_H

#include "tiles.h"

/*
 *	This class provides A* cost methods.
 */
class Pathfinder_client
	{
private:
	int		move_flags;
public:
					// Figure when to give up.
	virtual int get_max_cost(int cost_to_goal);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to) = 0;
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to) = 0;
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);

	int get_move_flags() { return move_flags; }
	void set_move_flags(int m) { move_flags = m;}
	};

/*
 *	Base class for all PathFinders.
 */
class	PathFinder
	{
protected:
	Tile_coord src;			// Source tile.
	Tile_coord dest;		// Destination.
public:
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	PathFinder() : src(),dest()
		{  }
	virtual	int NewPath(Tile_coord s, Tile_coord d, 
					Pathfinder_client *client)=0;
	// Retrieve starting point (set by subclasses).
	Tile_coord get_src()
		{ return src; }
	// Retrieve current destination (set by subclasses).
	Tile_coord get_dest()
		{ return dest; }
	// Retrieve the coordinates of the next step on the path
	virtual	int GetNextStep(Tile_coord& n, bool& done)=0;
	int GetNextStep(Tile_coord& n)
		{			// If you don't care about last step.
		bool d;
		return GetNextStep(n, d);
		}
	// Set to retrieve in opposite order.
	virtual int set_backwards()
		{ return 0; }		// Default: Can't do it.
	virtual int following_smart_path()	// Astar?
		{ return 0; }
	virtual int get_num_steps() = 0;// # of steps left to take.
	virtual ~PathFinder();
	};

#endif
