/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Paths.h - Various pathfinding clients.
 **
 **	Written: 6/12/2000 - JSF
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

#ifndef INCL_PATHS
#define INCL_PATHS	1

#include "PathFinder.h"
#include "tiles.h"

/*
 *	This class provides A* cost methods.
 */
class Actor_pathfinder_client : public Pathfinder_client
	{
public:
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
	};

/*
 *	This client succeeds when the path makes it to just one X/Y coord.
 *	It assumes that a -1 was placed in the coord. that we should ignore.
 */
class Onecoord_pathfinder_client : public Actor_pathfinder_client
	{
public:
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	};

/*
 *	This client is supposed to fail quickly, so that it can be used to
 *	test for when an object can be grabbed.
 */
class Fast_pathfinder_client : public Pathfinder_client
	{
	int dist;			// Succeeds at this distance from goal.
public:
	Fast_pathfinder_client(int d = 0) : dist(0)
		{  }
					// Figure when to give up.
	virtual int get_max_cost(int cost_to_goal);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	static int is_grabable(Tile_coord from, Tile_coord to);
	};

#endif	/* INCL_PATHS */

