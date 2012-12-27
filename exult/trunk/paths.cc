/*
 *	Paths.cc - Various pathfinding clients.
 *
 *  Copyright (C) 2000-2011  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdlib>
#include "paths.h"
#include "Astar.h"
#include "Zombie.h"
#include "gamewin.h"
#include "gamemap.h"
#include "actors.h"

/*
 *	Create for given NPC.
 */

Actor_pathfinder_client::Actor_pathfinder_client
	(
	Actor *n,
	int d
	) : dist(d), npc(n)
	{
					// +++++Shouldn't need this anymore?
	set_move_flags(npc->get_type_flags());
	}

/*
 *	Figure when to give up.
 */

int Actor_pathfinder_client::get_max_cost
	(
	int cost_to_goal		// From estimate_cost().
	)
	{
	int max_cost = 3*cost_to_goal;
	Game_window *gwin = Game_window::get_instance();
					// Do at least 3 screens width.
	int min_max_cost = (gwin->get_width()/c_tilesize)*2*3;
	return max_cost > min_max_cost ? max_cost : min_max_cost;
	}

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Actor_pathfinder_client::get_step_cost
	(
	Tile_coord const& frm,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int cx = to.tx/c_tiles_per_chunk, cy = to.ty/c_tiles_per_chunk;
	Map_chunk *olist = gwin->get_map()->get_chunk(cx, cy);
	int tx = to.tx%c_tiles_per_chunk;	// Get tile within chunk.
	int ty = to.ty%c_tiles_per_chunk;
	int cost = 1;
	olist->setup_cache();		// Make sure cache is valid.
	int water, poison;		// Get tile info.
	Actor::get_tile_info(0, gwin, olist, tx, ty, water, poison);
	int old_lift = to.tz;		// Might climb/descend.
	Tile_coord from(frm);
	if (npc->is_blocked(to, &from))
		{			// Blocked, but check for a door.
		Game_object *block = Game_object::find_door(to);
		if (!block)
			return -1;
		if (!block->is_closed_door() ||
					// Can't get past locked doors.
		    block->get_framenum()%4 >= 2)
			return -1;
					// Can't be either end of door.
		Rectangle foot = block->get_footprint();
		if (foot.h == 1 && (to.tx == foot.x ||
		                    to.tx == FIX_COORD(foot.x + foot.w - 1)))
			return -1;
		else if (foot.w == 1 && (to.ty == foot.y ||
		                         to.ty == FIX_COORD(foot.y + foot.h - 1)))
			return -1;
		if (foot.has_world_point(from.tx, from.ty))
			return -1;	// Don't walk within doorway.
		cost++;			// But try to avoid them.
		}
	if (old_lift != to.tz)
		cost++;
					// On the diagonal?
	if (from.tx != to.tx || from.ty != to.ty)
		cost *= 3;		// Make it 50% more expensive.
	else
		cost *= 2;
	if (poison && to.tz == 0)
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
	Tile_coord const& from,
	Tile_coord const& to
	)
	{
	int dx = to.tx - from.tx;
	if (dx < -c_num_tiles/2)	// Wrap around the world.
		dx += c_num_tiles;
	else if (dx < 0)
		dx = -dx;
	int dy = to.ty - from.ty;
	if (dy < -c_num_tiles/2)
		dy += c_num_tiles;
	else if (dy < 0)
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
 *	Is tile at goal?
 */

int Actor_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	return (goal.tz==-1 ? tile.distance_2d(goal) : tile.distance(goal))<= dist;
	}

/*
 *	Estimate cost from one point to another.
 */

int Onecoord_pathfinder_client::estimate_cost
	(
	Tile_coord const& from,
	Tile_coord const& to			// Should be the goal.
	)
	{
	if (to.tx == -1)		// Just care about Y?
		{			// Cost = 2/tile.
		int dy = to.ty - from.ty;
		if (dy < -c_num_tiles/2)
			dy += c_num_tiles;
		else if (dy < 0)
			dy = -dy;
		return (2*dy);
		}
	else if (to.ty == -1)
		{
		int dx = to.tx - from.tx;
		if (dx < -c_num_tiles/2)
			dx += c_num_tiles;
		else if (dx < 0)
			dx = -dx;
		return (2*dx);
		}
	else				// Shouldn't get here.
		return Actor_pathfinder_client::estimate_cost(from, to);
	}

/*
 *	Is tile at goal?
 */

int Onecoord_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	return ((goal.tx == -1 || tile.tx == goal.tx) && 
		(goal.ty == -1 || tile.ty == goal.ty) &&
		(goal.tz == -1 || tile.tz == goal.tz));
	}

/*
 *	Client to get offscreen.
 */

Offscreen_pathfinder_client::Offscreen_pathfinder_client
	(
	Actor *n
	) : Actor_pathfinder_client(n), screen(
	      Game_window::get_instance()->get_win_tile_rect().enlarge(3)),
	    best(-1, -1, -1)
	{
	}

/*
 *	Client to get offscreen.
 */

Offscreen_pathfinder_client::Offscreen_pathfinder_client
	(
	Actor *n,
	Tile_coord const& b			// Best offscreen pt. to aim for.
	) : Actor_pathfinder_client(n), screen(
	      Game_window::get_instance()->get_win_tile_rect().enlarge(3)),
	    best(b)
	{
	if (best.tx != -1)		// Scale (roughly) to edge of screen.
		{
		Rectangle scr = 
			Game_window::get_instance()->get_win_tile_rect();
					// Get center.
		int cx = scr.x + scr.w/2, cy = scr.y + scr.h/2;
					// More than 4 screens away?
		if (best.distance_2d(Tile_coord(cx, cy, 0)) > 4*scr.w)
			{
			best.tx = best.ty = -1;
			return;
			}
		if (best.tx > cx + scr.w)
					// Too far to right of screen.
			best.tx = scr.x + scr.w + 1;
		else if (best.tx < cx - scr.w)
			best.tx = scr.x - 1;
		if (best.ty > cy + scr.h)
			best.ty = scr.y + scr.h + 1;
		else if (best.ty < cy - scr.h)
			best.ty = scr.y - 1;
					// Give up if it doesn't look right.
		if (best.distance_2d(Tile_coord(cx, cy, 0)) > scr.w)
			best.tx = best.ty = -1;
		}
	}

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Offscreen_pathfinder_client::get_step_cost
	(
	Tile_coord const& from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	int cost = Actor_pathfinder_client::get_step_cost(from, to);
	if (cost == -1)
		return cost;
	if (best.tx != -1)		// Penalize for moving away from best.
		{
		if ((to.tx - from.tx)*(best.tx - from.tx) < 0)
			cost++;
		if ((to.ty - from.ty)*(best.ty - from.ty) < 0)
			cost++;
		}
	return cost;
	}

/*
 *	Estimate cost from one point to another.
 */

int Offscreen_pathfinder_client::estimate_cost
	(
	Tile_coord const& from,
	Tile_coord const& to			// Should be the goal.
	)
	{
	if (best.tx != -1)		// Off-screen goal?
		return Actor_pathfinder_client::estimate_cost(from, best);
//++++++World-wrapping here????
	int dx = from.tx - screen.x;	// Figure shortest dist.
	int dx1 = screen.x + screen.w - from.tx;
	if (dx1 < dx)
		dx = dx1;
	int dy = from.ty - screen.y;
	int dy1 = screen.y + screen.h - from.ty;
	if (dy1 < dy)
		dy = dy1;
	int cost = dx < dy ? dx : dy;
	if (cost < 0)
		cost = 0;
	if (to.tz != -1 && from.tz != to.tz)
		cost++;
	return 2*cost;
	}

/*
 *	Is tile at goal?
 */

int Offscreen_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	return !screen.has_world_point(tile.tx - tile.tz/2, tile.ty - tile.tz/2);//&&
					// Z-coord shouldn't matter.
//		(goal.tz == -1 || tile.tz == goal.tz);
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
	Tile_coord const& from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	int cx = to.tx/c_tiles_per_chunk, cy = to.ty/c_tiles_per_chunk;
	Map_chunk *olist = gwin->get_map()->get_chunk(cx, cy);
	int tx = to.tx%c_tiles_per_chunk;	// Get tile within chunk.
	int ty = to.ty%c_tiles_per_chunk;
	olist->setup_cache();		// Make sure cache is valid.
	int new_lift;			// Might climb/descend.
					// Look at 1 tile's height, and step up/down 1.
	if (olist->is_blocked(1, to.tz, tx, ty, new_lift, get_move_flags(), 
									1, 1))
		return -1;
	to.tz = new_lift;		// (I don't think we should do this above...)
	return 1;
	}

/*
 *	Estimate cost from one point to another.
 */

int Fast_pathfinder_client::estimate_cost
	(
	Tile_coord const& from,
	Tile_coord const& to
	)
	{
	return from.distance_2d(to);	// Distance() does world-wrapping.
	}

/*
 *	Is tile at goal?
 */

int Fast_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	if (tile.distance_2d(goal) > dist)
		return 0;		// Not close enough.
	int dz = tile.tz - goal.tz;	// Want to be within 1 story.
	return dz <= 5 && dz >= -5;
	}

#define MAX_GRAB_DIST 3

/*
 *	Just test to see if an object can be grabbed.
 */

int Fast_pathfinder_client::is_grabable
	(
	Tile_coord const& from,		// From this spot.
	Tile_coord const& to			// To this spot.
	)
	{
	if (from.distance(to) <= 1)
		return 1;		// Already okay.
	Fast_pathfinder_client client(MAX_GRAB_DIST,
	   Game_window::get_instance()->get_main_actor()->get_type_flags());
	Astar path;
	return path.NewPath(from, to, &client);
	}

int Fast_pathfinder_client::is_grabable
	(
	Game_object *from,		// From this object.
	Game_object *to			// To this object.
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.
				// Effectively enlarge object by its smallest 2d dimension.
	Shape_info& inf = to->get_info();
	int dx = inf.get_3d_xtiles(to->get_framenum()), dy = inf.get_3d_ytiles(to->get_framenum());
	int mindelta = dx < dy ? dx : dy;
				// Halve this (round up) for a radius, then increase by 1.
	Fast_pathfinder_client client((mindelta + 1) / 2 + 1, 
	   Game_window::get_instance()->get_main_actor()->get_type_flags());
	Astar path;
				// We search centered on the target object.
	return path.NewPath(from->get_tile(), to->get_center_tile(), &client);
	}

int Fast_pathfinder_client::is_grabable
	(
	Game_object *from,		// From this object.
	Tile_coord const& to			// To this spot.
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.
	Fast_pathfinder_client client(MAX_GRAB_DIST, 
	   Game_window::get_instance()->get_main_actor()->get_type_flags());
	Astar path;
	return path.NewPath(from->get_tile(), to, &client);
	}

/*
 *	Check for an unblocked straight path.
 *	NOTE1:  This really has nothing to do with Fast_pathfinder_client.
 *	NOTE2:	This version doesn't check the z-coord at all.
 */

int Fast_pathfinder_client::is_straight_path
	(
	Tile_coord const& from,		// From this spot.
	Tile_coord const& to			// To this spot.
	)
	{
	Game_map *gmap = Game_window::get_instance()->get_map();

	Zombie path;
	if (!path.NewPath(from, to, 0))	// Should always succeed.
		return 0;
	Tile_coord t;			// Check each tile.
	bool done;
	while (path.GetNextStep(t, done))
		if (t != from && t != to && gmap->is_tile_occupied(t))
			return 0;	// Blocked.
	return 1;			// Looks okay.
	}

/*
 *	Check for path from one object to another, using their closest corners.
 */

int Fast_pathfinder_client::is_straight_path
	(
	Game_object *from, Game_object *to
	)
	{
	Block fromvol = from->get_block(),
	      tovol = to->get_block();
	Tile_coord pos1 = from->get_tile();
	Tile_coord pos2 = to->get_tile();
	int ht1 = fromvol.h;
	int ht2 = tovol.h;
	if (pos2.tx < pos1.tx)	// Going left?
		pos1.tx = fromvol.x;
	else			// Right?
		pos2.tx = tovol.x;
	if (pos2.ty < pos1.ty)	// Going north?
		pos1.ty = fromvol.y;
	else			// South.
		pos2.ty = tovol.y;
				// Use top tile.
	pos1.tz += ht1 - 1;
	pos2.tz += ht2 - 1;
	Game_map *gmap = Game_window::get_instance()->get_map();
	Zombie path;
	if (!path.NewPath(pos1, pos2, 0))	// Should always succeed.
		return 0;
	Tile_coord t;			// Check each tile.
	bool done;
	while (path.GetNextStep(t, done) && 
				!tovol.has_world_point(t.tx, t.ty, t.tz))
		if (!fromvol.has_world_point(t.tx, t.ty, t.tz) && 
					gmap->is_tile_occupied(t))
			return 0;	// Blocked.
	return 1;			// Looks okay.
	}

/*
 *	Create client for getting within a desired distance of a
 *	destination.
 */

Monster_pathfinder_client::Monster_pathfinder_client
	(
	Actor *npc,			// 'Monster'.
	Tile_coord const& dest,
	int dist
	) : Fast_pathfinder_client(dist), destbox(dest.tx, dest.ty, 0, 0)
	{
	intelligence = npc->get_property(Actor::intelligence);
	Shape_info& info1 = npc->get_info();
	int frame = npc->get_framenum();
	axtiles = info1.get_3d_xtiles(frame);
	aytiles = info1.get_3d_ytiles(frame);
	aztiles = info1.get_3d_height();
	set_move_flags(npc->get_type_flags());
	destbox.enlarge(dist);		// How close we need to get.
	}

/*
 *	Create client for combat pathfinding.
 */

Monster_pathfinder_client::Monster_pathfinder_client
	(
	Actor *attacker,
	int reach,			// Weapon reach in tiles.
	Game_object *opponent
	) : Fast_pathfinder_client(reach), destbox(0, 0, 0, 0)
	{
	intelligence = attacker->get_property(Actor::intelligence);
	Shape_info& info1 = attacker->get_info();
	int frame1 = attacker->get_framenum();
	axtiles = info1.get_3d_xtiles(frame1);
	aytiles = info1.get_3d_ytiles(frame1);
	aztiles = info1.get_3d_height();
	if (!opponent)
		return;			// Means this isn't usable.
	set_move_flags(attacker->get_type_flags());
	Shape_info& info2 = opponent->get_info();
	Tile_coord opos = opponent->get_tile();
	int frame2 = opponent->get_framenum();
	int oxtiles = info2.get_3d_xtiles(frame2), oytiles = info2.get_3d_ytiles(frame2);
	destbox = Rectangle(opos.tx - oxtiles + 1, opos.ty - oytiles + 1,
							oxtiles, oytiles);
	destbox.enlarge(reach);		// This is how close we need to get.
	}

/*
 *	Figure when to give up.
 */

int Monster_pathfinder_client::get_max_cost
	(
	int cost_to_goal		// From estimate_cost().
	)
	{
	int max_cost = 2*cost_to_goal;	// Don't try to hard.
					// Use intelligence.
	max_cost += intelligence/2;
	Game_window *gwin = Game_window::get_instance();
					// Limit to 3/4 screen width.
	int scost = ((3*gwin->get_width())/4)/c_tilesize;
	if (max_cost > scost)
		max_cost = scost;
	if (max_cost < 18)		// But not too small.
		max_cost = 18;
	return max_cost;
	}

/*
 *	Is tile at goal?
 */

int Monster_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	int dz = tile.tz - goal.tz;	// Got to be on same floor.
	if (dz/5 != 0)
		return 0;
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
	Tile_coord const& from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	if (Map_chunk::is_blocked(axtiles, aytiles, aztiles,
						from, to, get_move_flags()))
		return -1;
	else
		return 1;
	}
