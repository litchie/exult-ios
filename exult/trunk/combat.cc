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
#include "paths.h"
#include "Astar.h"
#include "actions.h"
#include "items.h"
#include "effects.h"
#include "Audio.h"
#include "ready.h"

unsigned long Combat_schedule::battle_time = 0;

/*
 *	Find nearby opponents in the 9 surrounding chunks.
 */

void Combat_schedule::find_opponents
	(
	)
	{
	opponents.clear();
	Game_window *gwin = Game_window::get_game_window();
	if (npc->get_alignment() == Npc_actor::hostile)
		{
		Actor *party[9];
		int cnt = gwin->get_party(party, 1);
		for (int i = 0; i < cnt; i++)
					// But ignore invisible ones.
			if (!party[i]->get_flag(Actor::invisible))
				opponents.append(party[i]);
		return;
		}
	Slist nearby;			// Get all nearby NPC's.
	gwin->get_nearby_npcs(nearby);
	Actor *actor;
	Slist_iterator next(nearby);
	while ((actor = (Actor *) next()) != 0)
		if (actor->get_alignment() == Npc_actor::hostile &&
		    !actor->is_dead_npc())
			{
			opponents.append(actor);
					// And set hostile monsters.
			if (actor->get_alignment() >= Npc_actor::hostile &&
			    actor->get_schedule_type() != Schedule::combat)
				actor->set_schedule_type(Schedule::combat);
			}
					// None found?  Use Avatar's.
	if (!opponents.get_last() && npc->get_party_id() >= 0 &&
	    npc != gwin->get_main_actor())
		{
		Game_object *opp = gwin->get_main_actor()->get_opponent();
		if (opp && opp != npc && (opp->get_npc_num() > 0 ||
					opp->is_monster()))
			opponents.append(opp);
		}
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
		{
		opponents.remove(new_opponent);
		unsigned long curtime = SDL_GetTicks();
					// .5 minute since last start?
		if (!started_battle && curtime - battle_time >= 30000)
			{
			audio->start_music(rand()%2 ? ATTACKED1 : ATTACKED2,
									0);
			battle_time = curtime;
			}
		started_battle = 1;
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
					// Find opponent.
	if (!opponent && !(opponent = find_foe()))
		{
		failures++;
		npc->start(200, 200);	// Try again in 1/5 sec.
		return;			// No one left to fight.
		}
	Actor::Attack_mode mode = npc->get_attack_mode();
	Tile_coord pos = npc->get_abs_tile_coord();
					// Time to run?
	if (mode == Actor::flee || 
	    (mode != Actor::beserk && 
		npc != Game_window::get_game_window()->get_main_actor() &&
					npc->get_property(Actor::health) < 3))
		{
		if (npc->get_party_id() >= 0 && !fleed)
			{
			fleed = 1;
			audio->start_music(RUN_AWAY, 0);
			}
		int rx = rand();	// Get random position away from here.
		int ry = rand();
		int dirx = 2*(rx%2) - 1;// Get 1 or -1.
		int diry = 2*(ry%2) - 1;
		pos.tx += dirx*(8 + rx%8);
		pos.ty += diry*(8 + ry%8);
		npc->walk_to_tile(pos, 100, 0);
		return;
		}
	PathFinder *path = new Astar();
					// Try this for now:
	Monster_pathfinder_client cost(npc, max_reach, opponent);
	if (!path->NewPath(pos, opponent->get_abs_tile_coord(), &cost))
		{			// Failed?  Try nearest opponent.
		failures++;
		opponent = find_foe(Actor::nearest);
		Monster_pathfinder_client cost(npc, max_reach, opponent);
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
 *	Swap weapon with the one in the belt.
 *
 *	Output:	1 if successful.
 */

static int Swap_weapons
	(
	Actor *npc
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Game_object *bobj = npc->get_readied(Actor::belt);
	if (!bobj)
		return 0;
	Shape_info& info = gwin->get_info(bobj);
	Weapon_info *winf = info.get_weapon_info();
	if (!winf)
		return 0;		// Not a weapon.
	int ammo = winf->get_ammo();
	if (ammo && !npc->find_item(ammo, -359, -359))
		return 0;		// No ammo.
	if (info.get_ready_type() == two_handed_weapon &&
	    npc->get_readied(Actor::rhand) != 0)
		return 0;		// Needs two free hands.
	Game_object *oldweap = npc->get_readied(Actor::lhand);
	if (oldweap)
		npc->remove(oldweap);
	npc->remove(bobj);
	npc->add(bobj, 1);		// Should go into weapon hand.
	if (oldweap)
		npc->add(oldweap, 1);	
	return 1;
	}

/*
 *	Begin a strike at the opponent.
 */

void Combat_schedule::start_strike
	(
	)
	{
	if (ammo_shape)			// Firing?
		{
		if (ammo_consumed && !npc->find_item(ammo_shape, -359, -359))
			{		// Out of ammo.
			if (Swap_weapons(npc))
				set_weapon_info();
			state = approach;
			npc->start(200, 500);
			return;
			}
		Tile_coord pos = npc->get_abs_tile_coord();
		if (!Fast_pathfinder_client::is_straight_path(pos,
					opponent->get_abs_tile_coord()))
			{		// Blocked.  Find another spot.
			pos.tx += rand()%7 - 3;
			pos.ty += rand()%7 - 3;
			npc->walk_to_tile(pos, 100, 0);
			state = approach;
			return;
			}
		state = fire;		// Clear to go.
		}
	else
		state = strike;
	cout << npc->get_name() << " attacks " << opponent->get_name() << endl;
	int dir = npc->get_direction(opponent);
	char frames[12];		// Get frames to show.
	int cnt = npc->get_attack_frames(dir, frames);
	npc->set_action(new Frames_actor_action(frames, cnt));
	npc->start();			// Get back into time queue.
					// Have them attack back.
	Actor *opp = dynamic_cast<Actor *> (opponent);
					// But only if it's a monster.
	if (opp && !opp->get_opponent() && opp->is_monster())
		opp->set_opponent(npc);
	}

/*
 *	Set weapon 'max_reach' and 'ammo'.
 */

void Combat_schedule::set_weapon_info
	(
	)
	{
	int points;
	Weapon_info *info = npc->get_weapon(points, weapon_shape);
					// No ammo. required?
	if (!info || !(ammo_shape = info->get_ammo()))
		max_reach = 1;		// For now.
	else
		max_reach = 20;		// Guessing.
	ammo_consumed = (info != 0 && info->is_ammo_consumed());
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
	if (!opponent || opponent->is_dead_npc() ||
					// Or invisible?
	    opponent->get_flag(Game_object::invisible))
		return 1;
					// See if off screen.
	Tile_coord t = opponent->get_abs_tile_coord();
	Rectangle screen = gwin->get_win_tile_rect().enlarge(2);
	return (!screen.has_point(t.tx, t.ty));
	}

/*
 *	Get rectangle in tiles.
 */

inline Rectangle Get_tiles
	(
	Game_object *obj
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(obj);
					// Get lower-right corner pos.
	Tile_coord pos = obj->get_abs_tile_coord();
	int w = info.get_3d_xtiles(), h = info.get_3d_ytiles();
	return Rectangle(pos.tx - w + 1, pos.ty - h + 1, w, h);
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
	if (npc->get_flag(Actor::asleep))
		{
		npc->start(200, 1000);	// Check again in a second.
		return;
		}
					// Check if opponent still breathes.
	if (Need_new_opponent(gwin, opponent))
		{
		opponent = 0;
		if (state != initial)
			state = approach;
		}
	switch (state)			// Note:  state's action has finished.
		{
	case initial:			// Way far away (50 tiles)?
		if (npc->distance(gwin->get_main_actor()) > 50)
			return;		// Just go dormant.
		state = approach;	// FALL THROUGH.
	case approach:
		if (opponent && Get_tiles(npc).enlarge(max_reach).intersects(
						Get_tiles(opponent)))
					// Close enough.  ++++Need to parry??
			start_strike();	// Start strike animation, for now.
		else
			approach_foe();
		break;
	case strike:			// He hasn't moved away?
		if (Get_tiles(npc).enlarge(max_reach).intersects(
						Get_tiles(opponent)))
			{
			int dir = npc->get_direction(opponent);
			opponent = opponent->attacked(npc);
			gwin->add_dirty(npc);
			npc->set_frame(npc->get_dir_framenum(dir,
							Actor::standing));
			gwin->add_dirty(npc);
			}
		state = approach;
		npc->start(200);	// Back into queue.
		break;
	case fire:			// Range weapon.
		if (!ammo_consumed ||
		    npc->remove_quantity(1, ammo_shape, -359, -359) == 0)
			gwin->add_effect(new Projectile_effect(npc, opponent,
						ammo_shape, weapon_shape));
		state = approach;
		npc->start(200);	// Back into queue.
		break;
	default:
		break;
		}
	if (failures > 5 && npc != gwin->get_main_actor())
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
		prev_schedule != npc->get_schedule_type() && npc->is_monster())
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

/*
 *	Get opponent.
 */

Game_object *Combat_schedule::get_opponent
	(
	)
	{
	return opponent;
	}

/*
 *	Create duel schedule.
 */

Duel_schedule::Duel_schedule
	(
	Actor *n
	) : Combat_schedule(n, duel), start(n->get_abs_tile_coord()),
		attacks(0)
	{
	started_battle = 1;		// Avoid playing music.
	}

/*
 *	Find dueling opponents.
 */

void Duel_schedule::find_opponents
	(
	)
	{
	opponents.clear();
	Vector vec;			// Find all nearby NPC's.
	int cnt = npc->find_nearby(vec, -359, 24, 8);
	for (int i = 0; i < cnt; i++)
		{
		Actor *opp = (Actor *) vec.get(i);
		Game_object *oppopp = opp->get_opponent();
		if (opp != npc && opp->get_schedule_type() == duel &&
		    (!oppopp || oppopp == npc))
			if (rand()%2)
				opponents.append(opp);
			else
				opponents.insert(opp);
		}
	}

/*
 *	Previous action is finished.
 */

void Duel_schedule::now_what
	(
	)
	{
	if (state == strike || state == fire)
		attacks++;
	else
		{
		Combat_schedule::now_what();
		return;
		}
	if (attacks%8 == 0)		// Time to break off.
		{
		opponent = 0;
		Tile_coord pos = start;
		pos.tx += rand()%24 - 12;
		pos.ty += rand()%24 - 12;
		Tile_coord dest(-1, -1, -1);	// Find a free spot.
		for (int i = 0; i < 4 && dest.tx == -1; i++)
			dest = npc->find_unblocked_tile(pos, i, 4);
		if (dest.tx == -1 || 
			!npc->walk_path_to_tile(dest, 250, rand()%2000))
					// Failed?  Try again a little later.
			npc->start(250, rand()%3000);
		}
	else
		Combat_schedule::now_what();
	}
