/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Path.cc - Pathfinding algorithm.
 **
 **	Written: 4/7/2000 - JSF
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

#include "objs.h"
#include "gamewin.h"

/*
 *	A node for our search:
 */
class Search_node
	{
	friend class Game_window;
	Tile_coord tile;		// The coords (x, y, z) in tiles.
	short cost_from_start;		// Actual cost from start.
	short cost_to_goal;		// Estimated cost to goal.
	Search_node *parent;		// Prev. in path.
	Search_node(Tile_coord& t, short scost, short gcost, Search_node *p)
		: tile(t), cost_from_start(scost), cost_to_goal(gcost),
		  parent(p)
		{  }
	void update(short scost, short gcost, Search_node *p)
		{
		cost_from_start = scost;
		cost_to_goal = gcost;
		parent = p;
		}
	};

/*
 *	Iterate through neighbors of a tile (in 2 dimensions).
 */
class Neighbor_iterator
	{
	Tile_coord tile;		// Original tile.
	static int coords[2][8];	// Coords to go through.
	int index;			// 0-7.
public:
	Neighbor_iterator(Tile_coord t) : tile(t), index(0)
		{  }
					// Get next neighbor.
	int operator()(Tile_coord& newt)
		{
		while (index < 8)
			{
			newt = Tile_coord(t.tx + coords[index][0],
					t.ty + coords[index][1], t.tz);
			index++;
			if (nt.tx >= 0 && nt.tx < num_tiles &&
			    nt.ty >= 0 && nt.ty < num_tiles)
				return (1);
			}
		return (0);
		}
	};

/*
 *	First cut at using the A* pathfinding algorithm.
 *
 *	Output:	->(allocated) array of Tile_coords to follow, or 0 if failed.
 */

Tile_coord *Game_window::find_path
	(
	Tile_coord start,		// Where to start from.
	Tile_coord goal			// Where to end up.
	)
	{
	Vector open(0, 100), closed(0, 100);
	int opencnt = 0;		// Actual entries in 'open'.
					// Create start node.
	open.append(new Search_node(start, 0, Cost_to_goal(start, goal), 0));
	opencnt++;
	while (opencnt)			// Main loop of algorithm.
		{
		Search_node *node = (Search_node *) open.get(--opencnt);
		if (node->tile == goal)
			{		// Success.
			//++++++create path.
			return;++++++
			}
					// Go through surrounding tiles.
		Neighbor_iterator next(node->tile);
		Tile_coord ntile;
		while (next(ntile))
			{		// Calc. cost from start.
			int new_cost = node->cost_from_start + 
							Cost(tile, ntile);
					// See if next tile already seen.
			int open_index, closed_index = -1;
			Search_node *next = Find(open, ntile, open_index);
			if (!next)
				next = Find(closed, ntile, closed_index);
					// Already there, and cheaper?
			if (next && next->cost_from_start < new_cost)
				continue;
			int new_cost_to_goal = Cost_to_goal(ntile, goal);
			if (!next)	// Create if necessary.
				next = new Search_node(ntile, new_cost,
						new_cost_to_goal, node);
			else
				next.update(new_cost, new_cost_to_goal, node);
					// Remove from closed.
			if (closed_index != -1)
				closed.put(closed_index, 0);
					// Not in open?  Push it.
			if (open_index == -1)
				open.put(opencnt++, next);
			}
		}
	return 0;			// Failed if here.
	}
