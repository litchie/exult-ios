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

/*
 *	Find monsters in given chunk.
 */

void Combat_schedule::find_monsters
	(
	Chunk_object_list *chunk,
	Vector& vec			// Returned here.
	)
	{
	for (Npc_actor *npc = chunk->get_npcs(); npc; npc = npc->get_next())
		if (npc->is_monster())
			vec.append(npc);
	}


/*
 *	Find nearby opponents in the 9 surrounding chunks.
 */

void Combat_schedule::find_opponents
	(
	Vector& vec			// Returned here.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (npc->is_monster())		// Monster?  Return party.
		{
		vec.append(gwin->get_main_actor());
		Usecode_machine *uc = gwin->get_usecode();
		int cnt = uc->get_party_count();
		for (int i = 0; i < cnt; i++)
			vec.append(gwin->get_npc(uc->get_party_member(i)));
		return;
		}
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
			find_monsters(gwin->get_objects(cx, cy), vec);
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
	Vector vec(20);
	find_opponents(vec);		// Find all nearby.
	int cnt = vec.get_cnt();
	Actor *new_opponent = 0;
	switch ((Actor::Attack_mode) mode)
		{
		case Actor::weakest:
			{
			int str, least_str = 100;
			for (int i = 0; i < cnt; i++)
				{
				Actor *opp = (Actor *) vec.get(i);
				str = opp->get_property(Actor::strength);
				if (str < least_str)
					{
					least_str = str;
					new_opponent = (Actor *) vec.get(i);
					}
				}
			break;
			}
		case Actor::strongest:
			{
			int str, best_str = -100;
			for (int i = 0; i < cnt; i++)
				{
				Actor *opp = (Actor *) vec.get(i);
				str = opp->get_property(Actor::strength);
				if (str > best_str)
					{
					best_str = str;
					new_opponent = (Actor *) vec.get(i);
					}
				}
			break;
			}
		case Actor::nearest:
			{
			int dist, best_dist = 4*tiles_per_chunk;
			for (int i = 0; i < cnt; i++)
				if ((dist = npc->distance(
					(Actor *) vec.get(i))) < best_dist)
					{
					best_dist = dist;
					new_opponent = (Actor *) vec.get(i);
					}
			break;
			}
		case Actor::protect:		// ++++++For now, do random.
		case Actor::random:
		default:		// Default to random.
			new_opponent = (Actor *) vec.get(rand()%cnt);
			break;
		}
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
		npc->walk_path_to_tile(pos, 100, 0);
		return;
		}
	if (!opponent && !(opponent = find_foe()))
		return;			// No one left to fight.
	// +++++npc->get_weapon_range(mindist, max_reach);
	PathFinder *path = new Astar();
	Fast_pathfinder_client cost(max_reach);
	if (!path->NewPath(pos, opponent->get_abs_tile_coord(), &cost))
		{			// Failed?  Try nearest opponent.
		opponent = find_foe(Actor::nearest);
		if (!opponent || !path->NewPath(
				pos, opponent->get_abs_tile_coord(), &cost))
			{
			delete path;	// Really failed.  Try again in .5 sec.
			npc->start(200, 500);
			return;
			}
		}
					// Walk there, but don't retry if
					//   blocked.
	npc->set_action(new Path_walking_actor_action(path, 0));
	npc->start(200, 0);		// Start walking.
	}

/*
 *	Previous action is finished.
 */

void Combat_schedule::now_what
	(
	)
	{
	if (npc->get_attack_mode() == Actor::manual)
		return;
	if (opponent)			// Check if opponent still breathes.
		if (opponent->is_dead_npc())
			{
			opponent = 0;
			state = approach;
			}
		else if (npc->distance(opponent) <= max_reach)
			{		// For now, let's just strike.
//+++++Want to animate.			state = strike;
			cout << npc->get_name() << " attacks " <<
					opponent->get_name() << endl;
			opponent->attacked(npc);	//++++For now.
			npc->start(200, 500);	//+++++
			return;			//+++++
			}
	switch (state)
		{
	case approach:
		approach_foe();
		break;
	default:
		break;
		}
	}
