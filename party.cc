/**
 **	Party.cc - Manage the party.
 **
 **	Written: 4/8/02 - JSF
 **/
/*  Copyright (C) 2000-2003  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <string>
#include "party.h"
#include "actors.h"
#include "gamewin.h"
#include "frameseq.h"

using std::cout;
using std::endl;

/*
 *	Create.
 */

Party_manager::Party_manager
	(
	) : party_count(0), dead_party_count(0)
	{  
					// Clear party list.
	std::memset((char *) &party[0], 0, sizeof(party));
	std::memset((char *) &dead_party[0], 0, sizeof(dead_party));
	}

/*
 *	Add NPC to party.
 *
 *	Output:	false if no room or already a member.
 */

bool Party_manager::add_to_party
	(
	Actor *npc			// (Should not be the Avatar.)
	)
	{
	const int maxparty = sizeof(party)/sizeof(party[0]);
	if (!npc || party_count == maxparty || npc->is_in_party())
		return false;
	remove_from_dead_party(npc);	// Just to be sure.
	npc->set_party_id(party_count);
	npc->set_flag (Obj_flags::in_party);
					// We can take items.
	npc->set_flag_recursively(Obj_flags::okay_to_take);
	party[party_count++] = npc->get_npc_num();
	return true;
	}

/*
 *	Remove party member.
 *
 *	Output:	false if not found.
 */

bool Party_manager::remove_from_party
	(
	Actor *npc
	)
	{
	if (!npc)
		return false;
	int id = npc->get_party_id();
	if (id == -1)			// Not in party?
		return false;
	int npc_num = npc->get_npc_num();
	if (party[id] != npc_num)
		{
		cout << "Party mismatch!!" << endl;
		return false;
		}
					// Shift the rest down.
	for (int i = id + 1; i < party_count; i++)
		{
		Actor *npc2 = gwin->get_npc(party[i]);
		if (npc2)
			npc2->set_party_id(i - 1);
		party[i - 1] = party[i];
		}
	npc->clear_flag (Obj_flags::in_party);
	party_count--;
	party[party_count] = 0;
	npc->set_party_id(-1);
	return true;
	}

/*
 *	Find index of NPC in dead party list.
 *
 *	Output:	Index, or -1 if not found.
 */

int Party_manager::in_dead_party
	(
	Actor *npc
	)
	{
	int num = npc->get_npc_num();
	for (int i = 0; i < dead_party_count; i++)
		if (dead_party[i] == num)
			return i;
	return -1;
	}

/*
 *	Add NPC to dead party list.
 *
 *	Output:	false if no room or already a member.
 */

bool Party_manager::add_to_dead_party
	(
	Actor *npc			// (Should not be the Avatar.)
	)
	{
	const int maxparty = sizeof(dead_party)/sizeof(dead_party[0]);
	if (!npc || dead_party_count == maxparty || in_dead_party(npc) >= 0)
		return false;
	dead_party[dead_party_count++] = npc->get_npc_num();
	return true;
	}

/*
 *	Remove NPC from dead party list.
 *
 *	Output:	false if not found.
 */

bool Party_manager::remove_from_dead_party
	(
	Actor *npc
	)
	{
	if (!npc)
		return false;
	int id = in_dead_party(npc);	// Get index.
	if (id == -1)			// Not in list?
		return false;
	int npc_num = npc->get_npc_num();
					// Shift the rest down.
	for (int i = id + 1; i < dead_party_count; i++)
		dead_party[i - 1] = dead_party[i];
	dead_party_count--;
	dead_party[dead_party_count] = 0;
	return true;
	}

/*
 *	Update party status of an NPC that has died or been resurrected.
 */

void Party_manager::update_party_status
	(
	Actor *npc
	)
	{
	if (npc->is_dead())		// Dead?
		{
					// Move party members to dead list.
		if (remove_from_party(npc))
			add_to_dead_party(npc);
		}
	else				// Alive.
		{
		if (remove_from_dead_party(npc))
			add_to_party(npc);
		}
	}

/*
 *	In case NPC's were read after usecode, set party members' id's, and
 *	move dead members into separate list.
 */

void Party_manager::link_party
	(
	)
	{
	// avatar is a party member too
	gwin->get_main_actor()->set_flag(Obj_flags::in_party);
					// You own your own stuff.
	gwin->get_main_actor()->set_flag_recursively(Obj_flags::okay_to_take);
	const int maxparty = sizeof(party)/sizeof(party[0]);
	int tmp_party[maxparty];
	int tmp_party_count = party_count;
	int i;
	for (i = 0; i < maxparty; i++)
		tmp_party[i] = party[i];
	party_count = dead_party_count = 0;
					// Now process them.
	for (i = 0; i < tmp_party_count; i++)
		{
		Actor *npc = gwin->get_npc(party[i]);
		int oldid;
		if (!npc ||		// Shouldn't happen!
					// But this has happened:
		    ((oldid = npc->get_party_id()) >= 0 && 
							oldid < party_count))
			continue;	// Skip bad entry.
		int npc_num = npc->get_npc_num();
		if (npc->is_dead())	// Put dead in special list.
			{
			npc->set_party_id(-1);
			if (dead_party_count >= 
				    sizeof(dead_party)/sizeof(dead_party[0]))
				continue;
			dead_party[dead_party_count++] = npc_num;
			continue;
			}
		npc->set_party_id(party_count);
		party[party_count++] = npc_num;
// ++++This messes up places where they should wait, and should be unnecessary.
//		npc->set_schedule_type(Schedule::follow_avatar);
					// We can use all his/her items.
		npc->set_flag_recursively(Obj_flags::okay_to_take);
		npc->set_flag (Obj_flags::in_party);
		}
	}

/*
 *	For each party member, this array has the party ID's (or -1) of the
 *	two member's followers, arrayed as follows:
 *			A
 *		       0 1
 *		      2 3 4
 *		     7 5 6 8
 */
static int followers[EXULT_PARTY_MAX + 1][2] = {
	{0, 1},				// These follow Avatar (ID = -1).
	{2, 3},				// Follow 0.
	{-1, 4},			// Follow 1.
	{7, -1},			// Follow 2.
	{5, 6},				// Follow 3.
	{-1, 8},			// Follow 4.
	{-1, -1}, {-1, -1}, {-1, -1}};

/*
 *	Offsets for the follower, depending on direction (0-3, with
 *	0 = North, 1 = East, 2 = South, 3 = West).
 */
static int left_offsets[4][2] = {	// Follower is behind and to left.
	{-2, 2},			// North.
	{-2, -2},			// East.
	{2, -2},			// South.
	{2, 2} };			// West.
static int right_offsets[4][2] = {	// Follower is behind and to right.
	{2, 2},				// North.
	{-2, 2},			// East.
	{-2, -2},			// South.
	{2, -2} };			// West.

/*
 *	To walk in formation, each party member will have one or two other
 *	party members who will follow him on each step.
 */

void Party_manager::move_followers
	(
	Actor *npc,			// Party member who just stepped.
	int dir				// Direction (0-7) they're going.
	)
	{
	int id = npc->get_party_id();	// (-1 if Avatar).
	Tile_coord pos = npc->get_tile();
	int lnum = followers[1 + id][0], rnum = followers[1 + id][1];
	if (lnum == -1 && rnum == -1)
		return;			// Nothing to do.
	int dir4 = dir/2;		// 0-3 now.
//++++++Got to deal with NPC's that are asleep, etc.
	Actor *lnpc = (lnum == -1 || lnum >= party_count) ? 0 
					: gwin->get_npc(party[lnum]);
	Actor *rnpc = (rnum == -1 || rnum >= party_count) ? 0 
					: gwin->get_npc(party[rnum]);
	bool lmoved = false, rmoved = false;
					// Have each take a step.
	if (lnpc)
		lmoved = step(lnpc, npc, dir, pos + Tile_coord(
			left_offsets[dir4][0], left_offsets[dir4][1], 0));
	if (rnpc)
		rmoved = step(rnpc, npc, dir, pos + Tile_coord(
			right_offsets[dir4][0], right_offsets[dir4][1], 0));
	if (lmoved)
		move_followers(lnpc, dir);
	if (rmoved)
		move_followers(rnpc, dir);
	}

/*
 *	Get tile to step to, given destination tile (possibly more than 1
 *	step away), and the party's direction.
 */

inline Tile_coord Get_step_tile
	(
	Tile_coord pos,			// Current pos.
	Tile_coord dest,		// Desired dest.
	int dir				// Dir. party is moving (0-7).
	)
	{
	int dx = dest.tx - pos.tx, dy = dest.ty - pos.ty;
					// Get adjacent tile in given dir.
	Tile_coord adj = pos.get_neighbor(dir);
					// See if we're past desired spot.
	if (dx < 0)			// Dest. to our left?
		{
		if (adj.tx > pos.tx && dx >= -4)// But walking right.
			dx = 0;
		else if (dx < -1)
			dx = -1;	// Can only step by 1 tile.
		}
	else if (dx > 0)
		{
		if (adj.tx < pos.tx && dx <= 4)	// Walking left?
			dx = 0;
		else if (dx > 1)
			dx = 1;
		}
	if (dy < 0)			// Dest. North?
		{
		if (adj.ty > pos.ty && dy >= -4)	// But walking South?
			dy = 0;
		else if (dy < -1)
			dy = -1;
		}
	else if (dy > 0)		// Dest. South?
		{
		if (adj.ty < pos.ty && dy <= 4)
			dy = 0;
		else if (dy > 1)
			dy = 1;
		}
	return pos + Tile_coord(dx, dy, 0);
	}

/*
 *	Find the party member occupying a given tile, starting with a given
 *	party #.
 *	Note:	Maybe it should check a rectangle of tiles someday if we want
 *		to have NPC's bigger than 1 tile.
 */

static Actor *Find_member_blocking
	(
	Tile_coord pos,			// Position to check.
	int first			// Party ID to start with.
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Party_manager *pman = gwin->get_party_man();
	int count = pman->get_count();

	for (int i = first; i < count; i++)
		{
		Actor *npc = gwin->get_npc(pman->get_member(i));
		if (npc->blocks(pos))
			return npc;	// Found.
		}
	return 0;
 	}

/*
 *	Get a notion of 'cost' for stepping to a particular tile.
 *
 *	Output:	Currently, 10000 if blocked, or (delta-z)**2, so a
 *		difference in z of 1 will return 1 as the cost.
 */

const int max_cost = 10000;
static int Get_cost
	(
	Actor *npc,			// NPC to take the step.
	Actor *leader,			// NPC he's following.
	Tile_coord to,			// Tile to step to.
	Actor **find_blocking = 0	// Returns blocking party member.
	)
	{
	int cost = 0;
	if (npc->is_blocked(to))	// (To.tz is updated.)
		{			// Can't go there.
		if (find_blocking)
			{		// Find member we can swap with.
			*find_blocking = Find_member_blocking(to,
						1 + npc->get_party_id());
			if (!*find_blocking)
				return max_cost;
			to.tz = (*find_blocking)->get_lift();
			cost += 1;	// Assess one point to swap.
			}
		else
			return max_cost;
		}
	int ltz = leader->get_lift();	// Leader's z-coord.
	int difftz = to.tz - ltz;	// Measure closeness.
	cost += difftz*difftz;
	return cost;
	}
#if 0	/* ++++ Not used. */
/*
 *	Take the better of two steps.
 *
 *	Output:	True if successful.
 */

static bool Take_better_step
	(
	Actor *npc,
	Actor *leader,
	Tile_coord& pos,		// Current pos.
	int frame,			// Frame to show.
	int dir1, int dir2		// Dirs.  We'll take mod 8.
	)
	{
	dir1 %= 8; dir2 %= 8;
	Tile_coord dest1 = pos.get_neighbor(dir1);
	int cost1 = Get_cost(npc, leader, dest1);
	if (!cost1 && npc->step(dest1, frame))
		return true;
	Tile_coord dest2 = pos.get_neighbor(dir2);
	int cost2 = Get_cost(npc, leader, dest2);
	if (cost1 <= cost2)
		{
		if (cost1 == max_cost)	// Both fail.
			return false;
		if (npc->step(dest1, frame))
			return true;
		}
	return npc->step(dest2, frame);
	}
#endif

/*
 *	Take best step to follow the leader.
 *
 *	Output:	True if a step taken.
 */

static bool Take_best_step
	(
	Actor *npc,
	Actor *leader,
	Tile_coord& pos,		// Current pos.
	int frame,			// Frame to show.
	int dir				// Direction we want to go.
	)
	{
	static int deltadir[8] = {0, 1, 7, 2, 6, 3, 5, 4};

	int best_cost = max_cost + 8;
	Tile_coord best(-1, -1, -1);
	for (int i = 0; i < 8; i++)
		{
		int diri = (dir + deltadir[i])%8;
		Tile_coord to = pos.get_neighbor(diri);
					// Fudge cost with diff. in dir.
		int cost = Get_cost(npc, leader, to) + (i + 1)/2;
		if (cost < best_cost)
			{
			best_cost = cost;
			best = to;
			}
		}
	if (best_cost < max_cost)
		return npc->step(best, frame);
	else
		return false;
	}

/*
 *	Move one follower to its destination (if possible).
 *
 *	Output:	True if he moved.
 */

bool Party_manager::step
	(
	Actor *npc,
	Actor *leader,			// Who NPC is following.
	int dir,			// Direction we're walking (0-7).
	Tile_coord dest			// Destination tile.
	)
	{
	Tile_coord pos = npc->get_tile();	// Current position.
	Tile_coord to = Get_step_tile(pos, dest, dir);
	if (to.tx == pos.tx && to.ty == pos.ty)
		return false;		// Not moving.
	Frames_sequence *frames = npc->get_frames(dir);
	int& step_index = npc->get_step_index();
	if (!step_index)		// First time?  Init.
		step_index = frames->find_unrotated(npc->get_framenum());
					// Get next (updates step_index).
	int frame = frames->get_next(step_index);
	int cost = Get_cost(npc, leader, to);
					// Want delta-z <= 1.
	if (cost <= 3 && npc->step(to, frame))
		{
		if (to.tx != dest.tx || to.ty != dest.ty)
			{		// Take a 2nd step (w/ same frame).
			to = Get_step_tile(npc->get_tile(), dest, dir);
			npc->step(to, frame);
			}
		return true;		// Moved.
		}
	int destdir = npc->get_direction(dest);
	int cost0, cost1;
					// Try next/prev dir.
#if 0
	if (Take_better_step(npc, leader, pos, frame, destdir+1, destdir+7))
		return true;
	if (Take_better_step(npc, leader, pos, frame, destdir+2, destdir+6))
		return true;
	if (Take_better_step(npc, leader, pos, frame, destdir+3, destdir+5))
		return true;
#else
	if (Take_best_step(npc, leader, pos, frame, destdir))
		return true;
#endif
	frames->decrement(step_index);	// We didn't take the step.
	return false;
	}

