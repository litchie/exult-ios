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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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

using std::cout;
using std::endl;
using std::rand;

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
	if (npc->get_alignment() >= Npc_actor::hostile)
	{
		Actor *party[9];
		int cnt = gwin->get_party(party, 1);
		for (int i = 0; i < cnt; i++)
					// But ignore invisible ones.
			if (!party[i]->get_flag(Obj_flags::invisible))
				opponents.push(party[i]);
		return;
	}
	Actor_queue nearby;			// Get all nearby NPC's.
	gwin->get_nearby_npcs(nearby);
	for (Actor_queue::const_iterator it = nearby.begin(); it != nearby.end(); ++it)
	{
		Actor *actor = *it;
		if (actor->get_alignment() >= Npc_actor::hostile &&
		    !actor->is_dead_npc())
		{
			opponents.push(actor);
					// And set hostile monsters.
			if (actor->get_alignment() >= Npc_actor::hostile &&
			    actor->get_schedule_type() != Schedule::combat)
				actor->set_schedule_type(Schedule::combat);
		}
	}
/*
	Slist_iterator next(nearby);
	while ((actor = (Actor *) next()) != 0)
		if (actor->get_alignment() >= Npc_actor::hostile &&
		    !actor->is_dead_npc())
		{
			opponents.push(actor);
					// And set hostile monsters.
			if (actor->get_alignment() >= Npc_actor::hostile &&
			    actor->get_schedule_type() != Schedule::combat)
				actor->set_schedule_type(Schedule::combat);
		}
*/
					// None found?  Use Avatar's.
	if (opponents.empty() && npc->get_party_id() >= 0 &&
	    npc != gwin->get_main_actor())
	{
		Game_object *opp = gwin->get_main_actor()->get_opponent();
		if (opp && opp != npc && (opp->get_npc_num() > 0 ||
					opp->is_monster()))
			opponents.push((Actor *)opp);
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
	if (opponents.empty())	// No more from last scan?
		find_opponents();	// Find all nearby.
	Actor *new_opponent = 0;
//	Slist_iterator next(opponents);	// For going through list.
	switch ((Actor::Attack_mode) mode)
	{
		case Actor::weakest:
		{
			int str, least_str = 100;
			for (Actor_queue::const_iterator it = opponents.begin(); it != opponents.end(); ++it)
			{
				Actor *opp = *it;
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
			for (Actor_queue::const_iterator it = opponents.begin(); it != opponents.end(); ++it)
			{
				Actor *opp = *it;
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
			int dist, best_dist = 4*c_tiles_per_chunk;
			for (Actor_queue::const_iterator it = opponents.begin(); it != opponents.end(); ++it)
			{
				Actor *opp = *it;
				if ((dist = npc->distance(opp)) < best_dist)
				{
					best_dist = dist;
					new_opponent = opp;
				}
			}
			break;
		}
		case Actor::protect:		// ++++++For now, do random.
		case Actor::random:
		default:		// Default to random.
			new_opponent = opponents.empty() ? 0 : opponents.front();
			break;
	}
	if (new_opponent)
	{
		opponents.remove(new_opponent);
		unsigned long curtime = SDL_GetTicks();
					// .5 minute since last start?
		if (!started_battle && curtime - battle_time >= 30000)
		{
			Audio::get_ptr()->start_music_combat(rand()%2 ? CSAttacked1 : CSAttacked2,
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
			Audio::get_ptr()->start_music_combat(CSRun_Away, 0);
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
	Monster_pathfinder_client cost(npc, max_range, opponent);
	if (!path->NewPath(pos, opponent->get_abs_tile_coord(), &cost))
		{			// Failed?  Try nearest opponent.
		failures++;
		opponent = find_foe(Actor::nearest);
		Monster_pathfinder_client cost(npc, max_range, opponent);
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
	int ammo = winf->get_ammo_consumed();
	if (ammo)			// Check for readied ammo.
		{
		Game_object *aobj = npc->get_readied(Actor::ammo);
		if (!aobj || !Ammo_info::is_in_family(aobj->get_shapenum(),
								ammo))
			return 0;
		}
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
	Rectangle& npcrect, 
	Rectangle& opprect	// Npc, opponent rectangles.
	)
	{
					// Close enough to strike?
	if (strike_range && (!projectile_range ||
		npcrect.enlarge(strike_range).intersects(opprect)))
		state = strike;
	else
		{
		Game_object *aobj;
		if (ammo_shape &&
		    (!(aobj = npc->get_readied(Actor::ammo)) ||
			!Ammo_info::is_in_family(aobj->get_shapenum(), 
								ammo_shape)))
			{		// Out of ammo.
			if (Swap_weapons(npc))
				set_weapon_info();
			state = approach;
			npc->start(200, 500);
			return;
			}
		Rectangle foot = npc->get_footprint();
		Tile_coord pos = npc->get_abs_tile_coord();
		Tile_coord opos = opponent->get_abs_tile_coord();
		if (opos.tx < pos.tx)	// Going left?
			pos.tx = foot.x;
		if (opos.ty < pos.ty)	// Going north?
			pos.ty = foot.y;
		if (!Fast_pathfinder_client::is_straight_path(pos, opos))
			{		// Blocked.  Find another spot.
			pos.tx += rand()%7 - 3;
			pos.ty += rand()%7 - 3;
			npc->walk_to_tile(pos, 100, 0);
			state = approach;
			return;
			}
		state = fire;		// Clear to go.
		}
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
 *	Set weapon 'max_range' and 'ammo'.
 */

void Combat_schedule::set_weapon_info
	(
	)
	{
	int points;
	Weapon_info *info = npc->get_weapon(points, weapon_shape);
	if (!info)
		{
		projectile_shape = ammo_shape = 0;
		projectile_range = 0;
		strike_range = 1;	// Can always bite.
		is_thrown = false;
		}
	else
		{
		projectile_shape = info->get_projectile();
		ammo_shape = info->get_ammo_consumed();
		strike_range = info->get_striking_range();
		projectile_range = info->get_projectile_range();
		is_thrown = info->is_thrown();
		}
	max_range = projectile_range > strike_range ? projectile_range
					: strike_range;
#if 0
					// Not shooting?
	if (!projectile_shape)
		max_range = 1;		// For now.
	else
		max_range = 20;		// Guessing.
#endif
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
	    opponent->get_flag(Obj_flags::invisible))
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
 *	Use one unit of ammo.
 *
 *	Output:	Actual ammo shape.
 *		0 if failed.
 */

static int Use_ammo
	(
	Actor *npc,
	int ammo,			// Ammo family shape.
	int proj			// Projectile shape.
	)
	{
	Game_object *aobj = npc->get_readied(Actor::ammo);
	if (!aobj)
		return 0;
	int actual_ammo = aobj->get_shapenum();
	if (!Ammo_info::is_in_family(actual_ammo, ammo))
		return 0;
	npc->remove(aobj);		// Remove all.
	int quant = aobj->get_quantity();
	aobj->modify_quantity(-1);	// Reduce amount.
	if (quant > 1)			// Still some left?  Put back.
		npc->add_readied(aobj, Actor::ammo);
					// Use actual shape unless a different
					//   projectile was specified.
	return ammo == proj ? actual_ammo : proj;
	}

/*
 *	Does this weapon come back?
 */

static bool Boomerangs
	(
	int shapenum
	)
	{
	if (shapenum == 552 ||		// Magic axe.
	    shapenum == 555 ||		// Hawk.
	    shapenum == 605 ||		// Boomerang.
	    shapenum == 557)		// Juggernaut hammer.
		return true;
	else
		return false;
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
	if (npc->get_flag(Obj_flags::asleep))
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
		if (opponent)
			{
			Rectangle npctiles = Get_tiles(npc),
				  opptiles = Get_tiles(opponent);
			Rectangle wtiles = npctiles;
			if (wtiles.enlarge(max_range).intersects(opptiles))
				{	// Close enough.  ++++Need to parry??
					// Start strike animation.
				start_strike(npctiles, opptiles);	
				break;
				}
			}
		approach_foe();
		break;
	case strike:			// He hasn't moved away?
		if (Get_tiles(npc).enlarge(strike_range).intersects(
						Get_tiles(opponent)))
			{
			int dir = npc->get_direction(opponent);
			opponent = opponent->attacked(npc);
			npc->add_dirty(gwin);
			npc->set_frame(npc->get_dir_framenum(dir,
							Actor::standing));
			npc->add_dirty(gwin, 1);
					// Glass sword?  Only 1 use.
			if (weapon_shape == 604)
				{
				npc->remove_quantity(1, weapon_shape,
						c_any_qual, c_any_framenum);
				set_weapon_info();
				}
			}
		state = approach;
		npc->start(200);	// Back into queue.
		break;
	case fire:			// Range weapon.
		{
		int ashape = 0;
		if (is_thrown)		// Throwing the weapon?
			{
			if (Boomerangs(weapon_shape))
				ashape = weapon_shape;
			else if (npc->remove_quantity(1, weapon_shape,
					c_any_qual, c_any_framenum) == 0)
				{
				ashape = weapon_shape;
				set_weapon_info();
				}
			}
		else			// Ammo required?
			ashape = ammo_shape ? Use_ammo(
					npc, ammo_shape, projectile_shape)
				: (projectile_shape ? projectile_shape
					: weapon_shape);
		if (ashape > 0)
			gwin->add_effect(new Projectile_effect(npc, opponent,
				ashape, weapon_shape));
		state = approach;
		npc->start(200);	// Back into queue.
		break;
		}
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
	Actor_vector vec;			// Find all nearby NPC's.
	npc->find_nearby_actors(vec, c_any_shapenum, 24);
	for (Actor_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Actor *opp = *it;
		Game_object *oppopp = opp->get_opponent();
		if (opp != npc && opp->get_schedule_type() == duel &&
		    (!oppopp || oppopp == npc))
			if (rand()%2)
				opponents.push(opp);
			else
				opponents.push_front(opp);
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
