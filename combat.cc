/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Combat.cc - Combat scheduling.
 **
 **	Written: 6/20/2000 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include "combat.h"
#include "gamewin.h"
#include "actors.h"
#include "usecode.h"
#include "paths.h"
#include "Astar.h"
#include "actions.h"
#include "items.h"

#if 0
/*
 *	Find monsters in given chunk.
 */

void Combat_schedule::find_monsters
	(
	Chunk_object_list *chunk
	)
	{
	const int maxdist = 20;		// Max distance in tiles.
	for (Npc_actor *each = chunk->get_npcs(); each; 
						each = each->get_next())
		if (npc->distance(each) < maxdist && each->is_monster())
			opponents.append(each);
	}
#endif

/*
 *	Find nearby opponents in the 9 surrounding chunks.
 */

void Combat_schedule::find_opponents
	(
	)
	{
	opponents.clear();
	Game_window *gwin = Game_window::get_game_window();
	if (npc->is_monster())		// Monster?  Return party.
		{
		if (gwin->get_main_actor())
			opponents.append(gwin->get_main_actor());
		Usecode_machine *uc = gwin->get_usecode();
		int cnt = uc->get_party_count();
		for (int i = 0; i < cnt; i++)
			{
			Actor *npc = gwin->get_npc(uc->get_party_member(i));
			if (npc)
				opponents.append(npc);
			}
		return;
		}
	Slist nearby;			// Get all nearby NPC's.
	gwin->get_nearby_npcs(nearby);
	Actor *actor;
	Slist_iterator next(nearby);
	while ((actor = (Actor *) next()) != 0)
		if (actor->is_monster() && !actor->is_dead_npc())
			opponents.append(actor);
#if 0
	//+++++Switch to using gwin->get_nearby_npcs(opponents);
					// Get top-left chunk.
	int startcx = npc->get_cx() - 1, startcy = npc->get_cy() - 1;
	if (startcx < 0)
		startcx = 0;
	if (startcy < 0)
		startcy = 0;
					// Get bottom-right.
	int endcx = npc->get_cx() + 1, endcy = npc->get_cy() + 1;
	if (endcx >= num_chunks)
		endcx = num_chunks - 1;
	if (endcy >= num_chunks)
		endcy = num_chunks - 1;
	for (int cy = startcy; cy <= endcy; cy++)
		for (int cx = startcx; cx <= endcx; cx++)
			find_monsters(gwin->get_objects(cx, cy));
#endif
	}		

/*
 *	Find a foe.
 *
 *	Output:	Opponent that was found.
 */

Actor *Combat_schedule::find_foe
	(
	int mode			// Mode to use.
	)
	{
	cout << "'" << npc->get_name() << "' is looking for a foe" << endl;
	Actor *opp;
	if (!opponents.get_first())	// No more from last scan?
		find_opponents();	// Find all nearby.
	Actor *new_opponent = 0;
	Slist_iterator next(opponents);	// For going through list.
	switch ((Actor::Attack_mode) mode)
		{
		case Actor::weakest:
			{
			int str, least_str = 100;
			while ((opp = (Actor *) next()) != 0)
				{
				str = opp->get_property(Actor::strength);
				if (str < least_str)
					{
					least_str = str;
					new_opponent = opp;
					}
				}
			break;
			}
		case Actor::strongest:
			{
			int str, best_str = -100;
			while ((opp = (Actor *) next()) != 0)
				{
				str = opp->get_property(Actor::strength);
				if (str > best_str)
					{
					best_str = str;
					new_opponent = opp;
					}
				}
			break;
			}
		case Actor::nearest:
			{
			int dist, best_dist = 4*tiles_per_chunk;
			while ((opp = (Actor *) next()) != 0)
				if ((dist = npc->distance(opp)) < best_dist)
					{
					best_dist = dist;
					new_opponent = opp;
					}
			break;
			}
		case Actor::protect:		// ++++++For now, do random.
		case Actor::random:
		default:		// Default to random.
			new_opponent = (Actor *) opponents.get_first();
			break;
		}
	if (new_opponent)
		opponents.remove(new_opponent);
	return new_opponent;
	}

/*
 *	Find a foe.
 *
 *	Output:	Opponent that was found.
 */

inline Actor *Combat_schedule::find_foe
	(
	)
	{
	return find_foe((int) npc->get_attack_mode());
	}

/*
 *	Handle the 'approach' state.
 */

void Combat_schedule::approach_foe
	(
	)
	{
					// Find opponent.
	if (!opponent && !(opponent = find_foe()))
		{
		failures = 100;
		return;			// No one left to fight.
		}
	Actor::Attack_mode mode = npc->get_attack_mode();
	Tile_coord pos = npc->get_abs_tile_coord();
					// Time to run?
	if (mode == Actor::flee || 
	    (mode != Actor::beserk && npc->get_property(Actor::health) < 3))
		{
		int rx = rand();	// Get random position away from here.
		int ry = rand();
		int dirx = 2*(rx%2) - 1;// Get 1 or -1.
		int diry = 2*(ry%2) - 1;
		pos.tx += dirx*(8 + rx%8);
		pos.ty += diry*(8 + ry%8);
		npc->walk_to_tile(pos, 100, 0);
		return;
		}
	// +++++npc->get_weapon_range(mindist, max_reach);
	PathFinder *path = new Astar();
	Fast_pathfinder_client cost(max_reach);
	if (!path->NewPath(pos, opponent->get_abs_tile_coord(), &cost))
		{			// Failed?  Try nearest opponent.
		failures++;
		opponent = find_foe(Actor::nearest);
		if (!opponent || !path->NewPath(
				pos, opponent->get_abs_tile_coord(), &cost))
			{
			delete path;	// Really failed.  Try again in .5 sec.
			npc->start(200, 500);
			failures++;
			return;
			}
		}
	failures = 0;			// Clear count.  We succeeded.
	cout << npc->get_name() << " is pursuing " << opponent->get_name() <<
		endl;
	if (!yelled++ &&		// First time (or 256th)?
	    !npc->is_monster())
		npc->say(first_to_battle, last_to_battle);
					// Walk there, but don't retry if
					//   blocked.
	npc->set_action(new Path_walking_actor_action(path, 0));
	npc->start(200, 0);		// Start walking.
	}

/*
 *	Begin a strike at the opponent.
 */

void Combat_schedule::start_strike
	(
	)
	{
	state = strike;
	cout << npc->get_name() << " attacks " << opponent->get_name() << endl;
	int dir = npc->get_direction(opponent);
	char frames[12];		// Get frames to show.
	int cnt = npc->get_attack_frames(dir, frames);
	npc->set_action(new Frames_actor_action(frames, cnt));
	npc->start();			// Get back into time queue.
	}

/*
 *	See if we need a new opponent.
 */

inline int Need_new_opponent
	(
	Game_window *gwin,
	Game_object *opponent
	)
	{
					// Nonexistent or dead?
	if (!opponent || opponent->is_dead_npc())
		return 1;
					// See if off screen.
	Tile_coord t = opponent->get_abs_tile_coord();
	Rectangle screen = gwin->get_win_tile_rect().enlarge(2);
	return (!screen.has_point(t.tx, t.ty));
	}

/*
 *	Previous action is finished.
 */

void Combat_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (npc->get_attack_mode() == Actor::manual)
		return;
					// Check if opponent still breathes.
	if (Need_new_opponent(gwin, opponent))
		{
		opponent = 0;
		state = approach;
		}
	switch (state)			// Note:  state's action has finished.
		{
	case approach:
		if (opponent && npc->distance(opponent) <= max_reach)
					// Close enough.  ++++Need to parry??
			start_strike();	// Start strike animation, for now.
		else
			approach_foe();
		break;
	case strike:			// He hasn't moved away?
		if (npc->distance(opponent) <= max_reach)
			{
			int dir = npc->get_direction(opponent);
			opponent->attacked(npc);
			gwin->add_dirty(npc);
			npc->set_frame(npc->get_dir_framenum(dir,
							Actor::standing));
			gwin->add_dirty(npc);
			}
		state = approach;
		npc->start(200);	// Back into queue.
		break;
	default:
		break;
		}
	if (failures > 5)
		{			// Too many failures.  Give up for now.
		cout << npc->get_name() << " is giving up" << endl;
		if (npc->get_party_id() >= 0)
			{		// Party member.
			npc->walk_to_tile(
				gwin->get_main_actor()->get_abs_tile_coord());
					// WARNING:  Destroys ourself.
			npc->set_schedule_type(Schedule::follow_avatar);
			}
		else if (npc->get_alignment() == Npc_actor::friendly &&
						prev_schedule != combat)
			npc->set_schedule_type(prev_schedule);
		}
	}

/*
 *	Npc just went dormant (probably off-screen).
 */

void Combat_schedule::im_dormant
	(
	)
	{
	if (npc->get_alignment() == Npc_actor::friendly && 
			prev_schedule != combat && npc->is_monster())
					// Friendly, so end combat.
		npc->set_schedule_type(prev_schedule);
	}

/*
 *	Set opponent.  (Gets called when you double-click on one.)
 */

void Combat_schedule::set_opponent
	(
	Game_object *obj		// Could be a door, too.
	)
	{
	opponent = obj;
	state = approach;
	}

