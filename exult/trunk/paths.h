/*
 *	Paths.h - Various pathfinding clients.
 *
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

#ifndef PATHS_H
#define PATHS_H	1

#include "PathFinder.h"
#include "tiles.h"
#include "rect.h"
#include "chunks.h"

class Actor;
class Game_object;
class Game_window;

/*
 *	This class provides A* cost methods.
 */
class Actor_pathfinder_client : public Pathfinder_client
	{
	int dist;			// Distance for success.
	Actor *npc;			// Who this represents.
public:
//	Actor_pathfinder_client(Actor *npc, int d = 0) : dist(d)
//		{ set_move_flags(mf); }
	Actor_pathfinder_client(Actor *npc, int d = 0);
					// Figure when to give up.
	virtual int get_max_cost(int cost_to_goal);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	};

/*
 *	This client succeeds when the path makes it to just one X/Y coord.
 *	It assumes that a -1 was placed in the coord. that we should ignore.
 */
class Onecoord_pathfinder_client : public Actor_pathfinder_client
	{
public:
	Onecoord_pathfinder_client(Actor *n) : Actor_pathfinder_client(n)
		{  }
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	};

/*
 *	This client succeeds when the path makes it offscreen.
 *	Only the tz coord. of the dest. is used.
 */
class Offscreen_pathfinder_client : public Actor_pathfinder_client
	{
	Rectangle screen;		// Screen rect. in tiles.
	Tile_coord best;		// Best offscreen pt. to aim for.
public:
	Offscreen_pathfinder_client(Actor *n);
	Offscreen_pathfinder_client(Actor *n, Tile_coord b);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
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
	Fast_pathfinder_client(int d = 0, int mf = 1 << 5) : dist(d)
		{ set_move_flags(mf); }
					// Figure when to give up.
	virtual int get_max_cost(int cost_to_goal);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	static int is_grabable(Tile_coord from, Tile_coord to);
					// Check for unblocked straight path.
	static int is_straight_path(Tile_coord from, Tile_coord to);
	};

/*
 *	Pathfinding for monsters, who may be bigger than 1x1:
 */
class Monster_pathfinder_client : public Fast_pathfinder_client
	{
	Rectangle destbox;		// Got to intersect this box.
	int intelligence;		// NPC's intelligence.
	int axtiles, aytiles, aztiles;	// NPC's dims. in tiles.
public:
	Monster_pathfinder_client(Actor *npc, Tile_coord dest, int dist);
					// For combat:
	Monster_pathfinder_client(Actor *attacker, int reach,
						Game_object *opponent);
					// Figure when to give up.
	virtual int get_max_cost(int cost_to_goal);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
	};


#endif	/* INCL_PATHS */

