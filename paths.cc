/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Paths.cc - Various pathfinding clients.
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

#include "paths.h"
#include "Astar.h"
#include "gamewin.h"

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Actor_pathfinder_client::get_step_cost
	(
	Tile_coord from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int cx = to.tx/tiles_per_chunk, cy = to.ty/tiles_per_chunk;
	Chunk_object_list *olist = gwin->get_objects(cx, cy);
	int tx = to.tx%tiles_per_chunk;	// Get tile within chunk.
	int ty = to.ty%tiles_per_chunk;
	int cost = 1;
	olist->setup_cache();		// Make sure cache is valid.
	int water, poison;		// Get tile info.
	Actor::get_tile_info(gwin, olist, tx, ty, water, poison);
	int new_lift;			// Might climb/descend.
					// For now, assume height=3.
	if (olist->is_blocked(3, to.tz, tx, ty, new_lift) ||
	    (water && new_lift == 0))
		{			// Blocked, but check for a door.
		Game_object *block = Game_object::find_blocking(to);
		if (!block)
			return -1;
		if (!block->is_closed_door())
			return -1;
		new_lift = to.tz;	// We can open doors.
		cost++;			// But try to avoid them.
		}
	if (new_lift != to.tz)
		{
		cost++;
		to.tz = new_lift;
		}
					// On the diagonal?
	if (from.tx != to.tx || from.ty != to.ty)
		cost *= 3;		// Make it 50% more expensive.
	else
		cost *= 2;
	if (poison && new_lift == 0)
		cost *= 2;		// And avoid poison if possible.
					// Get 'flat' shapenum.
	ShapeID flat = olist->get_flat(tx, ty);
	int shapenum = flat.get_shapenum();
	if (shapenum == 24)		// Cobblestone path in BlackGate?
		{
		int framenum = flat.get_framenum();
		if (framenum <= 1)
			cost--;
		}
	return (cost);
	}

/*
 *	Estimate cost from one point to another.
 */

int Actor_pathfinder_client::estimate_cost
	(
	Tile_coord& from,
	Tile_coord& to
	)
	{
	int dx = to.tx - from.tx;
	if (dx < 0)
		dx = -dx;
	int dy = to.ty - from.ty;
	if (dy < 0)
		dy = -dy;
	int larger, smaller;		// Start with larger.
	if (dy <= dx)
		{
		larger = dx;
		smaller = dy;
		}
	else
		{
		larger = dy;
		smaller = dx;
		}
	return (2*larger + smaller);	// Straight = 2, diag = 3.
	}

/*
 *	Estimate cost from one point to another.
 */

int Onecoord_pathfinder_client::estimate_cost
	(
	Tile_coord& from,
	Tile_coord& to			// Should be the goal.
	)
	{
	if (to.tx == -1)		// Just care about Y?
		{			// Cost = 2/tile.
		int dy = to.ty - from.ty;
		return (2*(dy < 0 ? -dy : dy));
		}
	else if (to.ty == -1)
		{
		int dx = to.tx - from.tx;
		return (2*(dx < 0 ? -dx : dx));
		}
	else				// Should get here.
		return Actor_pathfinder_client::estimate_cost(from, to);
	}

/*
 *	Is tile at goal?
 */

int Onecoord_pathfinder_client::at_goal
	(
	Tile_coord& tile,
	Tile_coord& goal
	)
	{
	return ((goal.tx == -1 || tile.tx == goal.tx) && 
		(goal.ty == -1 || tile.ty == goal.ty) &&
		(goal.tz == -1 || tile.tz == goal.tz));
	}

/*
 *	Figure when to give up.
 */

int Fast_pathfinder_client::get_max_cost
	(
	int cost_to_goal		// From estimate_cost().
	)
	{
	int max_cost = 2*cost_to_goal;	// Don't try too hard.
	if (max_cost < 8)
		max_cost = 8;
	else if (max_cost > 64)
		max_cost = 64;
	return max_cost;
	}

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Fast_pathfinder_client::get_step_cost
	(
	Tile_coord from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int cx = to.tx/tiles_per_chunk, cy = to.ty/tiles_per_chunk;
	Chunk_object_list *olist = gwin->get_objects(cx, cy);
	int tx = to.tx%tiles_per_chunk;	// Get tile within chunk.
	int ty = to.ty%tiles_per_chunk;
	olist->setup_cache();		// Make sure cache is valid.
	int new_lift;			// Might climb/descend.
					// For now, look at 1 tile's height.
	if (olist->is_blocked(1, to.tz, tx, ty, new_lift))
		return -1;
	else
		return 1;
	}

/*
 *	Estimate cost from one point to another.
 */

int Fast_pathfinder_client::estimate_cost
	(
	Tile_coord& from,
	Tile_coord& to
	)
	{
	return from.distance(to);
	}

/*
 *	Is tile at goal?
 */

int Fast_pathfinder_client::at_goal
	(
	Tile_coord& tile,
	Tile_coord& goal
	)
	{
	return tile.distance(goal) <= dist;
	}

/*
 *	Just test to see if an object can be grabbed.
 */

int Fast_pathfinder_client::is_grabable
	(
	Tile_coord from,		// From this spot.
	Tile_coord to			// To this spot.
	)
	{
	from.tz = to.tz;		// Just look along dest's lift.
	Fast_pathfinder_client client(1);
	Astar path;
	return path.NewPath(from, to, &client);
	}

/*
 *	Create client for combat pathfinding.
 */

Combat_pathfinder_client::Combat_pathfinder_client
	(
	Actor *attacker,
	int reach,			// Weapon reach in tiles.
	Game_object *opponent
	) : Fast_pathfinder_client(reach), destbox(0, 0, 0, 0)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info1 = gwin->get_info(attacker);
	axtiles = info1.get_3d_xtiles();
	aytiles = info1.get_3d_ytiles();
	aztiles = info1.get_3d_height();
	if (!opponent)
		return;			// Means this isn't usable.
	Shape_info& info2 = gwin->get_info(opponent);
	Tile_coord opos = opponent->get_abs_tile_coord();
	int oxtiles = info2.get_3d_xtiles(), oytiles = info2.get_3d_ytiles();
	destbox = Rectangle(opos.tx - oxtiles + 1, opos.ty - oytiles + 1,
							oxtiles, oytiles);
	destbox.enlarge(reach);		// This is how close we need to get.
	}


/*
 *	Is tile at goal?
 */

int Combat_pathfinder_client::at_goal
	(
	Tile_coord& tile,
	Tile_coord& goal
	)
	{
	Rectangle abox(tile.tx - axtiles + 1, tile.ty - aytiles + 1,
						axtiles, aytiles);
	return abox.intersects(destbox);
	}

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Monster_pathfinder_client::get_step_cost
	(
	Tile_coord from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (Chunk_object_list::is_blocked(axtiles, aytiles, aztiles,
							from, to))
		return -1;
	else
		return 1;
	}


