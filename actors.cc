/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actors.cc - Game actors.
 **
 **	Written: 11/3/98 - JSF
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


#include "alpha_kludges.h"

#ifndef ALPHA_LINUX_CXX
#  include <iostream>			/* Debugging. */
#  include <cstdlib>
#  include <cstring>
#endif
#include <algorithm>		/* swap. */
#include "gamewin.h"
#include "actors.h"
#include "imagewin.h"
#include "ucmachine.h"
#include "actions.h"
#include "ready.h"
#include "combat.h"
#include "Zombie.h"
#include "Astar.h"
#include "dir.h"
#include "items.h"
#include "egg.h"
#include "chunks.h"
#include "bodies.h"
#include "Audio.h"
#include "npctime.h"
#include "game.h"
#include "cheat.h"
#include "frameseq.h"
#include "Paperdoll_gump.h"

using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::rand;
using std::string;
using std::swap;

// Party positions
// Direction, Party num, xy (tile) from leader
//
// Please Don't Touch - Colourless
//
const short Actor::party_pos[4][10][2] = {
		// North Facing
	{
		{ -2, 2 },
		{ 2, 2 },
		{ 0, 4 },
		{ -4, 4 },
		{ 4, 4 },
		{ -2, 6 },
		{ 2, 6 },
		{ 0, 8 },
		{ -4, 8 },
		{ 4, 8 }
	},
		// East Facing,
	{
		{ -2, -2 },
		{ -2, 2 },
		{ -4, 0 },
		{ -4, -4 },
		{ -4, 4 },
		{ -6, -2 },
		{ -6, 2 },
		{ -8, 0 },
		{ -8, -4 },
		{ -8, 4 }
	},
		// South Facing
	{
		{ -2, -2 },
		{ 2, -2 },
		{ 0, -4 },
		{ -4, -4 },
		{ 4, -4 },
		{ -2, -6 },
		{ 2, -6 },
		{ 0, -8 },
		{ -4, -8 },
		{ 4, -8 }
	},
		// West Facing
	{
		{ 2, -2 },
		{ 2, 2 },
		{ 4, 0 },
		{ 4, -4 },
		{ 4, 4 },
		{ 6, -2 },
		{ 6, 2 },
		{ 8, 0 },
		{ 8, -4 },
		{ 8, 4 }
	}
};

Frames_sequence *Actor::frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const char Actor::attack_frames1[4] = {3, 4, 5, 6};
const char Actor::attack_frames2[4] = {3, 7, 8, 9};
Dead_body *Dead_body::in_world = 0;
Equip_record *Monster_info::equip = 0;
int Monster_info::equip_cnt = 0;
Monster_actor *Monster_actor::in_world = 0;
int Monster_actor::in_world_cnt = 0;

extern Cheat cheat;

/*
 *	Get/create timers.
 */

Npc_timer_list *Actor::need_timers
	(
	)
	{
	if (!timers)
		timers = new Npc_timer_list(this);
	return timers;
	}

/*
 *	Initialize.
 */

void Actor::init
	(
	)
	{
	if (!frames[(int) north])
		set_default_frames();
	for (size_t i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		properties[i] = 0;
	for (size_t i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		spots[i] = 0;
	}

/*
 *	Ready ammo for weapon being carried.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::ready_ammo
	(
	)
	{
	int points;
	Weapon_info *winf = Actor::get_weapon(points);
	int ammo;
	if (!winf || (ammo = winf->get_ammo()) == 0)
		return 0;		// No weapon, or ammo not needed.
					// See if already have ammo.
	Game_object *aobj = get_readied(Actor::ammo);
	if (aobj && Ammo_info::is_in_family(aobj->get_shapenum(), ammo))
		return 1;		// Already readied.
	Game_object_vector vec(50);		// Get list of all possessions.
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	Game_object *found = 0;
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		if (Ammo_info::is_in_family(obj->get_shapenum(), ammo))
			found = obj;
		}
	if (!found)
		return 0;
	if (aobj)			// Something there already?
		aobj->remove_this(1);	// Remove it.
	found->remove_this(1);
	add(found, 1);			// Should go to the right place.
	if (aobj)			// Put back old ammo.
		add(aobj, 1);
	return 1;
	}

/*
 *	If no weapon readied, look through all possessions for the best one.
 */

void Actor::ready_best_weapon
	(
	)
	{
	int points;
	if (Actor::get_weapon(points) != 0)
		{
		ready_ammo();
		return;			// Already have one.
		}
	Game_window *gwin = Game_window::get_game_window();
	Game_object_vector vec(50);		// Get list of all possessions.
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	Game_object *best = 0;
	int best_damage = -20;
	Ready_type wtype=other;
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *obj = *it;
		if (obj->get_shapenum() == 595)
			continue;	// Don't pick the torch. (Don't under-
					//   stand weapons.dat well, yet!)
		Shape_info& info = gwin->get_info(obj);
		Weapon_info *winf = info.get_weapon_info();
		if (!winf)
			continue;	// Not a weapon.
					// +++Might be a class to check.
		int damage = winf->get_damage();
		if (damage > best_damage)
			{
			wtype = (Ready_type) info.get_ready_type();
			best = obj;
			best_damage = damage;
			}
		}
	if (!best)
		return;
	Game_object *remove1 = 0, *remove2 = 0;
	if (wtype == two_handed_weapon)
		{
		remove1 = spots[lhand];
		remove2 = spots[rhand];
		}
	else
		if (free_hand() == -1)
			remove1 = spots[lhand];
					// Free the spot(s).
	if (remove1)
		remove1->remove_this(1);
	if (remove2)
		remove2->remove_this(1);
	best->remove_this(1);
	add(best, 1);			// Should go to the right place.
	if (remove1)			// Put back other things.
		add(remove1, 1);
	if (remove2)
		add(remove2, 1);
	ready_ammo();			// Get appropriate ammo.
	}

/*
 *	Add dirty rectangle(s).
 *
 *	Output:	0 if not on screen.
 */
inline int Actor::add_dirty
	(
	Game_window *gwin,
	int figure_rect			// Recompute weapon rectangle.
	)
	{
	if (!gwin->add_dirty(this))
		return 0;
	int weapon_x, weapon_y, weapon_frame;
	if (figure_rect)
		if (figure_weapon_pos(weapon_x, weapon_y, weapon_frame))
			{
			Game_object * weapon = spots[lhand];
			int shnum = weapon->get_shapenum();
			Shape_frame *wshape = gwin->get_shape(
							shnum, weapon_frame);
					// Set dirty area rel. to NPC.
			weapon_rect = gwin->get_shape_rect(wshape, 
							weapon_x, weapon_y);
			}
		else
			weapon_rect.w = 0;
	if (weapon_rect.w > 0)		// Repaint weapon area too.
		{
		Rectangle r = weapon_rect;
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		r.shift(xoff, yoff);
		r.enlarge(4);
		gwin->add_dirty(gwin->clip_to_win(r));
		}
	return 1;
	}

/*
 *	Move an object, and possibly change its shape too.
 */
inline void Actor::movef
	(
	Chunk_object_list *old_chunk, 
	Chunk_object_list *new_chunk, 
	int new_sx, int new_sy, int new_frame, 
	int new_lift
	)
	{
	if (old_chunk)			// Remove from current chunk.
		old_chunk->remove(this);
	set_shape_pos(new_sx, new_sy);
	if (new_frame >= 0)
		set_frame(new_frame);
	if (new_lift >= 0)
		set_lift(new_lift);
	new_chunk->add(this);
	}

/*
 *	Create character.
 */

Actor::Actor
	(
	const std::string &nm, 
	int shapenum, 
	int num,			// NPC # from npc.dat.
	int uc				// Usecode #.
	) : Container_game_object(), name(nm),usecode(uc), 
	    npc_num(num), party_id(-1), attack_mode(nearest),
	    schedule_type((int) Schedule::loiter), schedule(0), dormant(true),
	    alignment(0),
	    two_handed(false), two_fingered(false), light_sources(0),
	    usecode_dir(0), siflags(0), type_flags(0), action(0), 
	    frame_time(0), next_path_time(0), timers(0),
	    weapon_rect(0, 0, 0, 0)
	{
	set_shape(shapenum, 0); 
	init();
	}

/*
 *	Delete.
 */

Actor::~Actor
	(
	)
	{
	delete action;
	delete timers;
	}

/*
 *	Decrement food level and print complaints if it gets too low.
 */

void Actor::use_food
	(
	)
	{
	int food = get_property((int) food_level);
	food--;
	set_property((int) food_level, food);
	if (food <= 0)			// Really low?
		{
		if (rand()%4)
			say(first_starving, first_starving + 2);
		if (food < 0)		// Set timer for damage.
			need_timers()->start_hunger();
		}
	else if (food <= 4)
		{
		if (rand()%3)
			say(first_needfood, first_needfood + 2);
		}
	else if (food <= 8)
		if (rand()%2)
			say(first_hunger, first_hunger + 2);
	}

/*
 *	Get sequence of frames for an attack.
 *
 *	Output:	# of frames stored.
 */

int Actor::get_attack_frames
	(
	int dir,			// 0-7 (as in dir.h).
	char *frames			// Frames stored here.
	) const
	{
	const char *which;
	int cnt;
	if (two_handed)
		{
		which = attack_frames2;
		cnt = sizeof(attack_frames2);
		}
	else
		{
		which = attack_frames1;
		cnt = sizeof(attack_frames1);
		}
					// Check for empty shape.
	Shape_frame *shape = Game_window::get_game_window()->get_shape(
					get_shapenum(), which[1]);
	if (!shape || shape->is_empty())
					// If empty, the other usually isn't.
		which = two_handed ? attack_frames1 : attack_frames2;
	for (int i = 0; i < cnt; i++)	// Copy frames with correct dir.
		*frames++ = get_dir_framenum(dir, *which++);
	return (cnt);
	}		

/*
 *	Set default set of frames.
 */

void Actor::set_default_frames
	(
	)
	{
					// Set up actor's frame lists.
					// These are rough guesses.
	static unsigned char 	north_frames[3] = {0, 1, 2},
				south_frames[3] = {16, 17, 18},
				east_frames[3] = {48, 49, 50},
				west_frames[3] = {32, 33, 34};
	frames[(int) north] = new Frames_sequence(3, north_frames);
	frames[(int) south] = new Frames_sequence(3, south_frames);
	frames[(int) east] = new Frames_sequence(3, east_frames);
	frames[(int) west] = new Frames_sequence(3, west_frames);
	}

/*
 *	Set new action.
 */

void Actor::set_action
	(
	Actor_action *newact
	)
	{
	if (newact != action)
		{
		delete action;
		action = newact;
		}
	if (!action)			// No action?  We're stopped.
		frame_time = 0;
	}

/*
 *	Get destination, or current spot if no destination.
 */

Tile_coord Actor::get_dest
	(
	)
	{
	Tile_coord dest;
	if (action && action->get_dest(dest))
		return dest;
	else
		return get_abs_tile_coord();
	}

/*
 *	Walk towards a given tile.
 */

void Actor::walk_to_tile
	(
	Tile_coord dest,		// Destination.
	int speed,			// Time between frames (msecs).
	int delay			// Delay before starting (msecs) (only
					//   if not already moving).
	)
	{
	if (!action)
		action = new Path_walking_actor_action(new Zombie());
	set_action(action->walk_to_tile(get_abs_tile_coord(), dest, get_type_flags()));
	if (action)			// Successful at setting path?
		start(speed, delay);
	else
		frame_time = 0;		// Not moving.
	}

/*
 *	Find a path towards a given tile.
 *
 *	Output:	0 if failed.
 */

int Actor::walk_path_to_tile
	(
	Tile_coord src,			// Our location, or an off-screen
					//   location to try path from.
	Tile_coord dest,		// Destination.
	int speed,			// Time between frames (msecs).
	int delay			// Delay before starting (msecs) (only
					//   if not already moving).
	)
	{
	set_action(new Path_walking_actor_action(new Astar()));
	set_action(action->walk_to_tile(src, dest, get_type_flags()));
	if (action)			// Successful at setting path?
		{
		start(speed, delay);
		return (1);
		}
	frame_time = 0;			// Not moving.
	return (0);
	}

/*
 *	Begin animation.
 */

void Actor::start
	(
	int speed,			// Time between frames (msecs).
	int delay			// Delay before starting (msecs) (only
					//   if not already moving).
	)
	{
	frame_time = speed;
	Game_window *gwin = Game_window::get_game_window();
	if (!in_queue() || delay)	// Not already in queue?
		{
		if (delay)
			gwin->get_tqueue()->remove(this);
		uint32 curtime = SDL_GetTicks();
		gwin->get_tqueue()->add(curtime + delay, this, (long) gwin);
		}
	}

/*
 *	Stop animation.
 */
void Actor::stop
	(
	)
	{
	if (action)
		{
		action->stop(this);
		add_dirty(Game_window::get_game_window());
		}
	frame_time = 0;
	}

/*
 *	Want one value to approach another.
 */

inline int Approach
	(
	int from,
	int to,
	int dist			// Desired distance.
	)
	{
	if (from <= to)			// Going forwards?
		return (to - from <= dist ? from : to - dist);
	else				// Going backwards.
		return (from - to <= dist ? from : to + dist);
	}

/*
 *	Follow the leader.
 */

void Actor::follow
	(
	Actor *leader
	)
	{
	static const char *catchup_phrases[3] = { 
					"Thou shan't lose me so easily!",
					"Ah, there thou art!",
					"Found ye!" };
	if (Actor::is_dead_npc())
		return;			// Not when dead.
	int delay = 0;
					// How close to aim for.
	int dist = 3 + Actor::get_party_id()/3;
	Tile_coord leaderpos = leader->get_abs_tile_coord();
	Tile_coord pos = get_abs_tile_coord();
	Tile_coord goal;
	if (leader->is_moving())	// Figure where to aim.
		{			// Aim for leader's dest.
		goal = leader->get_dest();
		goal.tx = Approach(pos.tx, goal.tx, dist);
		goal.ty = Approach(pos.ty, goal.ty, dist);
		}
	else				// Leader stopped?
		{
		goal = leaderpos;	// Aim for leader.
//		cout << "Follow:  Leader is stopped" << endl;
		goal.tx += 1 - rand()%3;// Jiggle a bit.
		goal.ty += 1 - rand()%3;
		}
					// Already aiming along a path?
	if (is_moving() && action && action->following_smart_path() /* &&
					goal.distance(get_dest()) <= 3 */ )
		return;
					// Tiles to goal.
	int goaldist = goal.distance(pos);
	if (goaldist < dist)		// Already close enough?
		return;
					// Get leader's distance from goal.
	int leaderdist = goal.distance(leaderpos);
					// Get his speed.
	int speed = leader->get_frame_time();
	if (!speed)			// Not moving?
		speed = 100;
	if (goaldist < leaderdist)	// Closer than leader?
					// Delay a bit.
		delay = (1 + leaderdist - goaldist)*100;
	else if (goaldist - leaderdist >= 5)
		speed -= 20;		// Speed up if too far.
	if (goaldist > 32 &&		// Getting kind of far away?
	    get_party_id() >= 0)	// And a member of the party.
		{			// Teleport.
		int pixels = goaldist*c_tilesize;
		Game_window *gwin = Game_window::get_game_window();
		if (pixels > gwin->get_width() + 16)
			{
					// Find a free spot.
			goal = leader->find_unblocked_tile(2, 3);
			if (goal.tx != -1)
				{
				move(goal.tx, goal.ty, goal.tz);
				int phrase = rand()%6;
				if (phrase < 3)
					say(catchup_phrases[phrase]);
				gwin->paint();
				return;
				}
			}
		}
	uint32 curtime = SDL_GetTicks();
	int dist2lead;
	if ((((dist2lead = pos.distance(leaderpos)) >= 5 &&
						curtime >= next_path_time) ||
	     (dist2lead >= 4 && !leader->is_moving())) && 
	      get_party_id() >= 0 && 
	      (!is_moving() || !action || !action->following_smart_path()))
		{			// A little stuck?
		cout << get_name() << " at distance " << dist2lead 
				<< " trying to catch up." << endl;
					// Don't try again for a second.
		next_path_time = SDL_GetTicks() + 1000;
		if (Chunk_object_list::is_blocked(goal, 3, get_type_flags()))
					// Find a free spot.
			goal = leader->find_unblocked_tile(1, 3);
		if (goal.tx == -1)	//+++++Testing
			cout << "... but is blocked." << endl;
		if (goal.tx == -1 ||	// No free spot?  Give up.
		    walk_path_to_tile(goal, speed - speed/4, 0))
			return;
		else
			cout << "... but failed to find path." << endl;
		}
	walk_to_tile(goal, speed, delay);
	}

/*
 *	Get information about a tile that an actor is about to step onto.
 */

#if !defined(BEOS) && !defined(ALPHA_LINUX_CXX)
inline
#endif
void Actor::get_tile_info
	(
	Actor *actor,			// May be 0 if not known.
	Game_window *gwin,
	Chunk_object_list *nlist,	// Chunk.
	int tx, int ty,			// Tile within chunk.
	int& water,			// Returns 1 if water.
	int& poison			// Returns 1 if poison.
	)
	{
	ShapeID flat = nlist->get_flat(tx, ty);
	if (flat.get_shapenum() == -1)
		water = poison = 0;
	else
		{
		Shape_info& finfo = gwin->get_info(flat.get_shapenum());
		water = finfo.is_water();
		poison = finfo.is_poisonous();
		Game_object *boots;	// Check for swamp/swamp boots.
		if (poison && actor && 
		    (boots = actor->Actor::get_readied(Actor::feet)) != 0 &&
		    boots->get_shapenum() == 588)
			poison = 0;
		}
	}

/*
 *	Set combat opponent.
 */

void Actor::set_opponent
	(
	Game_object *obj
	)
	{
	if (schedule_type != Schedule::combat || !schedule)
		set_schedule_type(Schedule::combat);
	if (schedule)
		schedule->set_opponent(obj);
	start(100);			// Get going if not already.
	}

/*
 *	Return current opponent.
 */

Game_object *Actor::get_opponent
	(
	)
	{
	return schedule ? schedule->get_opponent() : 0;
	}

/*
 *	Find the best spot where an item may be readied.
 *
 *	Output:	Index, or -1 if none found.
 */

int Actor::find_best_spot
	(
	Game_object *obj
	)
{

	Shape_info& info = 
		Game_window::get_game_window()->get_info(obj);

	if (Game::get_game_type() != SERPENT_ISLE)
	{
		if (info.get_shape_class() == Shape_info::container)
			return !spots[back] ? back : free_hand();

		Ready_type type = (Ready_type) info.get_ready_type();
		
		switch (type)
		{
			case spell:
			case other_spell:
			case one_handed_weapon:			// Also includes shields.
			case tongs:
			{
				int spot = free_hand();
				if (spot == -1 && !spots[belt])
					spot = belt;
				return spot;
			}
			case neck_armor:
			return !spots[neck] ? neck : free_hand();
				
			case torso_armor:
			return !spots[torso] ? torso : free_hand();
				
			case ring:
			return (free_finger() != -1 ? free_finger() : free_hand());
				
			case ammunition:		// ++++++++Check U7.
			return !spots[ammo] ? ammo : free_hand();
			
			case head_armor:
			return !spots[head] ? head : free_hand();
				
			case leg_armor:
			return !spots[legs] ? legs : free_hand();
				
			case foot_armor:
			return !spots[feet] ? feet : free_hand();
				
			case two_handed_weapon:
			return (!spots[lhand] && !spots[rhand]) ? lrhand : -1;

			// Gloves occupy both finger spots
			case gloves:
			return (!spots[lfinger] && !spots[rfinger]) ? lrfinger : free_hand();
			case other:
			return free_hand();

			default:
			#ifdef DEBUG
			cerr << "NPC " << get_npc_num() << ":  Unknown ready type: " << type << "  Name: " << item_names[obj->get_shapenum()] << endl;
			cerr << "Shape: " << obj->get_shapenum() << "  Frame: " << obj->get_framenum() << endl;
			cerr << endl;
			#endif //DEBUG
			return free_hand();
		}
	}
	else	// Serpent Isle Types
	{
		Ready_type_SI type = (Ready_type_SI) info.get_ready_type();
		
		switch (type)
		{
			case spell_si:
			case other_spell_si:
			case one_handed_si:

			// First check for preferences
			if (!two_handed && !spots[lhand] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), lhand))
				return lhand;
			else if (!two_handed && !spots[rhand] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), rhand))
				return rhand;
						
			// Ok, no spot found, now check for free hand
			if (free_hand() != -1) return free_hand();
				
			// Still, no spot found, lastly check for secondary places
			if (!spots[belt] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), belt))
				return belt;
			else if (!spots[shield_spot] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), shield_spot))
				return shield_spot;
			
			return -1;
			

			case cloak_si:
			return !spots[cloak_spot] ? cloak_spot : free_hand();
			
			case amulet_si:
			return !spots[neck] ? neck : free_hand();

			case helm_si:
			return !spots[head] ? head : free_hand();

			case gloves_si:
			return !spots[hands2_spot] ? hands2_spot : free_hand();

			case usecode_container_si:
			return !spots[ucont_spot] ? ucont_spot : -1;	// don't put in hands
			
			case ring_si:
			return (free_finger() != -1 ? free_finger() : free_hand());

			case earrings_si:
			return !spots[ears_spot] ? ears_spot : free_hand();

			case ammo_si:
			return !spots[ammo] ? ammo : free_hand();
			
			case belt_si:
			return !spots[belt] ? belt : free_hand();
			
			case armour_si:
			return !spots[torso] ? torso : free_hand();
			
			case boots_si:
			return !spots[feet] ? feet : free_hand();
			
			case leggings_si:
			return !spots[legs] ? legs : free_hand();
			
			case backpack_si:
			return !spots[back] ? back : free_hand();
		
			
			case two_handed_si:
			
			// First check hands
			if (!spots[lhand] && !spots[rhand]) return lrhand;
				
			// Check storage
			if (!spots[belt] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), belt))
				return belt;
			else if (!spots[back2h_spot] && Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), back2h_spot))
				return back2h_spot;
				
			return -1;


			case other:
			
			if (obj->get_shapenum() == 802 && !spots[belt])			// Bag
				return belt;
			else if (obj->get_shapenum() == 583 && !spots[back2h_spot])	// Bed roll
				return back2h_spot;
			else if (obj->get_shapenum() == 583 && !spots[back])		// Bed roll
				return back;
		
			return free_hand();


			default:
			#ifdef DEBUG
			cerr << "NPC " << get_npc_num() << ":  Unknown ready type: " << type << "  Name: " << item_names[obj->get_shapenum()] << endl;
			cerr << "Shape: " << obj->get_shapenum() << "  Frame: " << obj->get_framenum() << endl;
			cerr << endl;
			#endif //DEBUG
			return free_hand();
		}
	}

	return -1;
}

/*
 *	Set new schedule by type.
 */

void Actor::set_schedule_type
	(
	int new_schedule_type,
	Schedule *newsched		// New sched., or 0 to create here.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	stop();				// Stop moving.
	if (schedule)			// Finish up old if necessary.
		schedule->ending(new_schedule_type);
	set_action(0);			// Clear out old action.
					// Save old for a moment.
	Schedule::Schedule_types old_schedule = (Schedule::Schedule_types)
								schedule_type;
	schedule_type = new_schedule_type;
	delete schedule;		// Done with the old.
	schedule = newsched;
	if (!schedule)
		switch ((Schedule::Schedule_types) schedule_type)
			{
		case Schedule::combat:
			schedule = new Combat_schedule(this, old_schedule);
			break;
		case Schedule::horiz_pace:
			schedule = Pace_schedule::create_horiz(this);
			break;
		case Schedule::vert_pace:
			schedule = Pace_schedule::create_vert(this);
			break;
		case Schedule::talk:
			schedule = new Talk_schedule(this);
			break;
		case Schedule::dance:
			schedule = new Dance_schedule(this);
			break;
		case Schedule::farm:	// Use a scythe.
			schedule = new Tool_schedule(this, 618);
			break;
		case Schedule::tend_shop:// For now.
			schedule = new Loiter_schedule(this, 3);
			break;
		case Schedule::miner:	// Use a pick.
			schedule = new Tool_schedule(this, 624);
			break;
		case Schedule::loiter:
		case Schedule::hound:	// For now.
		case Schedule::graze:
			schedule = new Loiter_schedule(this);
			break;
		case Schedule::wander:
			schedule = new Wander_schedule(this);
			break;
		case Schedule::blacksmith:
			break;		//++++++++++
		case Schedule::sleep:
			schedule = new Sleep_schedule(this);
			break;
		case Schedule::wait:	// Loiter just a little
//Figure out why this messes up Mayor's talk at intro.
//			schedule = new Loiter_schedule(this, 1);
			break;
		case Schedule::eat:		// For now.
		case Schedule::sit:
			schedule = new Sit_schedule(this);
			break;
		case Schedule::bake:
			break;		//+++++++
		case Schedule::sew:
			break;		//+++++++
		case Schedule::shy:
			break;		//+++++++
		case Schedule::lab:
			break;		//+++++++
		case Schedule::thief:		// Just face north, for now.
			gwin->add_dirty(this);
			set_frame(get_dir_framenum(0, Actor::standing));
			gwin->add_dirty(this);
			break;
		case Schedule::waiter:
			schedule = new Waiter_schedule(this);
			break;
		case Schedule::kid_games:
			schedule = new Kid_games_schedule(this);
			break;
		case Schedule::eat_at_inn:
			schedule = new Eat_at_inn_schedule(this);
			break;
		case Schedule::duel:
			schedule = new Duel_schedule(this);
			break;
		case Schedule::preach:
			schedule = new Preach_schedule(this);
			break;
		case Schedule::patrol:
			schedule = new Patrol_schedule(this);
			break;
		case Schedule::desk_work:
			break;		//+++++++
		default:
			break;
			}
	if (!gwin->is_chunk_read(get_cx(), get_cy()))
		dormant = true;		// Chunk hasn't been read in yet.
	else if (schedule)		// Try to start it.
		{
		dormant = false;
		schedule->now_what();
		}
	}

/*
 *	Render.
 */

void Actor::paint
	(
	Game_window *gwin
	)
	{
	if (!(flags & (1L << Obj_flags::dont_render)))
		{
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		if (flags & (1L << Obj_flags::invisible))
			gwin->paint_invisible(xoff, yoff, get_shapenum(),
							get_framenum());
		else
			gwin->paint_shape(xoff, yoff, get_shapenum(),
							get_framenum());
		paint_weapon(gwin);
		if (flags & ((1L<<Obj_flags::protection) | (1L << Obj_flags::poisoned)))
			{
			if (flags & (1L << Obj_flags::poisoned))
				gwin->paint_poison_outline(xoff, yoff,
					get_shapenum(), get_framenum());
			else
				gwin->paint_protect_outline(xoff, yoff,
					get_shapenum(), get_framenum());
			}
		}
	}
/*
 *	Draw the weapon in the actor's hand (if any).
 */
/* Weapon frames:
	0 - normal item
	1 - in hand, actor facing north/south
	2 - attacking (pointing north)
	3 - attacking (pointing east)
	4 - attacking (pointing south)
*/
void Actor::paint_weapon
	(
	Game_window *gwin
	)
	{
	int weapon_x, weapon_y, weapon_frame;
	if (figure_weapon_pos(weapon_x, weapon_y, weapon_frame))
		{
		Game_object * weapon = spots[lhand];
		int shnum = weapon->get_shapenum();
		Shape_frame *wshape = gwin->get_shape(shnum, weapon_frame);
					// Set dirty area rel. to NPC.
		weapon_rect = gwin->get_shape_rect(wshape, weapon_x, weapon_y);
		// Paint the weapon shape using the actor's coordinates
		int xoff, yoff;
		gwin->get_shape_location(this, xoff, yoff);
		xoff += weapon_x;
		yoff += weapon_y;
		if (flags & (1L<<Obj_flags::invisible))
			gwin->paint_invisible(xoff, yoff, shnum, weapon_frame);
		else
			gwin->paint_shape(xoff, yoff, wshape);
		}
	else
		weapon_rect.w = 0;
	}

/*
 *	Figure weapon drawing info.  We need this in advance to set the dirty
 *	rectangle.
 *
 *	Output:	0 if don't need to paint weapon.
 */

int Actor::figure_weapon_pos
	(
	int& weapon_x, int& weapon_y,	// Pos. rel. to NPC.
	int& weapon_frame
	)
	{
	unsigned char actor_x, actor_y;
	unsigned char wx, wy;
	Game_object * weapon = spots[lhand];
	if(weapon == 0)
		return 0;
	Game_window *gwin = Game_window::get_game_window();
	// Get offsets for actor shape
	gwin->get_info(this).get_weapon_offset(get_framenum() & 0x1f, actor_x,
			actor_y);
	// Get offsets for weapon shape
	// NOTE: when combat is implemented, weapon frame should depend on
	// actor's current attacking frame
	weapon_frame = 1;
	gwin->get_info(weapon).get_weapon_offset(weapon_frame, wx,
			wy);
	// actor_x will be 255 if (for example) the actor is lying down
	// wx will be 255 if the actor is not holding a proper weapon
	if(actor_x != 255 && wx != 255)
		{			// Store offsets rel. to NPC.
		weapon_x = wx - actor_x;
		weapon_y = wy - actor_y;
		// Need to swap offsets if actor's shape is reflected
		if(get_framenum() & 32)
			{
			swap(weapon_x, weapon_y);
			weapon_frame |= 32;
			}
		return 1;
		}
	else
		return 0;
	}

/*
 *	Run usecode when double-clicked.
 */
void Actor::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	int serpent = (Game::get_game_type()==SERPENT_ISLE);
	
	Game_window *gwin = Game_window::get_game_window();
					// In gump mode?  Or Avatar?
	if (!npc_num && !serpent)	// Avatar?
		gwin->show_gump(this, ACTOR_FIRST_GUMP);
	else if (!npc_num && serpent)	// Avatar?
		gwin->show_gump(this, 123);
					// Gump/combat mode?
	else if ((gwin->get_mode() == Game_window::gump || gwin->in_combat())&&
		 get_party_id() >= 0 &&
		 npc_num >= 1 && npc_num <= 10 && !serpent)
					// Show companions' pictures. (BG)
			gwin->show_gump(this, ACTOR_FIRST_GUMP + 1 + npc_num);
	else if ((gwin->get_mode() == Game_window::gump || gwin->in_combat())&&
		 get_party_id() >= 0 && serpent)
					// Show companions' pictures. (SI)
			gwin->show_gump(this, 123);
	else if (get_schedule_type() == (int) Schedule::sleep ||
		 get_flag(Obj_flags::asleep))
		return;			// Asleep.
	else if (usecode == -1)
		umachine->call_usecode(get_shapenum(), this,
			(Usecode_machine::Usecode_events) event);
	else
		umachine->call_usecode(usecode, this, 
			(Usecode_machine::Usecode_events) event);
	
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Actor::drop
	(
	Game_object *obj
	)
	{
	return (add(obj));		// We'll take it.
	}

/*
 *	Get name.
 */

string Actor::get_name
	(
	) const
	{
	return (name.empty() || 
		!get_flag (Obj_flags::met)) ? Game_object::get_name() : name;
	}

/*
 *	Set property.
 */
void Actor::set_property
	(
	int prop, 
	int val
	)
	{
	if (prop == health && ((party_id != -1) || (npc_num == 0)) && 
		cheat.in_god_mode() && val < properties[prop])
		return;
	if (prop >= 0 && prop < 12)
		{
		if (prop == (int) exp)
			{		// Experience?  Check for new level.
			int old_level = get_level();
			properties[(int) exp] = (short) val;
			int delta = get_level() - old_level;
			if (delta > 0)
				properties[(int) training] += 3*delta;
			}
		else
			properties[prop] = (short) val;
		Game_window *gwin = Game_window::get_game_window();
		if (gwin->get_mode() == Game_window::gump)
			gwin->set_all_dirty();
		}
	}

/*
 *	This method should be called to decrement health from attacks, traps.
 */

void Actor::reduce_health
	(
	int delta			// # points to lose.
	)
	{
	if (cheat.in_god_mode() && ((party_id != -1) || (npc_num == 0)))
		return;
	int oldhp = properties[(int) health];
	int maxhp = properties[(int) strength];
	int val = oldhp - delta;
	properties[(int) health] = val;
	Game_window *gwin = Game_window::get_game_window();
	if (this == gwin->get_main_actor() && val < maxhp/8)
					// Flash red if Avatar badly hurt.
		if (rand()%2)
			gwin->flash_palette_red();
	if (Actor::is_dead_npc())
		die();
	else if (val < 0 && !get_flag(Obj_flags::asleep))
		set_flag(Obj_flags::asleep);
	}

/*
 *	Set flag.
 */

void Actor::set_flag
	(
	int flag
	)
	{
//	cout << "Set flag for NPC " << get_npc_num() << " = " << flag << endl;

	// Small hack to stop SI from hiding the avatar
	if (get_npc_num() == 0
		&& flag == 16
		&& Game::get_game_type() == SERPENT_ISLE)
		return;
		
	if (flag >= 0 && flag < 32)
		flags |= ((uint32) 1 << flag);
	else if (flag >= 32 && flag < 64)
		flags2 |= ((uint32) 1 << (flag-32));
	Game_window *gwin = Game_window::get_game_window();
					// Check sched. to avoid waking
					//   Penumbra.
	if (flag == Obj_flags::asleep && schedule_type != Schedule::sleep)
		{			// Set timer to wake in a few secs.
		need_timers()->start_sleep();
		if ((get_framenum()&0xf) != Actor::sleep_frame &&
		    get_shapenum() > 0)	// (Might not be initialized yet.)
			{		// Lie down.
			gwin->add_dirty(this);
			set_frame(Actor::sleep_frame + ((rand()%4)<<4));
			gwin->add_dirty(this);
			set_action(0);	// Stop what you're doing.
			}
		}
	if (flag == Obj_flags::poisoned)
		need_timers()->start_poison();
	if (flag == Obj_flags::protection)
		need_timers()->start_protection();
	if (flag == Obj_flags::invisible)
		{
		need_timers()->start_invisibility();
		gwin->set_palette();
		}
					// Update stats if open.
	if (gwin->get_mode() == Game_window::gump)
		gwin->set_all_dirty();
	set_actor_shape();
	}

void Actor::set_siflag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		siflags |= ((uint32) 1 << flag);

	set_actor_shape();
	}

void Actor::set_type_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 16)
		type_flags |= ((uint32) 1 << flag);

	set_actor_shape();
	}

/*
 *	Clear flag.
 */

void Actor::clear_flag
	(
	int flag
	)
	{
//	cout << "Clear flag for NPC " << get_npc_num() << " = " << flag << endl;
	if (flag >= 0 && flag < 32)
		flags &= ~((uint32) 1 << flag);
	else if (flag >= 32 && flag < 64)
		flags2 &= ~((uint32) 1 << (flag-32));
	if (flag == Obj_flags::invisible)		// Restore normal palette.
		Game_window::get_game_window()->set_palette();
	else if (flag == Obj_flags::asleep && schedule_type == Schedule::sleep)
		set_schedule_type(Schedule::stand);
	set_actor_shape();
	}

void Actor::clear_siflag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		siflags &= ~((uint32) 1 << flag);

	set_actor_shape();
	}

void Actor::clear_type_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 16)
		type_flags &= ~((uint32) 1 << flag);

	set_actor_shape();
	}

/*
 *	Get flags.
 */

int Actor::get_siflag
	(
	int flag
	) const
	{
	return (flag >= 0 && flag < 32) ? (siflags & ((uint32) 1 << flag))
			!= 0 : 0;
	}

int Actor::get_type_flag
	(
	int flag
	) const
	{
	return (flag >= 0 && flag < 16) ? (type_flags & ((uint32) 1 << flag))
			!= 0 : 0;
	}
/*
 *	SetFlags
 */

void Actor::set_type_flags
	(
	unsigned short tflags
	)
	{
	type_flags = tflags;
	set_actor_shape();
	}

/*
 *	Call usecode function for an object that's readied/unreadied.
 */

inline void Call_readied_usecode
	(
	Game_window *gwin,
	Actor *npc,
	Game_object *obj,
	Usecode_machine::Usecode_events eventid
	)
	{
	if (obj->get_shapenum() == 297)	// Fix special case:  ring of protect.
		{
		if (eventid == Usecode_machine::readied)
			npc->Actor::set_flag(Obj_flags::protection);
		else
			npc->Actor::clear_flag(Obj_flags::protection);
		return;
		}
	Shape_info& info = gwin->get_info(obj);
	if (info.get_shape_class() != Shape_info::container)
		{
		Ready_type type = (Ready_type) info.get_ready_type();
		if (type != other)
			gwin->get_usecode()->call_usecode(obj->get_shapenum(),
					obj, eventid);
		}
	}


/*
 *	Remove an object.
 */

void Actor::remove
	(
	Game_object *obj
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int index = Actor::find_readied(obj);	// Remove from spot.
	if (index == rfinger || index == lfinger)
		Call_readied_usecode(gwin, this, 
					obj, Usecode_machine::unreadied);
	Container_game_object::remove(obj);
	if (index >= 0)
		{			// Update light-source count.
		if (gwin->get_info(obj).is_light_source())
			light_sources--;
		spots[index] = 0;
		if (index == rhand || index == lhand)
			two_handed = false;
		if (index == rfinger || index == lfinger)
			two_fingered = false;
		}
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Actor::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume check (AND also
					//   to skip usecode call).
	)
	{
	int index = find_best_spot(obj);// Where should it go?
#ifdef DEBUG
	if (Game::get_game_type() == SERPENT_ISLE)
	{
		cerr << "NPC " << get_npc_num() << "  Name: " << item_names[obj->get_shapenum()] << endl;
		cerr << "Shape: " << obj->get_shapenum() << "  Frame: " << obj->get_framenum() << endl;
		cerr << "Index: " << index << endl;
		cerr << endl;
	}
#endif //DEBUG
		
	if (index < 0)			// No free spot?  Look for a bag.
		{
		if (spots[back] && spots[back]->drop(obj))
			return (1);
		if (spots[belt] && spots[belt]->drop(obj))
			return (1);
		if (spots[lhand] && spots[lhand]->drop(obj))
			return (1);
		if (spots[rhand] && spots[rhand]->drop(obj))
			return (1);
		return (0);
		}
					// Add to ourself.
	if (!Container_game_object::add(obj, dont_check))
		return (0);
	if (index == lrhand)		// Two-handed?
		{
		two_handed = true;
		index = lhand;
		}
	if (index == lrfinger)		// Gloves?
		{
		two_fingered = true;
		index = lfinger;
		}
	spots[index] = obj;		// Store in correct spot.
	obj->set_chunk(0, 0);		// Clear coords. (set by gump).
	Game_window *gwin = Game_window::get_game_window();
	if (!dont_check &&		// Watch for initialization.
	    (index == lfinger || index == rfinger))
		Call_readied_usecode(gwin, this, obj,Usecode_machine::readied);
	if (gwin->get_info(obj).is_light_source())
		light_sources++;
	return (1);
	}

/*
 *	Okay to store object in belt?
 */

static int Belt_okay
	(
	Game_object *obj
	)
	{
	Shape_info& info = 
		Game_window::get_game_window()->get_info(obj);
	Ready_type type = (Ready_type) info.get_ready_type();
	switch (type)
		{
	case spell:
	case other_spell:
	case one_handed_weapon:
	case tongs:
		return 1;
	default:
		return 0;
		}
	}

/*
 *	Add to given spot.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::add_readied
	(
	Game_object *obj,
	int index			// Spot #.
	)
	{

	if (index < 0 || index >= (int)(sizeof(spots)/sizeof(spots[0])))
		return (0);		// Out of range.
	if (spots[index])		// Already something there?
					// Try to drop into it.
		return (spots[index]->drop(obj));
					// Get best place it should go.
	int best_index = find_best_spot(obj);
	if (best_index == -1)		// No place?
		return (0);

#ifdef DEBUG
	if (Game::get_game_type() == SERPENT_ISLE)
	{
		cerr << "NPC " << get_npc_num() << "  Name: " << item_names[obj->get_shapenum()] << endl;
		cerr << "Shape: " << obj->get_shapenum() << "  Frame: " << obj->get_framenum() << endl;
		cerr << "Index: " << index << "  Best Index: " << best_index << endl;
	}
#endif //DEBUG

	// If it's a two handed weapon and put on a hand change to lhand
	if (best_index == lrhand && index == rhand) index = lhand;

	// Special Serpent Isle Code for if something is not going in a hand
	if (Game::get_game_type() == SERPENT_ISLE && index != best_index)
	{
		Shape_info& info =  Game_window::get_game_window()->get_info(obj);
		Ready_type_SI type = (Ready_type_SI) info.get_ready_type();

		// Can it go where we want? If so set best index to this
		// Otherwise set this to best index
		if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), index) && index != lhand && index != rhand)
 			best_index = index;
		else if (best_index == lrhand)
			index = lhand;
		else if (type != spell_si && type != other_spell_si)
			index = best_index;
	}

#ifdef DEBUG
	if (Game::get_game_type() == SERPENT_ISLE)
	{
		cerr << "Index2: " << index << "  Best Index2: " << best_index << endl;
		cerr << endl;
	}
#endif //DEBUG

	if (index == best_index || (!two_handed && index == lhand)
			|| (!two_handed && index == rhand
				&& best_index != lrhand) ||
	   (index == belt && Belt_okay(obj)))
		{			// Okay.
		if (!Container_game_object::add(obj))
			return (0);	// No room, or too heavy.
		spots[index] = obj;
		obj->set_chunk(0, 0);	// Clear coords. (set by gump).
		if (best_index == lrhand)
			two_handed = true;	// Must be a two-handed weapon.
		if (best_index == lrfinger)
			two_fingered = true;	// Must be gloves
		Game_window *gwin = Game_window::get_game_window();
		if (index == lfinger || index == rfinger)
			Call_readied_usecode(gwin, this, 
						obj,Usecode_machine::readied);
		if (gwin->get_info(obj).is_light_source())
			light_sources++;
		return (1);
		}
	return (0);
	}

/*
 *	Find index of spot where an object is readied.
 *
 *	Output:	Index, or -1 if not found.
 */

int Actor::find_readied
	(
	Game_object *obj
	)
	{
	for (size_t i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		if (spots[i] == obj)
			return (i);
	return (-1);
	}

/*
 *	Change shape of a member.
 */

void Actor::change_member_shape
	(
	Game_object *obj,
	int newshape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (gwin->get_info(obj).is_light_source())
		light_sources--;
	Container_game_object::change_member_shape(obj, newshape);
	if (gwin->get_info(obj).is_light_source())
		light_sources++;
	}

/*
 *	Step aside to a free tile, or try to swap places.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::move_aside
	(
	Actor *for_actor,		// Moving aside for this one.
	int dir				// Direction to avoid (0-7).
	)
	{
	Tile_coord cur = get_abs_tile_coord();
	int opp = (dir + 4)%8;		// Don't go in opposite dir. either.
	Tile_coord to(-1, -1, -1);
	int i;
	for (i = 0; i < 8; i++)		// Go through directions.
		if (i == dir || i == opp)
			continue;	// Don't go that way.
		else
			{
			to = cur.get_neighbor(i);
					// Assume height = 3.
			if (!Chunk_object_list::is_blocked(
						to, 3, get_type_flags()))
				break;
			}
	int stepdir = i;		// This is the direction.
	if (i == 8 || to.tx < 0)	// Failed?  Try to swap places.
		return swap_positions(for_actor);
					// Step, and face direction.
	step(to, get_dir_framenum(stepdir, (int) Actor::standing));
	Tile_coord newpos = get_abs_tile_coord();
	return (newpos.tx == to.tx && newpos.ty == to.ty);
	}

/*
 *	Get frame if rotated 1, 2, or 3 quadrants clockwise.
 */

int Actor::get_rotated_frame
	(
	int quads			// 1=90, 2=180, 3=270.
	)
	{
	int curframe = get_framenum();
					// Bit 4=rotate180, 5=rotate-90.
	int curdir = (4 + 2*((curframe>>4)&1) - ((curframe>>5)&1))%4;
	int newdir = (curdir + quads)%4;
					// Convert to 8-value dir & get frame.
	return get_dir_framenum(2*newdir, curframe);
	}

/*
 *	Get total value of armor being worn.
 */

int Actor::get_armor_points
	(
	)
	{
	int points = 0;
	static enum Spots aspots[] = {neck, torso, lfinger, rfinger, head,
					rhand, legs, feet};
	const int num_armor_spots = sizeof(aspots)/sizeof(aspots[0]);
	Game_window *gwin = Game_window::get_game_window();
	for (int i = 0; i < num_armor_spots; i++)
		{
		Game_object *armor = spots[(int) aspots[i]];
		if (armor)
			points += gwin->get_info(armor).get_armor();
		}
	return points;
	}

/*
 *	Get weapon value.
 */

Weapon_info *Actor::get_weapon
	(
	int& points,
	int& shape
	)
	{
	points = 0;
	Weapon_info *winf = 0;
	Game_window *gwin = Game_window::get_game_window();
	Game_object *weapon = spots[(int) lhand];
	if (weapon)
		if ((winf = gwin->get_info(weapon).get_weapon_info()) != 0)
			{
			points = winf->get_damage();
			shape = weapon->get_shapenum();
			}
					// Try both hands.
	weapon = spots[(int) rhand];
	if (weapon)
		{
		Weapon_info *rwinf = gwin->get_info(weapon).get_weapon_info();
		int rpoints;
		if (rwinf && (rpoints = rwinf->get_damage()) > points)
			{
			winf = rwinf;
			points = rpoints;
			shape = weapon->get_shapenum();
			}
		}
	return winf;
	}

/*
 *	Figure hit points lost from an attack, and subtract from total.
 *
 *	Output:	# of hit points lost (already subtracted).
 */

int Actor::figure_hit_points
	(
	Actor *attacker,		// 0 if hit from a missile egg.
	int weapon_shape,
	int ammo_shape
	)
	{

	// godmode effects:
	if (((party_id != -1) || (npc_num == 0)) && cheat.in_god_mode())
		return 0;
	bool instant_death = (cheat.in_god_mode() && attacker &&
		((attacker->party_id != -1) || (attacker->npc_num == 0)));

	Game_window *gwin = Game_window::get_game_window();
	int armor = get_armor_points();
	int wpoints;
	Weapon_info *winf;
	if (weapon_shape > 0)
		{
		winf = gwin->get_info(weapon_shape).get_weapon_info();
		wpoints = winf ? winf->get_damage() : 0;
		}
	else if (ammo_shape > 0)
		{
		winf = gwin->get_info(ammo_shape).get_weapon_info();
		wpoints = winf ? winf->get_damage() : 0;
		}
	else
		winf = attacker->get_weapon(wpoints);
					// Get bonus ammo points.
	Ammo_info *ainf = Ammo_info::find(ammo_shape);
	if (ainf)
		wpoints += ainf->get_damage();
	int usefun;			// See if there's usecode for it.
	if (winf && (usefun = winf->get_usecode()) != 0)
		gwin->get_usecode()->call_usecode(usefun, this,
					Usecode_machine::weapon);
	if (!wpoints && (!winf || !winf->get_special_atts()))
		return 0;		// No harm can be done.

	int attacker_level = attacker ? attacker->get_level() : 4;
	int prob = 40 + attacker_level + (attacker ?
			(attacker->get_property((int) combat) +
			attacker->get_property((int) dexterity)) : 20) -
			get_property((int) dexterity) +
			wpoints - armor;
	if (get_flag(Obj_flags::protection))// Defender is protected?
		prob -= (40 + rand()%20);
	if (instant_death)
		prob = 200;	// always hits

	cout << "Hit probability is " << prob << endl;
	if (rand()%100 > prob)
		return 0;		// Missed.
					// +++++Do special atts. too.
					// Compute hit points to lose.
	int hp = (attacker ? attacker->get_property((int) strength)/4 : 2) +
			(rand()%attacker_level) +
			wpoints - armor;
	if (hp < 1)
		hp = 1;
	int oldhealth = properties[(int) health];
	int maxhealth = properties[(int) strength];

	if (instant_death)		//instant death
		hp = properties[(int) health] + properties[(int) strength] + 1;
	int newhp = oldhealth - hp;	// Subtract from health.

	if (oldhealth >= maxhealth/2 && newhp < maxhealth/2 && rand()%3 != 0)
					// A little oomph.
		if (instant_death)
			say("\"Cheater!\"");
		else
			say(first_ouch, last_ouch);
	reduce_health(hp);
	cout << "Attack damage was " << hp << " hit points, leaving " << 
		properties[(int) health] << " remaining" << endl;
	return hp;
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if destroyed, else object itself.
 */

Game_object *Actor::attacked
	(
	Actor *attacker,		// 0 if from a trap.
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape			// Also may be 0.
	)
	{
	if (attacker && attacker->get_schedule_type() == Schedule::duel)
		return this;			// Just play-fighting.
	figure_hit_points(attacker, weapon_shape, ammo_shape);
	if (attacker && is_dead_npc())
		{
					// Experience gained = strength???
		int expval = get_property((int) strength);
		if (is_monster())
					// Attacker gains experience.
			attacker->set_property((int) exp,
				attacker->get_property((int) exp) + expval);
		return 0;
		}
	return this;
	}

/*
 *	There's probably a smarter way to do this, but this routine checks
 *	for the dragon Draco.
 */

static int Is_draco
	(
	Actor *dragon
	)
	{
	Game_object_vector vec;		// Gets list.
						// Should have a special scroll.
	int cnt = dragon->get_objects(vec, 797, 241, 4);
	return cnt > 0;
	}

/*
 *	We're dead.  We're removed from the world, but not deleted.
 */

void Actor::die
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get location.
	Tile_coord pos = get_abs_tile_coord();
	set_action(0);
	delete schedule;
	schedule = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
	int shnum = get_shapenum();
					// Special case:  Hook, Dracothraxus.
	if ((shnum == 0x1fa || (shnum == 0x1f8 && Is_draco(this))) && 
	    Game::get_game_type() == BLACK_GATE)
		{			// Exec. usecode before dying.
		gwin->get_usecode()->call_usecode(
				shnum, this, Usecode_machine::internal_exec);
					// Restore mode.
		gwin->set_mode(Game_window::normal);
		if (get_cx() == 255)	// Invalid now?
			return;
		}
	properties[(int) health] = -50;
	add_dirty(gwin);		// Want to repaint area.
	remove_this(1);			// Remove (but don't delete this).
	cx = cy = 0xff;			// Set to invalid chunk coords.
	int frnum;			// Lookup body shape/frame.
	if (!Body_lookup::find(get_shapenum(), shnum, frnum))
		{
		shnum = 400;
		frnum = 3;
		}
					// Put body here.
	Dead_body *body = new Dead_body(shnum, frnum, 0, 0, 0, 
					npc_num > 0 ? npc_num : -1, 1);
	if (body->Dead_body::get_live_npc_num() != -1)
		body->set_quality(1);	// Flag for dead body of NPC.
	else
		body->set_quality(2);	// Set to decay.
	body->move(pos);
	body->set_flag(Obj_flags::okay_to_take);	// Okay to take its contents.
	Game_object *item;		// Move all the items.
	Game_object_vector tooheavy;		// Some shouldn't be moved.
	while ((item = objects.get_first()) != 0)
		{
		remove(item);
		if (item->is_dragable())
			body->add(item, 1);// Always succeed at adding.
		else
			tooheavy.push_back(item);
		}
					// Put the heavy ones back.
	for (Game_object_vector::const_iterator it = tooheavy.begin(); 
						it != tooheavy.end(); ++it)
		add(*it, 1);
	gwin->add_dirty(body);
	}

/*
 *	Restore from body.  It must not be owned by anyone.
 *
 *	Output:	->actor if successful, else 0.
 */

Actor *Actor::resurrect
	(
	Dead_body *body			// Must be this actor's body.
	)
	{
	if (body->get_owner())		// Must be on ground.
		return (0);
	Game_object *item;		// Get back all the items.
	while ((item = body->get_objects().get_first()) != 0)
		{
		body->remove(item);
		add(item, 1);		// Always succeed at adding.
		}
	Game_window *gwin = Game_window::get_game_window();
	gwin->add_dirty(body);		// Need to repaint here.
	Tile_coord pos = body->get_abs_tile_coord();
	body->remove_this();		// Remove and delete body.
	move(pos);			// Move back to life.
					// Restore health to max.
	properties[(int) health] = properties[(int) strength];
	return (this);
	}

/*
 *	Handle a time event (for animation).
 */

void Main_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (action)			// Doing anything?
		{			// Do what we should.
		int delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			gwin->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			{
			set_action(0);
			if (schedule)
				schedule->now_what();
			}
		}
	else if (schedule)
		schedule->now_what();
	}

/*
 *	Get the party to follow.
 */

void Main_actor::get_followers
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Usecode_machine *uc = gwin->get_usecode();
	int cnt = uc->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		Npc_actor *npc = (Npc_actor *) gwin->get_npc(
						uc->get_party_member(i));
		if (!npc || npc->get_flag(Obj_flags::asleep) ||
		    npc->Actor::is_dead_npc())
			continue;
		int sched = npc->get_schedule_type();
					// Skip if in combat or set to 'wait'.
		if (sched != Schedule::combat &&
		    sched != Schedule::wait)
			{
			if (sched != Schedule::follow_avatar)
				npc->set_schedule_type(
						Schedule::follow_avatar);
			npc->follow(this);
			}
		}
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 */

int Main_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	int old_lift = get_lift();
	int water, poison;		// Get tile info.
	get_tile_info(this, gwin, nlist, tx, ty, water, poison);
	int new_lift;			// Might climb/descend.
	Game_object *block;		// Just assume height==3.
	if (nlist->is_blocked(3, old_lift, tx, ty, new_lift, 
							get_type_flags()) &&
	   (!(block = Game_object::find_blocking(t)) || block == this ||
					// Try to get blocker to move aside.
	        !block->move_aside(this, get_direction(block)) ||
					// (May have swapped places.)
		(t != get_abs_tile_coord() &&
					// If okay, try one last time.
   		 nlist->is_blocked(3, old_lift, tx, ty, new_lift, 
							get_type_flags()))))
		{
		stop();
		return (0);
		}
	if (poison)
		Actor::set_flag((int) Obj_flags::poisoned);
					// Check for scrolling.
	gwin->scroll_if_needed(t);
	add_dirty(gwin);		/// Set to update old location.
					// Get old chunk, old tile.
	Chunk_object_list *olist = gwin->get_objects(get_cx(), get_cy());
	Tile_coord oldtile = get_abs_tile_coord();
					// Move it.
	Actor::movef(olist, nlist, tx, ty, frame, new_lift);
	add_dirty(gwin, 1);		// Set to update new.
					// In a new chunk?
	if (olist != nlist)
		switched_chunks(olist, nlist);
	int inside = nlist->is_roof (tx, ty, new_lift);
	if (gwin->set_above_main_actor (inside))
		{
		gwin->set_in_dungeon(nlist->has_dungeon() &&
					nlist->in_dungeon(tx, ty));
		gwin->set_all_dirty();
		}
	else if (inside && gwin->set_in_dungeon(nlist->has_dungeon() &&
				nlist->in_dungeon(tx, ty)))
		gwin->set_all_dirty();
					// Near an egg?  (Do this last, since
					//   it may teleport.)
	nlist->activate_eggs(this, t.tx, t.ty, new_lift, 
						oldtile.tx, oldtile.ty);
	return (1);
	}

/*
 *	Setup cache after a change in chunks.
 */

void Main_actor::switched_chunks
	(
	Chunk_object_list *olist,	// Old chunk, or null.
	Chunk_object_list *nlist	// New chunk.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int newcx = nlist->get_cx(), newcy = nlist->get_cy();
	int xfrom, xto, yfrom, yto;	// Get range of chunks.
	if (!olist)			// No old?  Use all 9.
		{
		xfrom = newcx > 0 ? newcx - 1 : newcx;
		xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
		yfrom = newcy > 0 ? newcy - 1 : newcy;
		yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
		}
	else
		{
		int oldcx = olist->get_cx(), oldcy = olist->get_cy();
		if (newcx == oldcx + 1)
			{
			xfrom = newcx;
			xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
			}
		else if (newcx == oldcx - 1)
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx;
			}
		else
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx < c_num_chunks - 1 ? newcx + 1 : newcx;
			}
		if (newcy == oldcy + 1)
			{
			yfrom = newcy;
			yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
			}
		else if (newcy == oldcy - 1)
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy;
			}
		else
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy < c_num_chunks - 1 ? newcy + 1 : newcy;
			}
		}
	for (int y = yfrom; y <= yto; y++)
		for (int x = xfrom; x <= xto; x++)
			gwin->get_objects(x, y)->setup_cache();
	}

/*
 *	Move (teleport) to a new spot.
 */

void Main_actor::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Store old chunk list.
	Chunk_object_list *olist = gwin->get_objects_safely(
						get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Chunk_object_list *nlist = gwin->get_objects(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
	int tx = get_tx(), ty = get_ty();
	if (gwin->set_above_main_actor(nlist->is_roof(tx, ty, newlift)))
		gwin->set_in_dungeon(nlist->has_dungeon() &&
				nlist->in_dungeon(tx, ty));

	}

/*
 *	We're dead.
 */

void Main_actor::die
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (gwin->in_combat())
		gwin->toggle_combat();	// Hope this is safe....
	gwin->end_gump_mode();		// Obviously.
					// Special function for dying:
	gwin->get_usecode()->call_usecode(
				0x60e, this, Usecode_machine::weapon);
	}

/*
 *	Set the shapenum based on skin color, sex, naked flag and petra flag
 */
void Actor::set_actor_shape()
{
	if ((get_npc_num() != 0) && (get_npc_num() != 28))
		return;

	Game_window *gwin = Game_window::get_game_window();
	Actor *avatar = Game_window::get_game_window()->get_main_actor();

	if (!avatar) return;

	int sn = get_shapenum();

	if (Game::get_game_type() == SERPENT_ISLE)
	{
		if ((avatar->get_flag (Obj_flags::petra) && npc_num == 0) ||
			(!avatar->get_flag (Obj_flags::petra) && npc_num != 0))
		{
			sn = 658;
		}
		else if (avatar->get_skin_color() == 0) // WH
		{
			sn = 1028+avatar->get_type_flag(Actor::tf_sex)+
					6*avatar->get_siflag(Actor::naked);
		}
		else if (avatar->get_skin_color() == 1) // BN
		{
			sn = 1026+avatar->get_type_flag(Actor::tf_sex)+
					6*avatar->get_siflag(Actor::naked);
		}
		else if (avatar->get_skin_color() == 2) // BK
		{
			sn = 1024+avatar->get_type_flag(Actor::tf_sex)+
					6*avatar->get_siflag(Actor::naked);
		}
	}
					// Here if Black Gate:
	else if (get_npc_num() == 0)
		{
		if (avatar->get_type_flag(Actor::tf_sex))
			sn = 989;
		else
			sn = 721;
		}

	set_shape (sn, get_framenum());

	// Set petra
	if (npc_num != 28 && gwin->get_npc(28))
		gwin->get_npc(28)->set_actor_shape();
}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	const std::string &nm, 			// Name.  A copy is made.
	int shapenum, 
	int fshape, 
	int uc
	) : Actor(nm, shapenum, fshape, uc), next(0), nearby(false),
		num_schedules(0), 
		schedules(0)
	{
	}

/*
 *	Kill an actor.
 */

Npc_actor::~Npc_actor
	(
	)
	{
	delete schedule;
	delete [] schedules;
	}

/*
 *	Set schedule list.
 */

void Npc_actor::set_schedules
	(
	Schedule_change *list, 
	int cnt
	)
	{
	delete [] schedules;
	schedules = list;
	num_schedules = cnt;
	}

/*
 *	Move and change frame.
 */

inline void Npc_actor::movef
	(
	Chunk_object_list *old_chunk,
	Chunk_object_list *new_chunk, 
	int new_sx, int new_sy,
	int new_frame, 
	int new_lift
	)
	{
	Actor::movef(old_chunk, new_chunk,
				new_sx, new_sy, new_frame, new_lift);
	if (old_chunk != new_chunk)	// In new chunk?
		switched_chunks(old_chunk, new_chunk);
	}

/*
 *	Find day's schedule for a given time-of-day.
 *
 *	Output:	index of schedule change.
 *		-1 if not found, or if a party member.
 */

int Npc_actor::find_schedule_change
	(
	int hour3			// 0=midnight, 1=3am, etc.
	)
	{
	if (Npc_actor::get_party_id() >= 0 || Npc_actor::is_dead_npc())
		return (-1);		// Fail if a party member or dead.
	for (int i = 0; i < num_schedules; i++)
		if (schedules[i].get_time() == hour3)
			return i;
	return -1;
	}

/*
 *	Update schedule at a 3-hour time change.
 */

void Npc_actor::update_schedule
	(
	Game_window *gwin,
	int hour3,			// 0=midnight, 1=3am, etc.
	int backwards			// Extra periods to look backwards.
	)
	{
	if (!gwin->get_objects(get_cx(), get_cy()))
		return;			// Not on the map.
	int i = find_schedule_change(hour3);
	if (i < 0)
		{			// Not found?  Look at prev.?
					// Always if noon of first day.
		long hour = gwin->get_total_hours();
		if (hour == 12 && !backwards)
			backwards++;
		while (backwards-- && i < 0)
			i = find_schedule_change((--hour3 + 8)%8);
		if (i < 0)
			return;
		if (schedule_type == schedules[i].get_type())
			return;		// Already in it.
		}
	stop();				// Stop moving.
	if (schedule)			// End prev.
		schedule->ending(schedules[i].get_type());
	Tile_coord dest = schedules[i].get_pos();
	if (!gwin->is_chunk_read(get_cx(), get_cy()) &&
	    !gwin->is_chunk_read(dest.tx/c_tiles_per_chunk,
						dest.ty/c_tiles_per_chunk))
		{			// Src, dest. are off the screen.
		move(dest.tx, dest.ty, dest.tz);
		set_schedule_type(schedules[i].get_type());
		return;
		}
					// Going to walk there.
	schedule_type = Schedule::walk_to_schedule;
	delete schedule;
	schedule = new Walk_to_schedule(this, dest, schedules[i].get_type());
	dormant = false;
	schedule->now_what();
	}

/*
 *	Render.
 */

void Npc_actor::paint
	(
	Game_window *gwin
	)
	{
	Actor::paint(gwin);		// Draw on screen.
	if (dormant && schedule)	// Resume schedule.
		{
		dormant = false;		// But clear out old entries first.??
		gwin->get_tqueue()->remove(this);
					// Force schedule->now_what().
					// DO NOT call now_what here!!!
		uint32 curtime = SDL_GetTicks();
		gwin->get_tqueue()->add(curtime, this, (long) gwin);
		set_action(new Null_action());
		}
	if (!nearby)			// Make sure we're in 'nearby' list.
		gwin->add_nearby_npc(this);
	}

/*
 *	Run usecode when double-clicked.
 */
void Npc_actor::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Converse, etc.
	Actor::activate(umachine, event);
	int i;				// Past 6:00pm first day?
	if (gwin->get_total_hours() >= 18 ||
					// Or party member/asleep?
	    (i = find_schedule_change(gwin->get_hour()/3)) < 0 ||
					// Or schedule is already correct?
	    schedules[i].get_type() == schedule_type)
		return;
	cout << "Setting '" << get_name() << "' to 1st schedule" << endl;
					// Maybe a delay here?  Okay for now.
	update_schedule(gwin, gwin->get_hour()/3);
	}

/*
 *	Handle a time event (for animation).
 */

void Npc_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	if (!action)			// Not doing anything?
		dormant = true;
	else
		{			// Do what we should.
		int delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			Game_window::get_game_window()->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			{
			set_action(0);
			if (schedule)
				if (dormant)
					schedule->im_dormant();
				else
					schedule->now_what();
			}
		}
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 *		Dormant is set if off screen.
 */

int Npc_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
	Game_window *gwin = Game_window::get_game_window();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
					// Get ->new chunk.
	Chunk_object_list *nlist = gwin->get_objects_safely(cx, cy);
	if (!nlist)			// Shouldn't happen!
		{
		stop();
		return (0);
		}
	nlist->setup_cache();		// Setup cache if necessary.
	int water, poison;		// Get tile info.
	get_tile_info(this, gwin, nlist, tx, ty, water, poison);
	int new_lift;			// Might climb/descend.
					// Just assume height==3.
	if (nlist->is_blocked(3, get_lift(), tx, ty, new_lift, 
							get_type_flags()))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
					// Offscreen, but not in party?
		if (!gwin->add_dirty(this) && Npc_actor::get_party_id() < 0)
			dormant = true;	// Go dormant.
		return (0);		// Done.
		}
	if (poison)
		Actor::set_flag((int) Obj_flags::poisoned);
	add_dirty(gwin);		// Set to repaint old area.
					// Get old chunk.
	Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
	movef(olist, nlist, tx, ty, frame, new_lift);
					// Offscreen, but not in party?
	if (!add_dirty(gwin, 1) && Npc_actor::get_party_id() < 0 &&
	    get_schedule_type() != Schedule::talk)
		{			// No longer on screen.
		stop();
		dormant = true;
		return (0);
		}
	return (1);			// Add back to queue for next time.
	}

/*
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Npc_actor::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	set_action(0);
	delete schedule;
	schedule = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
					// Store old chunk list.
	Chunk_object_list *olist = gwin->get_objects_safely(
							get_cx(), get_cy());
	Actor::remove_this(nodel);	// Remove.
	switched_chunks(olist, 0);
	cx = cy = 0xff;			// Set to invalid chunk coords.
	}

/*
 *	Update chunks' npc lists after this has moved.
 */

void Npc_actor::switched_chunks
	(
	Chunk_object_list *olist,	// Old chunk, or null.
	Chunk_object_list *nlist	// New chunk, or null.
	)
	{
	if (olist)			// Remove from old list.
		{
		if (this == olist->npcs)
			olist->npcs = next;
		else
			{
			Npc_actor *each, *prev = olist->npcs;
			while ((each = prev->next) != 0)
				if (each == this)
					{
					prev->next = next;
					break;
					}
				else
					prev = each;
			}
		}
	if (nlist)			// Add to new list.
		{
		next = nlist->npcs;
		nlist->npcs = this;
		}
	}

/*
 *	Move (teleport) to a new spot.
 */

void Npc_actor::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Store old chunk list.
	Chunk_object_list *olist = gwin->get_objects_safely(
							get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Chunk_object_list *nlist = gwin->get_objects(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
	}

/*
 *	Link body into global list, and set decay time.
 */

void Dead_body::link
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	uint32 cur_hour = gwin->get_total_hours();
	if (npc_num >= 0)		// Can be resurrected?  Give 3 days.
		decay_hour = cur_hour + 72;
	else				// Else give it several hours.
		decay_hour = cur_hour + 4 + rand()%4;
					// Store sorted by decay_hour.
	Dead_body *prev = 0, *each;
	for (each = in_world; each && each->decay_hour < decay_hour;
							each = each->next_body)
		prev = each;
	next_body = each;		// First one with later hour.
	prev_body = prev;
	if (prev)
		prev->next_body = this;
	else				// New head of chain.
		in_world = this;
	if (each)
		each->prev_body = this;
	}

/*
 *	Delete.
 */

Dead_body::~Dead_body
	(
	)
	{
	if (!decayable)
		return;
					// Remove from chain.
	if (next_body)
		next_body->prev_body = prev_body;
	if (prev_body)
		prev_body->next_body = next_body;
	else				// We're at start of list.
		in_world = next_body;
	}

/*
 *	Find all dead companions' bodies.
 *
 *	Output:	# found.  ->bodies returned in 'list'.
 */

int Dead_body::find_dead_companions
	(
	Dead_body *list[]		// List with at least 8 spots.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int cnt = 0;			// # found.
	for (Dead_body *each = in_world; each && cnt < 8; 
							each = each->next_body)
		{
		Actor *npc = gwin->get_npc(each->npc_num);
		if (npc && npc->get_party_id() >= 0)
			list[cnt++] = each;
		}
	return cnt;
	}

/*
 *	Delete all bodies.  (Should only be called after deleting chunks.)
 */

void Dead_body::delete_all
	(
	)
	{
	while (in_world)
		delete in_world;
	}

/*
 *	Remove all 'decayed' bodies.
 */

void Dead_body::decay
	(
	unsigned long hour		// Current game hour.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get those that are due.
	while (in_world && in_world->decay_hour <= hour)
		{
		gwin->add_dirty(in_world);
		in_world->remove_this();
		}
	}

/*
 *	Get # of NPC a body came from (or -1 if not known).
 */

int Dead_body::get_live_npc_num
	(
	)
	{
	return npc_num;
	}

/*
 *	See if it's blocked when trying to move to a new tile.
 *	And don't allow climbing/descending, at least for now.
 *
 *	Output: 1 if so, else 0.
 */

int Monster_actor::is_blocked
	(
	int destx, int desty		// Square we want to move to.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(this);
					// Get dim. in tiles.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int ztiles = info.get_3d_height();
	Tile_coord cur = get_abs_tile_coord();
	return Chunk_object_list::is_blocked(xtiles, ytiles, ztiles,
			cur, Tile_coord(destx, desty, cur.tz), get_type_flags());
	}

/*
 *	Set ->info.
 */

void Monster_actor::set_info
	(
	Monster_info *i			// May be 0.
	)
	{
	if (!i)
					// Not set?  Look it up.
		i = Game_window::get_game_window()->get_monster_info(
							get_shapenum());
	info = i;
	}

/*
 *	Delete.
 */

Monster_actor::~Monster_actor
	(
	)
	{
	if (creator)
		creator->monster_gone();
					// Remove from chain.
	if (next_monster)
		next_monster->prev_monster = prev_monster;
	if (prev_monster)
		prev_monster->next_monster = next_monster;
	else				// We're at start of list.
		in_world = next_monster;
	if (!Actor::is_dead_npc())	// Dying decrements count.
		in_world_cnt--;
	}

/*
 *	Delete all monsters.  (Should only be called after deleting chunks.)
 */

void Monster_actor::delete_all
	(
	)
	{
	while (in_world)
		delete in_world;
	in_world_cnt = 0;
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	0 if blocked.
 *		Dormant is set if off screen.
 */

int Monster_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
	Game_window *gwin = Game_window::get_game_window();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
					// Get ->new chunk.
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	nlist->setup_cache();		// Setup cache if necessary.
					// Blocked.
//Try is_blocked(t.tx, t.ty)+++++++
	if (is_blocked(cx*c_tiles_per_chunk + tx, cy*c_tiles_per_chunk + ty))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		if (!gwin->add_dirty(this))
			dormant = true;	// Off-screen.
		return (0);		// Done.
		}
	add_dirty(gwin);		// Set to repaint old area.
					// Get old chunk.
	Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
	movef(olist, nlist, tx, ty, frame, -1);
	if (!add_dirty(gwin, 1))
		{			// No longer on screen.
		stop();
		dormant = true;
		return (0);
		}
	return (1);			// Add back to queue for next time.
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Monster_actor::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume check.
	)
	{
	if (Npc_actor::add(obj, 1))	// Try to add to 'readied' spot.
		return (1);		// Successful.
					// Just add anything.
	return Container_game_object::add(obj, 1);
	}

/*
 *	Get total value of armor being worn.
 */

int Monster_actor::get_armor_points
	(
	)
	{
	Monster_info *inf = get_info();
					// Kind of guessing here.
	return Actor::get_armor_points() + (inf ? inf->armor/4 : 0);
	}

/*
 *	Get weapon value.
 */

Weapon_info *Monster_actor::get_weapon
	(
	int& points,
	int& shape
	)
	{
	Monster_info *inf = get_info();
					// Kind of guessing here.
	Weapon_info *winf = Actor::get_weapon(points, shape);
	if (!winf)			// No readied weapon?
		{			// Look up monster itself.
		shape = 0;
		Game_window *gwin = Game_window::get_game_window();
		winf = gwin->get_info(get_shapenum()).get_weapon_info();
		if (winf)
			points = winf->get_damage();
		else			// Guessing:
			points = inf ? inf->weapon/4 : 0;
		}
	return winf;
	}

/*
 *	We're dead.  We're removed from the world, but not deleted.
 */

void Monster_actor::die
	(
	)
	{
	Actor::die();
	if (creator)
		creator->monster_gone();
	creator = 0;
	Audio::get_ptr()->start_music_combat ( CSVictory, 0);
					// Got to delete this somewhere, but
					//   doing it here crashes.
	in_world_cnt--;			// So... Decrement 'live' count here.
	}

/*
 *	Another set of constants that should be in a data file (or that's
 *	probably already in one of the U7 data files), this one correlating
 *	a 'monster' shape with the food frame you get when you kill it.
 */
static int Monster_food[] = {
	498, 10,			// Chicken.
	500,  9,			// Cow - beef steaks.
	502,  14,			// Deer - meat.
	509, 12,			// Fish.
	811, 14,			// Rabbit - small leg.
	970, 8,				// Sheep - mutton.
	727, 23				// Horse - ribs.
	};

/*
 *	Find food frame for given monster shape.
 *
 *	Output:	Frame if found, else a random frame (0-31).
 */

static int Find_monster_food
	(
	int shnum			// Monster shape.
	)
	{
	const int cnt = sizeof(Monster_food)/(2*sizeof(Monster_food[0]));
	for (int i = 0; i < cnt; i++)
		if (Monster_food[2*i] == shnum)
			return Monster_food[2*i + 1];
	return rand()%32;
	}

/*
 *	Create an instance of a monster.
 */

Monster_actor *Monster_info::create
	(
	int chunkx, int chunky,		// Chunk to place it in.
	int tilex, int tiley,		// Tile within chunk.
	int lift			// Lift.
	)
	{
	Monster_actor *monster = new Monster_actor("", shapenum);
	monster->set_info(this);
	
	cout << monster->get_name() << " flags ";
	
	// Movement flags
	if ((flags >> fly)&1)
	{
		cout << "fly ";
		monster->set_type_flag(Actor::tf_fly);
	}
	if ((flags >> swim)&1)
	{
		cout << "swim ";
		monster->set_type_flag(Actor::tf_swim);
	}
	if ((flags >> walk)&1)
	{
		cout << "walk ";
		monster->set_type_flag(Actor::tf_walk);
	}
	if ((flags >> ethereal)&1)
	{
		cout << "ethereal ";
		monster->set_type_flag(Actor::tf_ethereal);
	}
	
	cout << endl;
	
					// Seems like the #'s are x4.
	monster->set_property(Actor::strength, strength/4);
					// Max. health = strength.
	monster->set_property(Actor::health, strength/4);
	monster->set_property(Actor::dexterity, dexterity/4);
	monster->set_property(Actor::intelligence, intelligence/4);
	monster->set_property(Actor::combat, combat/4);
					// ++++Armor?
					// Place in world.
	Game_window *gwin = Game_window::get_game_window();
	Chunk_object_list *olist = gwin->get_objects(chunkx, chunky);
	monster->movef(0, olist, tilex, tiley, 0, lift);
					// ++++++For now:
	if (flags & ((1<<walk)|(1<<fly)|(1<<swim)|(1<<ethereal)))
		monster->set_schedule_type(Schedule::loiter);
	else				// For the wounded men in bed:
		monster->set_schedule_type(Schedule::wait);
					// Get equipment.
	if (!equip_offset || equip_offset - 1 >= equip_cnt)
		return (monster);	// Out of range.
	Equip_record& rec = equip[equip_offset - 1];
	for (size_t i = 0;
			i < sizeof(equip->elements)/sizeof(equip->elements[0]);
			i++)
		{			// Give equipment.
		Equip_element& elem = rec.elements[i];
		if (!elem.shapenum || 1 + rand()%100 > elem.probability)
			continue;	// You lose.
		int frnum = (elem.shapenum == 377) ? 
					Find_monster_food(shapenum) : 0;
		monster->add_quantity(elem.quantity, elem.shapenum, c_any_qual,
								frnum);
		}
	return (monster);
	}
/*
 *	Create a sequence of frames.
 */

Frames_sequence::Frames_sequence
	(
	int cnt,			// # of frames.
	unsigned char *f		// List of frames.
	) : num_frames(cnt)
	{
	frames = new unsigned char[cnt];
	memcpy(frames, f, cnt);		// Copy in the list.
	}


