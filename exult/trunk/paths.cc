/*
 *	Paths.cc - Various pathfinding clients.
 *
 *  Copyright (C) 2000-2013  The Exult Team
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
#include "schedule.h"

/*
 *	Create for given NPC.
 */

Actor_pathfinder_client::Actor_pathfinder_client
	(
	Actor *n,
	int d,
	bool ign
	) : Pathfinder_client(n->get_type_flags()), dist(d), npc(n), ignore_npcs(ign)
	{
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

int Actor_pathfinder_client::check_blocking
	(
	Tile_coord const& from,
	Tile_coord const& to
	)
	{
	if (ignores_npcs())
		{
		Game_object *block = Game_object::find_blocking(to);
		if (!block)
			return -1;
		Actor *blk = block->as_actor();
		if (blk)
			{
				// If the blocking NPC is sitting, kneeling, lying down,
				// or in combat, he is an obstacle.
			int frnum = blk->get_framenum() & 0xf;
			if ((frnum >= Actor::sit_frame && frnum <= Actor::sleep_frame) ||
					blk->get_schedule_type() == Schedule::combat)
				return -1;
				// Otherwise, try to avoid non-moving NPCs.
			return blk->get_frame_time() ? 0 : 1;			// Try to avoid them.
			}
		}

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
	return 1;			// But try to avoid them.
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
		{			// Blocked, but check for a door or blocking NPC.
		int ret = check_blocking(from, to);
		if (ret < 0)
			return -1;
		else
			cost += ret;
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
	return cost;
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
	Actor *n,
	bool ign
	) : Actor_pathfinder_client(n, 0, ign), screen(
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
	Tile_coord const& b,			// Best offscreen pt. to aim for.
	bool ign
	) : Actor_pathfinder_client(n, 0, ign), screen(
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
 *	Create client for getting within a desired distance of a
 *	destination.
 */

Approach_object_pathfinder_client::Approach_object_pathfinder_client
	(
	Actor *from,
	Tile_coord const& dest,
	int dist
	) : Actor_pathfinder_client(from, dist, false),
	    destbox(Rectangle(dest.tx, dest.ty, 0, 0).enlarge(dist))
	{
	}

/*
 *	Create client for getting within a desired distance of a
 *	destination.
 */

Approach_object_pathfinder_client::Approach_object_pathfinder_client
	(
	Actor *from,
	Game_object *to,
	int dist
	) : Actor_pathfinder_client(from, dist, false),
	    destbox(to->get_footprint().enlarge(dist))
	{
	}

/*
 *	Is tile at goal?
 */

int Approach_object_pathfinder_client::at_goal
	(
	Tile_coord const& tile,
	Tile_coord const& goal
	)
	{
	int dz = tile.tz - goal.tz;	// Got to be on same floor.
	if (dz > 5 || dz < -5)
		return 0;
	return destbox.has_world_point(tile.tx, tile.ty);
	}

/*
 *	Create client for getting within a desired distance of a
 *	destination.
 */

Fast_pathfinder_client::Fast_pathfinder_client
	(
	Game_object *from,
	Tile_coord const& dest,
	int dist,
	int mf
	) : Pathfinder_client(mf),
	    destbox(Rectangle(dest.tx, dest.ty, 0, 0).enlarge(dist))
	{
	init(from, 0, dist);
	}

/*
 *	Create client for getting within a desired distance of an
 *	object.
 */

Fast_pathfinder_client::Fast_pathfinder_client
	(
	Game_object *from,
	Game_object *to,
	int dist,
	int mf
	) : Pathfinder_client(mf), destbox()
	{
	init(from, to, dist);
	}

/*
 *	Create client for getting within a desired distance of a
 *	destination.
 */

Fast_pathfinder_client::Fast_pathfinder_client
	(
	Actor *from,
	Tile_coord const& dest,
	int dist
	) : Pathfinder_client(from->get_type_flags()),
	    destbox(Rectangle(dest.tx, dest.ty, 0, 0).enlarge(dist))
	{
	init(from, 0, dist);
	}

/*
 *	Create client for getting within a desired distance of an
 *	object.
 */

Fast_pathfinder_client::Fast_pathfinder_client
	(
	Actor *from,
	Game_object *to,
	int dist
	) : Pathfinder_client(from->get_type_flags()), destbox()
	{
	init(from, to, dist);
	}

void Fast_pathfinder_client::init
	(
	Game_object *from,
	Game_object *to,
	int dist
	)
	{
	Shape_info& info1 = from->get_info();
	int frame1 = from->get_framenum();
	axtiles = info1.get_3d_xtiles(frame1);
	aytiles = info1.get_3d_ytiles(frame1);
	aztiles = info1.get_3d_height();
	if (to)			// Where we want to get.
		destbox = to->get_footprint().enlarge(dist);
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
	return from.distance(to);	// Distance() does world-wrapping.
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
	int dz = tile.tz - goal.tz;	// Got to be on same floor.
	if (dz > 5 || dz < -5)
		return 0;
	Rectangle abox(tile.tx - axtiles + 1, tile.ty - aytiles + 1,
						axtiles, aytiles);
	return abox.intersects(destbox);
	}

static void Get_closest_edge
	(
	Block const& fromvol,
	Block const& tovol,
	Tile_coord& pos1,
	Tile_coord& pos2
	)
	{
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
		// Needed for sails.
	if (pos2.tz < pos1.tz)	// Going down?
		pos2.tz += ht2 - 1;
				// Use top tile.
	pos1.tz += ht1 - 1;
	}

int Fast_pathfinder_client::is_grabable_internal
	(
	Game_object *from,
	Tile_coord const& ct,
	Tile_coord const& dt,
	Block const& tovol,
	Fast_pathfinder_client& client
	)
	{
	Game_map *gmap = Game_window::get_instance()->get_map();

	Block fromvol = from->get_block();
	Tile_coord src = from->get_tile(), dst(dt);
	Get_closest_edge(fromvol, tovol, src, dst);
	src.tz = from->get_lift();

	Astar path;
	if (!path.NewPath(src, dst, &client))
		return 0;

	Tile_coord t;			// Check each tile.
	bool done;
	if (path.get_num_steps() == 0)
		t = from->get_tile();
	else
		{
		while (path.GetNextStep(t, done))
			if (t != from->get_tile() && !client.at_goal(t, dst) &&
					gmap->is_tile_occupied(t))
				return 0;	// Blocked.
		}

	if (!client.at_goal(t, dst))
		return 0;

	Block srcvol(fromvol);
	fromvol.x += t.tx - src.tx;
	fromvol.y += t.ty - src.ty;
	fromvol.z += t.tz - src.tz;
	dst = dt;
	Get_closest_edge(fromvol, tovol, t, dst);

	Zombie zpath;
	if (!zpath.NewPath(t, dst, 0))	// Should always succeed.
		return 0;

	if (zpath.get_num_steps() == 0)
		return 0;

	Game_object *block;
	while (zpath.GetNextStep(t, done))
		if (!tovol.has_world_point(t.tx, t.ty, t.tz) &&
					!srcvol.has_world_point(t.tx, t.ty, t.tz) &&
					!fromvol.has_world_point(t.tx, t.ty, t.tz) && 
					gmap->is_tile_occupied(t) &&
					(block = Game_object::find_blocking(t)) != 0 &&
					// Ignore all blocking actors and movable objects.
					block->as_actor() == 0 && !block->is_dragable())
			return 0;	// Blocked.
	return 1;
	}

// Need this much for gems in swamp E of Empath Abbey
#define MAX_GRAB_DIST 5

int Fast_pathfinder_client::is_grabable
	(
	Game_object *from,		// From this object.
	Game_object *to,		// To this object.
	int mf
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.

	for (int i = 1; i <= MAX_GRAB_DIST; i++)
		{
		Fast_pathfinder_client client(from, to, i, mf);
		if (is_grabable_internal(from, to->get_center_tile(), to->get_tile(),
		                         to->get_block(), client))
			return 1;
		}
	return 0;
	}

int Fast_pathfinder_client::is_grabable
	(
	Game_object *from,		// From this object.
	Tile_coord const& to,	// To this spot.
	int mf
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.

	for (int i = 1; i <= MAX_GRAB_DIST; i++)
		{
		Fast_pathfinder_client client(from, to, i, mf);
		if (is_grabable_internal(from, to, to,
		                         Block(to.tx, to.ty, to.tz, 1, 1, 1),
		                         client))
			return 1;
		}
	return 0;
	}


int Fast_pathfinder_client::is_grabable
	(
	Actor *from,			// From this object.
	Game_object *to			// To this object.
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.

	for (int i = 1; i <= MAX_GRAB_DIST; i++)
		{
		Fast_pathfinder_client client(from, to, i);
		if (is_grabable_internal(from, to->get_center_tile(), to->get_tile(),
		                         to->get_block(), client))
			return 1;
		}
	return 0;
	}

int Fast_pathfinder_client::is_grabable
	(
	Actor *from,			// From this object.
	Tile_coord const& to	// To this spot.
	)
	{
	if (from->distance(to) <= 1)
		return 1;		// Already okay.

	for (int i = 1; i <= MAX_GRAB_DIST; i++)
		{
		Fast_pathfinder_client client(from, to, i);
		if (is_grabable_internal(from, to, to,
		                         Block(to.tx, to.ty, to.tz, 1, 1, 1),
		                         client))
			return 1;
		}
	return 0;
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
	Block fromvol = from->get_block(), tovol = to->get_block();
	Tile_coord pos1(from->get_tile()), pos2(to->get_tile());
	Get_closest_edge(fromvol, tovol, pos1, pos2);

	Game_map *gmap = Game_window::get_instance()->get_map();
	Zombie path;
	if (!path.NewPath(pos1, pos2, 0))	// Should always succeed.
		return 0;
	Tile_coord t;			// Check each tile.
	bool done;
	while (path.GetNextStep(t, done))
		if (!tovol.has_world_point(t.tx, t.ty, t.tz) &&
					!fromvol.has_world_point(t.tx, t.ty, t.tz) && 
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
	) : Fast_pathfinder_client(npc, dest, dist, npc->get_type_flags()),
	    intelligence(npc->get_property(Actor::intelligence))
	{
	}

/*
 *	Create client for combat pathfinding.
 */

Monster_pathfinder_client::Monster_pathfinder_client
	(
	Actor *attacker,
	int reach,			// Weapon reach in tiles.
	Game_object *opponent
	) : Fast_pathfinder_client(attacker, opponent, reach, attacker->get_type_flags()),
	    intelligence(attacker->get_property(Actor::intelligence))
	{
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
	if (Map_chunk::is_blocked(get_axtiles(), get_aytiles(), get_aztiles(),
						from, to, get_move_flags()))
		return -1;
	else
		return 1;
	}
