/*
 *	actors.cc - Game actors.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <iostream>			/* Debugging. */
#  include <cstdlib>
#  include <cstring>
#endif
#include <algorithm>		/* swap. */
#include "Astar.h"
#include "Audio.h"
#include "Gump_manager.h"
#include "Paperdoll_gump.h"
#include "Zombie.h"
#include "actions.h"
#include "actors.h"
#include "animate.h"
#include "bodies.h"
#include "cheat.h"
#include "chunks.h"
#include "combat.h"
#include "dir.h"
#include "egg.h"
#include "exult.h"
#include "frameseq.h"
#include "game.h"
#include "gamewin.h"
#include "imagewin.h"
#include "items.h"
#include "npctime.h"
#include "ready.h"
#include "ucmachine.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "mouse.h"
#include "servemsg.h"
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::memcpy;
using std::rand;
using std::string;
using std::swap;

Actor *Actor::editing = 0;

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
const char attack_frames1[4] = {3, 4, 5, 6};
const char attack_frames2[4] = {3, 7, 8, 9};
const char alligator_attack_frames[4] = {7, 8, 9};
const char sea_serpent_attack_frames[] = {13, 12, 11, 0, 1, 2, 3, 11, 12, 
								13, 14};
const char reaper_attack_frames[] = {7, 8, 9};
const char bee_attack_frames[] = {2, 9};
const char drake_attack_frames[] = {3, 8, 9};
const char scorpion_attack_frames[] = {7, 8, 9};
// inline int Is_attack_frame(int i) { return i >= 3 && i <= 9; }
inline int Is_attack_frame(int i) { return i == 6 || i == 9; }
inline int Get_dir_from_frame(int i)
	{ return ((((i&16)/8) - ((i&32)/32)) + 4)%4; }
Equip_record *Monster_info::equip = 0;
int Monster_info::equip_cnt = 0;
Monster_actor *Monster_actor::in_world = 0;
int Monster_actor::in_world_cnt = 0;

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
	size_t i;
	for (i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		properties[i] = 0;
	for (i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
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
	if (!winf || (ammo = winf->get_ammo_consumed()) == 0)
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
	// What about spell book????
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
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
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
 *	Try to store the weapon.
 */

void Actor::unready_weapon
	(
	int spot			// Lhand or rhand.
	)
	{
	Game_object *obj = spots[spot];
	if (!obj)
		return;
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(obj);
	if (!info.get_weapon_info())	// A weapon?
		return;
	if (!spots[belt])		// Belt free?
		{
		obj->remove_this(1);
		add_readied(obj, belt);
		}
	}

/*
 *	Add dirty rectangle(s).
 *
 *	Output:	0 if not on screen.
 */
#if !(defined(BEOS) || defined(ALPHA_LINUX_CXX))
#if __GNUG__ > 2
#else
inline 
#endif
#endif
int Actor::add_dirty
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
	Map_chunk *old_chunk, 
	Map_chunk *new_chunk, 
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
	    npc_num(num), face_num(num), party_id(-1), shape_save(-1), 
	    oppressor(-1), target(0), attack_mode(nearest),
	    schedule_type((int) Schedule::loiter), schedule(0),
	    dormant(true), dead(false), hit(false), combat_protected(false), 
	    user_set_attack(false), alignment(0),
	    two_handed(false), two_fingered(false), light_sources(0),
	    usecode_dir(0), siflags(0), type_flags(0), ident(0),
	    skin_color(-1), action(0), 
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
 *	NOTE:  Should be called every three hours.
 */

void Actor::use_food
	(
	)
	{
	int food = get_property((int) food_level);
	food -= 3;			// 1 level/hour should do.
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
	else switch (get_shapenum())
		{
	case 492:			// Alligator.
		which = alligator_attack_frames;
		cnt = sizeof(alligator_attack_frames);
		break;
	case 494:			// Bee.
		which = bee_attack_frames;
		cnt = sizeof(bee_attack_frames);
		break;
	case 505:			// Drake.
		which = drake_attack_frames;
		cnt = sizeof(drake_attack_frames);
		break;
	case 524:			// Reaper.
		which = reaper_attack_frames;
		cnt = sizeof(reaper_attack_frames);
		break;
	case 525:			// Sea serpent.
		which = sea_serpent_attack_frames;
		cnt = sizeof(sea_serpent_attack_frames);
		break;
	case 706:			// Scorpion.
		which = scorpion_attack_frames;
		cnt = sizeof(scorpion_attack_frames);
		break;
	default:
		which = attack_frames1;
		cnt = sizeof(attack_frames1);
		break;
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
	frames[(int) northeast] = frames[(int) north];
	frames[(int) south] = new Frames_sequence(3, south_frames);
	frames[(int) southwest] = frames[(int) south];
	frames[(int) east] = new Frames_sequence(3, east_frames);
	frames[(int) southeast] = frames[(int) east];
	frames[(int) west] = new Frames_sequence(3, west_frames);
	frames[(int) northwest] = frames[(int) west];
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
	dormant = false;		// 14-jan-2001 - JSF.
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
	if (Actor::is_dead())
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
					// Is leader following a path?
	bool leaderpath = leader->action && 
				leader->action->following_smart_path();
					// Get leader's distance from goal.
	int leaderdist = goal.distance(leaderpos);
					// Get his speed.
	int speed = leader->get_frame_time();
	if (!speed)			// Not moving?
		{
		speed = 100;
		if (goaldist < leaderdist)	// Closer than leader?
					// Delay a bit IF not moving.
			delay = (1 + leaderdist - goaldist)*100;
		}
	if (goaldist - leaderdist >= 5)
		speed -= 20;		// Speed up if too far.
	if (goaldist > 32 &&		// Getting kind of far away?
	    get_party_id() >= 0 &&	// And a member of the party.
	    !leaderpath)		// But leader is not following path.
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
	     (dist2lead >= 4 && !leader->is_moving()) || leaderpath) && 
	      get_party_id() >= 0 && 
	      (!is_moving() || !action || !action->following_smart_path()))
		{			// A little stuck?
#ifdef DEBUG
		cout << get_name() << " at distance " << dist2lead 
				<< " trying to catch up." << endl;
#endif
					// Don't try again for a second.
		next_path_time = SDL_GetTicks() + 1000;
		if (Map_chunk::is_blocked(goal, 3, get_type_flags()))
					// Find a free spot.
			goal = leader->find_unblocked_tile(1, 3);
		if (goal.tx == -1)	// No free spot?  Give up.
			{
			cout << "... but is blocked." << endl;
			return;
			}
		if (walk_path_to_tile(goal, speed - speed/4, 0))
			return;		// Success.
		else
			cout << "... but failed to find path." << endl;
		}
					// NOTE:  Avoid delay when moving,
					//  as it creates jerkiness.
	walk_to_tile(goal, speed, delay);
	}

/*
 *	Approach another actor from offscreen.
 *
 *	Output:	0 if failed.
 */

int Actor::approach_another
	(
	Actor *other,
	bool wait			// If true, game hangs until arrival.
	)
	{
	Tile_coord startdest = other->get_abs_tile_coord();
	Tile_coord dest(-1, -1, -1);	// Look outwards for free spot.
	for (int i = 2; dest.tx == -1 && i < 8; i++)
		dest = Game_object::find_unblocked_tile(startdest, i);
	if (dest.tx == -1)
		return 0;
					// Where are we now?
	Tile_coord src = get_abs_tile_coord();
	Game_window *gwin = Game_window::get_game_window();
	if (!gwin->get_win_tile_rect().has_point(src.tx, src.ty))
					// Off-screen?
		src = Tile_coord(-1, -1, 0);
	Actor_action *action = new Path_walking_actor_action();
	if (!action->walk_to_tile(src, dest, get_type_flags()))
		{
		delete action;
		return 0;
		}
	set_action(action);
	start(150);			// Walk fairly fast.
	if (wait)			// Only wait 1/10 sec.
		Wait_for_arrival(this, dest, 100);
	return 1;
	}

/*
 *	Get information about a tile that an actor is about to step onto.
 */

#if !defined(BEOS) && !defined(ALPHA_LINUX_CXX) && !defined(_MSC_VER)
#if __GNUG__ < 3
inline
#endif
#endif
void Actor::get_tile_info
	(
	Actor *actor,			// May be 0 if not known.
	Game_window *gwin,
	Map_chunk *nlist,	// Chunk.
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

void Actor::set_target
	(
	Game_object *obj,
	bool start_combat		// If true, set sched. to combat.
	)
	{
	target = obj;
	if (start_combat && (schedule_type != Schedule::combat || !schedule))
		set_schedule_type(Schedule::combat);
	}

/*
 *	Works out if an item can be readied in a spot
 *
 *	Output:	true if it does fit, or false if it can't
 */

bool Actor::fits_in_spot (Game_object *obj, int spot, FIS_Type type)
{
	// If occupied, can't place
	if (spots[spot])
		return false;
	// If want to use 2h or a 2h is already equiped, can't go in right
	else if ((type == FIS_2Hand || two_handed) && spot == rhand)
		return false;
	// If want to use 2f or a 2f is already equiped, can't go in right
	else if ((type == FIS_2Finger || two_fingered) && spot == rfinger)
		return false;
	// Can't use 2h in left if right occupied
	else if (type == FIS_2Hand && spot == lhand && spots[rhand])
		return false;
	// Can't use 2f in left if right occupied
	else if (type == FIS_2Finger && spot == lfinger && spots[rfinger])
		return false;
	// If in left or right hand allow it
	else if (spot == lhand || spot == rhand)
		return true;
	// Special Checks for Belt
	else if (spot == belt)
	{
		if (type == FIS_Spell)
			return true;
		else if (Game::get_game_type() == BLACK_GATE)
		{
			if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), back2h_spot))
				return true;
			else if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), shield_spot))
				return true;
		}
	}

	// Lastly if we have gotten here, check the paperdoll table 
	return Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), spot);
}

/*
 *	Find the spot(s) where an item would prefer to be readied
 *
 *	Output:	prefered slot, alternative slot, FIS_type
 */

void Actor::get_prefered_slots
	(
	Game_object *obj,
	int &prefered,
	int &alternate,
	FIS_Type &fistype
	)
{

	Shape_info& info = Game_window::get_game_window()->get_info(obj);

	// Defaults
	fistype = FIS_Other;
	prefered = lhand;
	alternate = lhand;

	if (Game::get_game_type() == BLACK_GATE)
	{
		Ready_type type = (Ready_type) info.get_ready_type();
		
		switch (type)
		{
			// Weapons, Sheilds, Spells, misc stuff
			default:
			if (type == spell || type == other_spell) fistype = FIS_Spell;
			else if (type == two_handed_weapon) fistype = FIS_2Hand;

			if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), rhand))
				prefered = rhand;
			else if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), back))
				prefered = back;
			else
				alternate = rhand;
			break;


			case gloves:
			fistype = FIS_2Finger;

			case ring:
			prefered = lfinger;
			alternate = rfinger;
			break;

			case neck_armor:
			prefered = neck;
			break;
				
			case torso_armor:
			prefered = torso;
			break;
				
			case ammunition:
			prefered = ammo;
			break;
			
			case head_armor:
			prefered = head;
			break;
				
			case leg_armor:
			prefered = legs;
			break;
				
			case foot_armor:
			prefered = feet;
			break;
		}
	}
	else if (Game::get_game_type() == SERPENT_ISLE)	// Serpent Isle Types
	{
		Ready_type_SI type = (Ready_type_SI) info.get_ready_type();
		
		switch (type)
		{
			// Weapons, Sheilds, Spells, misc stuff
			default:
			if (type == spell_si|| type == other_spell_si) fistype = FIS_Spell;
			else if (type == two_handed_si) fistype = FIS_2Hand;

			if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), rhand))
				prefered = rhand;
			else
				alternate = rhand;
			break;

			case other:
			if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), back2h_spot))
				prefered = back2h_spot;
			else if (Paperdoll_gump::IsObjectAllowed (obj->get_shapenum(), obj->get_framenum(), back))
				prefered = back;
			else 
				alternate = rhand;
			break;


			case helm_si:
			prefered = head;
			break;

			case gloves_si:
			prefered = hands2_spot;
			break;

			case boots_si:
			prefered = feet;
			break;

			case leggings_si:
			prefered = legs;
			break;
			
			case amulet_si:
			prefered = neck;
			break;
				
			case armour_si:
			prefered = torso;
			break;

			case ring_si:
			prefered = lfinger;
			alternate = rfinger;
			break;

			case ammo_si:
			prefered = ammo;
			break;
	
			case cloak_si:
			prefered = cloak_spot;
			break;

			case usecode_container_si:
			prefered = ucont_spot;
			break;
			
			case earrings_si:
			prefered = ears_spot;
			break;

			case belt_si:
			prefered = belt;
			break;
			
			case backpack_si:
			prefered = back;
			break;
		}
	}
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
	int prefered;
	int alternate;
	FIS_Type type;
	bool SI = Game::get_game_type() == SERPENT_ISLE;

	// Get the preferences
	get_prefered_slots (obj, prefered, alternate, type);
	
	// Check Prefered
	if (fits_in_spot (obj, prefered, type)) return prefered;
	// Alternate
	else if (fits_in_spot (obj, alternate, type)) return alternate;
	// Belt
	else if (fits_in_spot (obj, belt, type)) return belt;
	// Back - required???
	else if (fits_in_spot (obj, back, type)) return back;
	// Back2h
	else if (SI && fits_in_spot (obj, back2h_spot, type)) return back2h_spot;
	// Sheild Spot
	else if (SI && fits_in_spot (obj, back2h_spot, type)) return shield_spot;
	// Left Hand
	else if (fits_in_spot (obj, lhand, type)) return lhand;
	// Right Hand
	else if (fits_in_spot (obj, rhand, type)) return rhand;

	return -1;
}

/*
 *	Get previous schedule type.
 *
 *	Output:	Prev. schedule #, or -1 if not known.
 */

int Actor::get_prev_schedule_type
	(
	)
	{
	return schedule ? schedule->get_prev_type() : -1;
	}

/*
 *	Restore actor's schedule after reading.  This CANNOT be called from
 *	a constructor, since it may call virtual methods when setting up
 *	the schedule.
 */

void Actor::restore_schedule
	(
	)
	{
					// Make sure it's in valid chunk.
	Map_chunk *olist = Game_window::get_game_window()->
				get_chunk_safely(get_cx(), get_cy());
					// Activate schedule if not in party.
	if (olist && get_party_id() < 0)
		{
		if (next_schedule != 255 && 
				schedule_type == Schedule::walk_to_schedule)
			set_schedule_and_loc(next_schedule, schedule_loc);
		else
			set_schedule_type(schedule_type);
		}
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
	delete schedule;		// Done with the old.
	schedule = newsched;
	if (!schedule)
		switch ((Schedule::Schedule_types) new_schedule_type)
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
		case Schedule::hound:
			schedule = new Hound_schedule(this);
			break;
		case Schedule::loiter:
		case Schedule::graze:	// For now.
			schedule = new Loiter_schedule(this);
			break;
		case Schedule::wander:
			schedule = new Wander_schedule(this);
			break;
		case Schedule::blacksmith:
			schedule = new Forge_schedule(this);
			break;
		case Schedule::sleep:
			schedule = new Sleep_schedule(this);
			break;
		case Schedule::wait:
			schedule = new Wait_schedule(this);
			break;
		case Schedule::eat:		// For now.
		case Schedule::sit:
			schedule = new Sit_schedule(this);
			break;
		case Schedule::bake:
			schedule = new Bake_schedule(this);
			break;
		case Schedule::sew:
			schedule = new Sew_schedule(this);
			break;
		case Schedule::shy:
			schedule = new Shy_schedule(this);
			break;
		case Schedule::lab:
			schedule = new Lab_schedule(this);
			break;
		case Schedule::thief:		// Just face north, for now.
			gwin->add_dirty(this);
			unready_weapon(lhand);	// For Krieg in Empath Abbey.
			unready_weapon(rhand);
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
			schedule = new Desk_schedule(this);
			break;
		case Schedule::walk_to_schedule:
			cerr << "Attempted to set a \"walk to schedule\" activity for NPC "<< get_npc_num() << endl;
			break;
		default:
			break;
			}
					// Set AFTER creating new schedule.
	schedule_type = new_schedule_type;

	// Reset Next Schedule
	schedule_loc = Tile_coord(0,0,0);
	next_schedule = 255;

	if (!gwin->is_chunk_read(get_cx(), get_cy()))
		dormant = true;		// Chunk hasn't been read in yet.
	else if (schedule)		// Try to start it.
		{
		dormant = false;
		schedule->now_what();
		}
	}

/*
 *	Set new schedule by type AND location.
 */

void Actor::set_schedule_and_loc (int new_schedule_type, Tile_coord dest,
				int delay)	// -1 for random delay.
{
	Game_window *gwin = Game_window::get_game_window();
//Causes bugs in SI.  Maybe not needed.	if (!gwin->get_chunk_safely(get_cx(), get_cy()))
//		return;			// Not on the map.

	stop();				// Stop moving.
	if (schedule)			// End prev.
		schedule->ending(new_schedule_type);

	if (!gwin->is_chunk_read(get_cx(), get_cy()) &&
	    !gwin->is_chunk_read(dest.tx/c_tiles_per_chunk,
						dest.ty/c_tiles_per_chunk))
		{			// Src, dest. are off the screen.
		move(dest.tx, dest.ty, dest.tz);
		set_schedule_type(new_schedule_type);
		return;
		}
					// Going to walk there.
	schedule_loc = dest; 
	next_schedule = new_schedule_type;
	schedule_type = Schedule::walk_to_schedule;
	delete schedule;
	schedule = new Walk_to_schedule(this, dest, next_schedule, delay);
	dormant = false;
	schedule->now_what();
}

/*
 *	Render.
 */

void Actor::paint
	(
	Game_window *gwin
	)
	{
	if (!(flags & (1L << Obj_flags::dont_render)) ||
	    Game::get_game_type() == SERPENT_ISLE)
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
		if (hit)		// Want a momentary red outline.
			gwin->paint_hit_outline(xoff, yoff,
					get_shapenum(), get_framenum());
		else if (flags & ((1L<<Obj_flags::protection) | 
						(1L << Obj_flags::poisoned)))
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
			gwin->paint_shape(xoff, yoff, shnum, weapon_frame);
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
/* Weapon frames:
	0 - normal item
	1 - in hand, actor facing north/south
	2 - attacking (pointing north)
	3 - attacking (pointing east)
	4 - attacking (pointing south)
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
	int myframe = get_framenum();
	gwin->get_info(this).get_weapon_offset(myframe & 0x1f, actor_x,
			actor_y);
	// Get offsets for weapon shape
	// NOTE: when combat is implemented, weapon frame should depend on
	// actor's current attacking frame
	weapon_frame = 1;
	int baseframe = myframe&0xf;
	if (gwin->in_combat() && Is_attack_frame(baseframe))
		{			// Get direction (0-4).
		int dir = Get_dir_from_frame(myframe);
		if (dir < 3)		// N, E, S?
			weapon_frame = 2 + dir;
		else			// W = N reflected.
			weapon_frame = 2 | 32;
		}
	gwin->get_info(weapon).get_weapon_offset(weapon_frame&0xf, wx,
			wy);
	// actor_x will be 255 if (for example) the actor is lying down
	// wx will be 255 if the actor is not holding a proper weapon
	if(actor_x != 255 && wx != 255)
		{			// Store offsets rel. to NPC.
		weapon_x = wx - actor_x;
		weapon_y = wy - actor_y;
		// Need to swap offsets if actor's shape is reflected
		if((get_framenum() & 32))
			{
			swap(weapon_x, weapon_y);
					// Combat frames are already done.
			if (weapon_frame == 1)
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
	Game_window *gwin = Game_window::get_game_window();
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		editing = 0;
		Tile_coord t = get_abs_tile_coord();
		unsigned long addr = (unsigned long) this;
		int num_schedules;	// Set up schedule-change list.
		Schedule_change *changes;
		get_schedules(changes, num_schedules);
		Serial_schedule schedules[8];
		for (int i = 0; i < num_schedules; i++)
			{
			schedules[i].time = changes[i].get_time();
			schedules[i].type = changes[i].get_type();
			Tile_coord p = changes[i].get_pos();
			schedules[i].tx = p.tx;
			schedules[i].ty = p.ty;
			}
		if (Npc_actor_out(client_socket, addr, t.tx, t.ty, t.tz,
			get_shapenum(), get_framenum(), get_face_shapenum(),
			name, npc_num, ident, usecode, properties, attack_mode,
			alignment, flags, siflags, type_flags,
				num_schedules, schedules) != -1)
			{
			cout << "Sent npc data to ExultStudio" << endl;
			editing = this;
			}
		else
			cout << "Error sending npc data to ExultStudio" <<endl;
		return;
		}
#endif
	// We are serpent if we can use serpent isle paperdolls
	bool serpent = Game::get_game_type()==SERPENT_ISLE||
		(gwin->can_use_paperdolls() && gwin->get_bg_paperdolls());
	
	bool show_party_inv = gwin->get_gump_man()->showing_gumps(true) || gwin->in_combat();

	if (!npc_num ||		// Avatar
			(show_party_inv && get_party_id() >= 0 && // Party
			(serpent || (npc_num >= 1 && npc_num <= 10))) ||
			(cheat.in_pickpocket() && event == 1))		// Pickpocket cheat && double click
		show_inventory();
					// Asleep (but not awakened)?
	else if ((get_schedule_type() == (int) Schedule::sleep &&
		(get_framenum()&0xf) == Actor::sleep_frame) ||
		 get_flag(Obj_flags::asleep))
		return;
					// Usecode
					// Failed copy-protection?
	else if (serpent &&
		 gwin->get_main_actor()->get_flag(Obj_flags::confused))
		umachine->call_usecode(0x63d, this,
			(Usecode_machine::Usecode_events) event);	
	else if (usecode == -1)
		umachine->call_usecode(get_shapenum(), this,
			(Usecode_machine::Usecode_events) event);
	else if (party_id >= 0 || !gwin->is_time_stopped())
		umachine->call_usecode(usecode, this, 
			(Usecode_machine::Usecode_events) event);
	
	}

/*
 *	Message to update from ExultStudio.
 */

void Actor::update_from_studio
	(
	unsigned char *data,
	int datalen
	)
	{
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, face;
	std::string name;
	short npc_num, ident;
	int usecode;
	short properties[12];
	short attack_mode, alignment;
	unsigned long oflags;		// Object flags.
	unsigned long siflags;		// Extra flags for SI.
	unsigned long type_flags;	// Movement flags.
	short num_schedules;
	Serial_schedule schedules[8];
	if (!Npc_actor_in(data, datalen, addr, tx, ty, tz, shape, frame,
		face, name, npc_num, ident, usecode, 
			properties, attack_mode, alignment,
			oflags, siflags, type_flags, num_schedules, schedules))
		{
		cout << "Error decoding npc" << endl;
		return;
		}
	Actor *npc = (Actor *) addr;
	if (npc && npc != editing)
		{
		cout << "Npc from ExultStudio is not being edited" << endl;
		return;
		}
	editing = 0;
	Game_window *gwin = Game_window::get_game_window();
	if (!npc)			// Create a new one?
		{
		int x, y;
		if (!Get_click(x, y, Mouse::hand, 0))
			{
			if (client_socket >= 0)
				Send_data(client_socket, Exult_server::cancel);
			return;
			}
					// Create.  Gets initialized below.
		npc = new Npc_actor(name, shape, frame, usecode);
		npc->set_invalid();	// Set to invalid position.
		int lift;		// Try to drop at increasing hts.
		for (lift = 0; lift < 12; lift++)
			if (gwin->drop_at_lift(npc, x, y, lift))
				break;
		if (lift == 12)
			{
			if (client_socket >= 0)
				Send_data(client_socket, Exult_server::cancel);
			delete npc;
			return;
			}
		npc->npc_num = gwin->add_npc(npc);
		if (npc->npc_num != npc_num)
			cerr << "New NPC was assigned a different #" << endl;
		if (client_socket >= 0)
			Send_data(client_socket, Exult_server::user_responded);
		}
	else				// Old.
		{
		npc->add_dirty(gwin);
		npc->set_shape(shape, frame);
		npc->add_dirty(gwin);
		npc->usecode = usecode;
		}
	npc->face_num = face;
	npc->set_ident(ident);
	for (int i = 0; i < 12; i++)
		npc->set_property(i, properties[i]);
	npc->set_attack_mode((Actor::Attack_mode) attack_mode);
	npc->set_alignment(alignment);
	npc->flags = oflags;
	npc->siflags = siflags;
	npc->type_flags = type_flags;
	Schedule_change *scheds = num_schedules ? 
				new Schedule_change[num_schedules] : 0;
	for (int i = 0; i < num_schedules; i++)
		scheds[i].set(schedules[i].tx, schedules[i].ty, 
				schedules[i].type, schedules[i].time);
	npc->set_schedules(scheds, num_schedules);
	cout << "Npc updated" << endl;
#endif
	}


void Actor::show_inventory()
{
	Game_window *gwin = Game_window::get_game_window();
	Gump_manager *gump_man = gwin->get_gump_man();

	int shapenum = inventory_shapenum();
	if (shapenum)
		gump_man->add_gump(this, shapenum);
}

int Actor::inventory_shapenum()
{
	Game_window *gwin = Game_window::get_game_window();

	// We are serpent if we can use serpent isle paperdolls
	bool serpent = Game::get_game_type()==SERPENT_ISLE||(gwin->can_use_paperdolls() && gwin->get_bg_paperdolls());
	
	if (!npc_num && !serpent)	// Avatar No paperdolls
		return (ACTOR_FIRST_GUMP);
	else if (!npc_num && serpent)	// Avatar Paperdolls
		return (123);
					// Gump/combat mode?
					// Show companions' pictures. (BG)
	else if (get_party_id() >= 0 &&
		 npc_num >= 1 && npc_num <= 10 && !serpent)
			return (ACTOR_FIRST_GUMP + 1 + npc_num);
	// Show companions' pictures. (SI)
	else if (get_party_id() >= 0 && serpent)
		return (123);
	// Pickpocket Cheat Female no paperdolls
	else if (!serpent && Paperdoll_gump::IsNPCFemale(this->get_shapenum()))
		return (66);
	// Pickpocket Cheat Male no paperdolls
	else if (!serpent && !Paperdoll_gump::IsNPCFemale(this->get_shapenum()))
		return (65);
	// Pickpocket Cheat paperdolls
	else /* if (serpent) */
		return (123);
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
	if (get_party_id() >= 0 ||	// In party?
	    this == Game_window::get_game_window()->get_main_actor())
		return (add(obj));	// We'll take it.
	else
		return 0;
	}

/*
 *	Get name.
 */

string Actor::get_name
	(
	) const
	{
	return !get_flag(Obj_flags::met)?Game_object::get_name():get_npc_name();
	}

/*
 *	Get npc name.
 */

string Actor::get_npc_name
	(
	) const
	{
	return name.empty() ? Game_object::get_name() : name;
	}

/*
 *	Set npc name.
 */

void Actor::set_npc_name(const char *n)
{
	name = n;
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
		if (gwin->get_gump_man()->showing_gumps())
			gwin->set_all_dirty();
		}
	}

/*
 *	A class whose whole purpose is to clear the 'hit' flag.
 */
class Clear_hit : public Time_sensitive
	{
public:
	Clear_hit()
		{  }
	virtual void handle_event(unsigned long curtime, long udata);
	};
void Clear_hit::handle_event(unsigned long curtime, long udata)
	{ 
	Actor *a = (Actor *) udata;
	a->hit = false;
	Game_window *gwin = Game_window::get_game_window();
	a->add_dirty(gwin);
	delete this;
	}

/*
 *	This method should be called to decrement health from attacks, traps.
 */

void Actor::reduce_health
	(
	int delta,			// # points to lose.
	Actor *attacker			// Attacker, or null.
	)
	{
	if (cheat.in_god_mode() && ((party_id != -1) || (npc_num == 0)))
		return;
					// Watch for Skara Brae ghosts.
	if (npc_num > 0 && Game::get_game_type() == BLACK_GATE &&
	    Game_window::get_game_window()->get_info(this).has_translucency())
		return;
	int oldhp = properties[(int) health];
	int maxhp = properties[(int) strength];
	int val = oldhp - delta;
	properties[(int) health] = val;
	Game_window *gwin = Game_window::get_game_window();
	if (this == gwin->get_main_actor() && val < maxhp/8 &&
					// Flash red if Avatar badly hurt.
	    rand()%2)
		gwin->flash_palette_red();
	else
		{
		hit = true;		// Flash red outline.
		add_dirty(gwin);
		Clear_hit *c = new Clear_hit();
		gwin->get_tqueue()->add(SDL_GetTicks() + 200, c, (long) this);
		}
	Game_object_vector vec;		// Create blood.
	const int blood = 912;
	if (delta >= 3 && rand()%2 && find_nearby(vec, blood, 1, 0) < 2)
		{			// Create blood where actor stands.
		Game_object *bobj = gwin->create_ireg_object(blood, 0);
		bobj->move(get_abs_tile_coord());
		}
	if (Actor::is_dying())
		{
		if (Game::get_game_type() == SERPENT_ISLE && usecode >= 0 &&
					// SI 'tournament'?
		    get_flag(Obj_flags::si_tournament))
			{
					// Only if one or other is in party:
			Actor *c = gwin->get_camera_actor();
			if (get_flag(Obj_flags::in_party) || this == c ||
				(attacker && 
				 (attacker->get_flag(Obj_flags::in_party) ||
							attacker == c)))
				gwin->get_usecode()->call_usecode(
					usecode, this, Usecode_machine::died);

				// Still 'tournament'?  Set hp = 1.
			if (get_flag(Obj_flags::si_tournament) &&
			    get_property((int) health) < 1)
				set_property((int) health, 1);
			}
		else
			die();

		}
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

	// Hack :)
	if (flag == Obj_flags::dont_render && Game::get_game_type() == SERPENT_ISLE)
		set_siflag(dont_move);

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
	if (gwin->get_gump_man()->showing_gumps())
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
	if (flag == Obj_flags::dont_render && Game::get_game_type() == SERPENT_ISLE)
		clear_siflag(dont_move);
	if (flag >= 0 && flag < 32)
		flags &= ~((uint32) 1 << flag);
	else if (flag >= 32 && flag < 64)
		flags2 &= ~((uint32) 1 << (flag-32));
	Game_window *gwin = Game_window::get_game_window();
	if (flag == Obj_flags::invisible)	// Restore normal palette.
		gwin->set_palette();
	else if (flag == Obj_flags::asleep)
		{
		if (schedule_type == Schedule::sleep)
			set_schedule_type(Schedule::stand);
		else if ((get_framenum()&0xf) == Actor::sleep_frame)
			{
			add_dirty(gwin);
			set_frame(Actor::standing);
			add_dirty(gwin);
			}
		}
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

void Actor::call_readied_usecode
	(
	Game_window *gwin,
	int index,
	Game_object *obj,
	int eventid
	)
	{
	if (index != Actor::rfinger && index != Actor::lfinger && 
	    index != Actor::belt &&
	    index != Actor::neck &&
	    index != Actor::head && index != Actor::hands2_spot &&
	    index != Actor::lhand && index != Actor::rhand)
		return;
	if (obj->get_shapenum() == 297)	// Fix special case:  ring of protect.
		{
		if (eventid == Usecode_machine::readied)
			Actor::set_flag(Obj_flags::protection);
		else
			Actor::clear_flag(Obj_flags::protection);
		return;
		}
	Shape_info& info = gwin->get_info(obj);
	if (info.get_shape_class() != Shape_info::container)
		{
		Ready_type type = (Ready_type) info.get_ready_type();
		if (type != other)
			gwin->get_usecode()->call_usecode(obj->get_shapenum(),
			    obj, (Usecode_machine::Usecode_events) eventid);
		}
	}

/*
 *	Should be called after actors and usecode are initialized.
 */

void Actor::init_readied
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (spots[lfinger])
		call_readied_usecode(gwin, lfinger, spots[lfinger],
						Usecode_machine::readied);
	if (spots[rfinger])
		call_readied_usecode(gwin, rfinger, spots[rfinger],
						Usecode_machine::readied);
	if (spots[belt])
		call_readied_usecode(gwin, belt, spots[belt],
						Usecode_machine::readied);
	if (spots[neck])
		call_readied_usecode(gwin, neck, spots[neck],
						Usecode_machine::readied);
	if (spots[head])
		call_readied_usecode(gwin, head, spots[head],
						Usecode_machine::readied);
	if (spots[hands2_spot])
		call_readied_usecode(gwin, hands2_spot, 
				spots[hands2_spot], Usecode_machine::readied);
	if (spots[lhand])
		call_readied_usecode(gwin, lhand, spots[lhand],
						Usecode_machine::readied);
	if (spots[rhand])
		call_readied_usecode(gwin, rhand, spots[rhand],
						Usecode_machine::readied);
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
					// Note:  gwin->drop() also does this,
					//   but it needs to be done before
					//   removal too.
	if (!gwin->get_usecode()->in_usecode())
		call_readied_usecode(gwin, index, obj,
						Usecode_machine::unreadied);
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
	int index, a; 
	FIS_Type type;
	get_prefered_slots (obj, index, a, type);
	index = find_best_spot(obj);// Where should it go?
		
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
		return dont_check ? Container_game_object::add(obj, dont_check)
					: 0;
		}
					// Add to ourself.
	if (!Container_game_object::add(obj, 1))
		return (0);

	if (type == FIS_2Hand)		// Two-handed?
		two_handed = true;
	if (type == FIS_2Finger)	// Gloves?
		index = lfinger;

	spots[index] = obj;		// Store in correct spot.
	obj->set_chunk(0, 0);		// Clear coords. (set by gump).
	Game_window *gwin = Game_window::get_game_window();
					// (Readied usecode now in drop().)
	if (gwin->get_info(obj).is_light_source())
		light_sources++;
	return (1);
	}

/*
 *	Add to given spot.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::add_readied
	(
	Game_object *obj,
	int index,			// Spot #.
	int dont_check,
	int force_pos
	)
{

	// Is Out of range?
	if (index < 0 || index >= (int)(sizeof(spots)/sizeof(spots[0])))
		return (0);		

	// Already something there? Try to drop into it.
	if (spots[index]) return (spots[index]->drop(obj));

	int prefered;
	int alternate;
	FIS_Type type;

	// Get the preferences
	get_prefered_slots (obj, prefered, alternate, type);
	
	// Check Prefered
	if (!fits_in_spot (obj, index, type) && !force_pos) return 0;

	// No room, or too heavy.
	if (!Container_game_object::add(obj, 1)) return 0;

	// Set the spot to this object
	spots[index] = obj;

	// Clear coords. (set by gump).
	obj->set_chunk(0, 0);

	// Must be a two-handed weapon.
	if (type == FIS_2Hand && index == lhand) two_handed = true;

	// Must be gloves
	if (type == FIS_2Finger && index == lfinger) two_fingered = true;

	Game_window *gwin = Game_window::get_game_window();

	// Usecode?  NOTE:  Done in gwin->drop() now.
//	if (!dont_check)
//		call_readied_usecode(gwin, index, obj,
//						Usecode_machine::readied);

	// Lightsource?
	if (gwin->get_info(obj).is_light_source()) light_sources++;

	return 1;
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
			if (!Map_chunk::is_blocked(
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
					// Same for ammo.
	if (ammo_shape == 0x238 && Game::get_game_type() == SERPENT_ISLE)
					// KLUDGE:  putting Draygan to sleep.
		gwin->get_usecode()->call_usecode(0x7e1, this,
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
	int sfx;			// Play 'hit' sfx.
	if (winf && (sfx = winf->get_hitsfx()) >= 0 &&
					// But only if Ava. involved.
	    (this == gwin->get_main_actor() || 
				attacker == gwin->get_main_actor()))
		Audio::get_ptr()->play_sound_effect(sfx);
	int oldhealth = properties[(int) health];
	int maxhealth = properties[(int) strength];

	if (instant_death)		//instant death
		hp = properties[(int) health] + properties[(int) strength] + 1;
	int newhp = oldhealth - hp;	// Subtract from health.

	if (oldhealth >= maxhealth/2 && newhp < maxhealth/2 && rand()%3 != 0)
					// A little oomph.
		if (instant_death)
			say("\"Cheater!\"");
					// Goblin?
		else if (Game::get_game_type() == SERPENT_ISLE &&
			 (get_shapenum() == 0x1de ||
			  get_shapenum() == 0x2b3 ||
			  get_shapenum() == 0x2d5 ||
			  get_shapenum() == 0x2e8))
			say(0x4d2, 0x4da);
		else
			say(first_ouch, last_ouch);

	reduce_health(hp, attacker);
	
	string name = "<trap>";
	if (attacker)
		name = attacker->get_name();

	cout << name << " hits " << get_name() <<
		" for " << hp << " hit points, leaving " <<
		properties[(int) health] << " remaining" << endl;
//	cout << "Attack damage was " << hp << " hit points, leaving " << 
//		properties[(int) health] << " remaining" << endl;
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
	if (attacker)
		{ 
		if (attacker->get_schedule_type() == Schedule::duel)
			return this;	// Just play-fighting.
		set_oppressor(attacker->get_npc_num());
		if (is_combat_protected() && Actor::get_party_id() >= 0 &&
		    rand()%5 == 0)
			say(first_need_help, last_need_help);
		}
					// Watch for Skara Brae ghosts.
	if (npc_num > 0 && Game::get_game_type() == BLACK_GATE &&
	    Game_window::get_game_window()->get_info(this).has_translucency())
		return this;
	figure_hit_points(attacker, weapon_shape, ammo_shape);
	if (attacker && is_dead())
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
	dead = true;
	int shnum = get_shapenum();
					// Special case:  Hook, Dracothraxus.
	if (((shnum == 0x1fa || (shnum == 0x1f8 && Is_draco(this))) && 
	    Game::get_game_type() == BLACK_GATE))
		{			// Exec. usecode before dying.
		gwin->get_usecode()->call_usecode(shnum, this, 
					Usecode_machine::internal_exec);
		if (get_cx() == 255)	// Invalid now?
			return;
		}
	properties[(int) health] = -50;
	int frnum;			// Lookup body shape/frame.
	if (!Body_lookup::find(get_shapenum(), shnum, frnum))
		{
		shnum = 400;
		frnum = 3;
		}
					// Put body here.
	Dead_body *body = new Dead_body(shnum, frnum, 0, 0, 0, 
					npc_num > 0 ? npc_num : -1);
	if (npc_num > 0)
		{
		body->set_quality(1);	// Flag for dead body of NPC.
		gwin->set_body(npc_num, body);
		}
					// Tmp. monster => tmp. body.
	if (get_flag(Obj_flags::is_temporary))
		body->set_flag(Obj_flags::is_temporary);
	body->move(pos);
					// Okay to take its contents.
	body->set_flag_recursively(Obj_flags::okay_to_take);
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

	add_dirty(gwin);		// Want to repaint area.
	delete_contents();      // remove what's left of inventory
	remove_this(1);			// Remove (but don't delete this).
	set_invalid();

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
	Game_window *gwin = Game_window::get_game_window();
	if (body->get_owner() ||	// Must be on ground.
	    npc_num <= 0 || gwin->get_body(npc_num) != body)
		return (0);
	gwin->set_body(npc_num, 0);	// Clear from gwin's list.
	Game_object *item;		// Get back all the items.
	while ((item = body->get_objects().get_first()) != 0)
		{
		body->remove(item);
		add(item, 1);		// Always succeed at adding.
		}
	gwin->add_dirty(body);		// Need to repaint here.
	Tile_coord pos = body->get_abs_tile_coord();
	body->remove_this();		// Remove and delete body.
	move(pos);			// Move back to life.
					// Restore health to max.
	properties[(int) health] = properties[(int) strength];
	dead = false;
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
		    npc->is_dead())
			continue;
		int sched = npc->get_schedule_type();
					// Skip if in combat or set to 'wait'.
		if (sched != Schedule::combat &&
		    sched != Schedule::wait &&
					// Loiter added for SI.
		    sched != Schedule::loiter)
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
	Map_chunk *nlist = gwin->get_chunk(cx, cy);
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
	if (poison && new_lift == 0)
		Actor::set_flag((int) Obj_flags::poisoned);
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty(gwin);		/// Set to update old location.
					// Get old chunk, old tile.
	Map_chunk *olist = gwin->get_chunk(get_cx(), get_cy());
	Tile_coord oldtile = get_abs_tile_coord();
					// Move it.
	Actor::movef(olist, nlist, tx, ty, frame, new_lift);
	add_dirty(gwin, 1);		// Set to update new.
					// In a new chunk?
	if (olist != nlist)
		switched_chunks(olist, nlist);
	int roof_height = nlist->is_roof (tx, ty, new_lift);
	if (gwin->set_above_main_actor (roof_height))
		{
		gwin->set_in_dungeon(nlist->has_dungeon()?
					nlist->is_dungeon(tx, ty):0);
		gwin->set_all_dirty();
		}
	else if (roof_height < 31 && gwin->set_in_dungeon(nlist->has_dungeon()?
 					nlist->is_dungeon(tx, ty):0))
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
	Map_chunk *olist,	// Old chunk, or null.
	Map_chunk *nlist	// New chunk.
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
			gwin->get_chunk(x, y)->setup_cache();

	// If change in Superchunk number, apply Old Style caching emulation
	if (olist) gwin->emulate_cache(olist->get_cx(), olist->get_cy(), newcx, newcy);
	else gwin->emulate_cache(-1, -1, newcx, newcy);
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
	Map_chunk *olist = gwin->get_chunk_safely(
						get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Map_chunk *nlist = gwin->get_chunk(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
	int tx = get_tx(), ty = get_ty();
	if (gwin->set_above_main_actor(nlist->is_roof(tx, ty, newlift)))
		gwin->set_in_dungeon(nlist->has_dungeon()?
		nlist->is_dungeon(tx, ty):0);

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
	gwin->get_gump_man()->close_all_gumps();	// Obviously.
					// Special function for dying:
	if (Game::get_game_type() == BLACK_GATE)
		gwin->get_usecode()->call_usecode(
				0x60e, this, Usecode_machine::weapon);

		else
			gwin->get_usecode()->call_usecode(0x400, this,
					Usecode_machine::died);
	}

/*
 *	Set the shapenum based on skin color, sex, naked flag and petra and polymorph flag
 */
void Actor::set_actor_shape()
{
	if (get_npc_num() != 0 || get_siflag (polymorph))
		return;

	int sn;

	if (Game::get_game_type() == SERPENT_ISLE)
	{
		if (get_skin_color() == 0) // WH
		{
			sn = 1028+get_type_flag(tf_sex)+6*get_siflag(naked);
		}
		else if (get_skin_color() == 1) // BN
		{
			sn = 1026+get_type_flag(tf_sex)+6*get_siflag(naked);
		}
		else if (get_skin_color() == 2) // BK
		{
			sn = 1024+get_type_flag(tf_sex)+6*get_siflag(naked);
		}
	}
	else if (get_type_flag(Actor::tf_sex))
		sn = 989;
	else
		sn = 721;
#ifdef DEBUG
	cerr << "Setting Shape to " << sn << endl;
#endif
	set_shape (sn, get_framenum());
}

// Sets the polymorph to shape
void Actor::set_polymorph (int shape)
{
	// Polymorph is only SI
	if (Game::get_game_type() != SERPENT_ISLE) return;

#ifdef DEBUG
	cerr << "Setting polymorph for " << get_npc_num() << endl;
	cerr << "Shape " << shape << endl;
	cerr << "Save shape " << shape_save << endl;
#endif
	
	// Want to set to Avatar
	if (shape == 721)
	{
		Actor *avatar = Game_window::get_game_window()->get_main_actor();
		if (!avatar) return;

		if (avatar->get_skin_color() == 0) // WH
			shape = 1028+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
		else if (avatar->get_skin_color() == 1) // BN
			shape = 1026+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
		else // BK
			shape = 1024+avatar->get_type_flag(tf_sex)+6*avatar->get_siflag(naked);
	}

	if (shape == shape_save)
	{
		set_shape (shape_save);
		shape_save = -1;
		clear_siflag(polymorph);
		return;
	}
	if (shape_save == -1) shape_save = get_shapenum();
	set_shape (shape, get_framenum());
	set_siflag (polymorph);
}

// Sets polymorph shape to defaults based on flags and npc num
void Actor::set_polymorph_default()
{
	// Polymorph is only SI
	if (Game::get_game_type() != SERPENT_ISLE || !get_siflag(polymorph)
				|| (get_npc_num() != 0 && get_npc_num() != 28))
		return;

	set_actor_shape();

	shape_save = get_shapenum();

	if (get_npc_num() == 28)		// Handle Petra First
		set_polymorph (721);
	else if (get_flag(Obj_flags::petra))	// Avatar as petra
		set_polymorph (658);
	else	// Snake
		set_polymorph (530);
}

/*
 *	Get 'real' shape for Usecode.
 */

int Actor::get_shape_real
	(
	)
	{
	if (Game::get_game_type() == BLACK_GATE)
		return get_shapenum();
	if (npc_num != 0)		// Not the avatar?
		return shape_save!=-1?shape_save:get_shapenum();
					// Taking guess (6/18/01):
	if (get_type_flag(Actor::tf_sex))
		return 989;
	else
		return 721;
	}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	const std::string &nm, 			// Name.  A copy is made.
	int shapenum, 
	int num, 
	int uc
	) : Actor(nm, shapenum, num, uc), next(0), nearby(false),
		num_schedules(0), force_update(false),
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
 *	Set schedule type.
 */

void Npc_actor::set_schedule_time_type (int time, int type)
{
	Tile_coord tile;
	int i;

	for (i = 0; i < num_schedules; i++) if (schedules[i].get_time() == time) break;
	
	if (i == num_schedules)	// Didn't find it
	{
		Schedule_change *scheds = new Schedule_change[num_schedules+1];

		for (i = 0; i < num_schedules; i++)
		{
			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, schedules[i].get_type(), schedules[i].get_time());
		}

		scheds[num_schedules].set(0, 0, (unsigned char) type, (unsigned char) time);
		set_schedules(scheds, num_schedules+1);
	}
	else	// Did find it
	{
		tile = schedules[i].get_pos();
		schedules[i].set(tile.tx, tile.ty, (unsigned char) type, (unsigned char) time);
	}
}

/*
 *	Set schedule location.
 */

void Npc_actor::set_schedule_time_location (int time, int x, int y)
{
	int i;

	for (i = 0; i < num_schedules; i++) if (schedules[i].get_time() == time) break;
	
	if (i == num_schedules)	// Didn't find it
	{
		Tile_coord tile;
		Schedule_change *scheds = new Schedule_change[num_schedules+1];

		for (i = 0; i < num_schedules; i++)
		{
			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, schedules[i].get_type(), schedules[i].get_time());
		}

		scheds[num_schedules].set(x, y, 0, (unsigned char) time);
		set_schedules(scheds, num_schedules+1);
	}
	else	// Did find it
	{
		schedules[i].set(x, y, schedules[i].get_type(), (unsigned char) time);
	}
}

/*
 *	Remove schedule
 */

void Npc_actor::remove_schedule (int time)
{
	int i;

	for (i = 0; i < num_schedules; i++) if (schedules[i].get_time() == time) break;
	
	if (i != num_schedules)	// Found it
	{
		Tile_coord tile;
		Schedule_change *scheds = new Schedule_change[num_schedules-1];

		for (i = 0; i < num_schedules; i++)
		{
			if (schedules[i].get_time() == time) break;

			tile = schedules[i].get_pos();
			scheds[i].set(tile.tx, tile.ty, schedules[i].get_type(), schedules[i].get_time());
		}

		for (; i < num_schedules; i++)
		{
			tile = schedules[i-1].get_pos();
			scheds[i].set(tile.tx, tile.ty, schedules[i-1].get_type(), schedules[i-1].get_time());
		}

		set_schedules(scheds, num_schedules-1);
	}
}

/*
 *	Set schedule list.
 */

void Npc_actor::get_schedules
	(
	Schedule_change *&list, 
	int &cnt
	)
	{
	list = schedules;
	cnt = num_schedules;
	}
/*
 *	Move and change frame.
 */

inline void Npc_actor::movef
	(
	Map_chunk *old_chunk,
	Map_chunk *new_chunk, 
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
	if (Npc_actor::get_party_id() >= 0 || is_dead())
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
	int backwards,			// Extra periods to look backwards.
	int delay			// Delay in msecs, or -1 for random.
	)
	{
	force_update = false;
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
		// This is bad, not always true
		// location might be different
		//if (schedule_type == schedules[i].get_type())
		//	return;		// Already in it.
		}
	set_schedule_and_loc (schedules[i].get_type(), schedules[i].get_pos(),
								delay);
	}

/*
 *	Update schedule at a 3-hour time change.
 */

bool Npc_actor::update_forced_schedule()
{
	if (force_update)
	{
		Game_window *gwin = Game_window::get_game_window();
		update_schedule(gwin, gwin->get_hour()/3, 7);
		force_update = false;
		return true;
	}
	return false;
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
		dormant = false;	// But clear out old entries first.??
		gwin->get_tqueue()->remove(this);
					// Force schedule->now_what() in .5secs
					// DO NOT call now_what here!!!
		uint32 curtime = SDL_GetTicks();
		gwin->get_tqueue()->add(curtime + 500, this, (long) gwin);
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
	if (is_dead())
		return;
	Game_window *gwin = Game_window::get_game_window();
					// Converse, etc.
	Actor::activate(umachine, event);
	int i;				// Past 6:00pm first day?
	if (gwin->get_total_hours() >= 18 || 
	    Game::get_game_type() == SERPENT_ISLE ||
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
#if 0	/* ++++Causes Iolo bug in Fire&Ice test. */
	if (update_forced_schedule())
		return;
#endif
	if (!action)			// Not doing anything?
		{
		if (schedule)
			schedule->now_what();
		else
			dormant = true;
		}
	else
		{			// Do what we should.
		Game_window *gwin = Game_window::get_game_window();
		int delay = party_id < 0 ? gwin->is_time_stopped() : 0;
		if (delay <= 0)		// Time not stopped?
			delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			gwin->get_tqueue()->add(
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
	Map_chunk *nlist = gwin->get_chunk_safely(cx, cy);
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
	if (poison && new_lift == 0)
		Actor::set_flag((int) Obj_flags::poisoned);
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty(gwin);		// Set to repaint old area.
					// Get old chunk.
	Map_chunk *olist = gwin->get_chunk(old_cx, old_cy);
					// Move it.
	movef(olist, nlist, tx, ty, frame, new_lift);
					// Offscreen, but not in party?
	if (!add_dirty(gwin, 1) && Npc_actor::get_party_id() < 0 &&
					// And > a screenful away?
	    distance(gwin->get_camera_actor()) > 1 + 320/c_tilesize &&
			//++++++++Try getting rid of the 'talk' line:
	    get_schedule_type() != Schedule::talk &&
	    get_schedule_type() != Schedule::street_maintenance)
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
// Messes up resurrection	num_schedules = 0;
	gwin->get_tqueue()->remove(this);// Remove from time queue.
	gwin->remove_nearby_npc(this);	// Remove from nearby list.
					// Store old chunk list.
	Map_chunk *olist = gwin->get_chunk_safely(get_cx(), get_cy());
	Actor::remove_this(nodel);	// Remove.
	switched_chunks(olist, 0);
	cx = cy = 0xff;			// Set to invalid chunk coords.
	}

/*
 *	Update chunks' npc lists after this has moved.
 */

void Npc_actor::switched_chunks
	(
	Map_chunk *olist,	// Old chunk, or null.
	Map_chunk *nlist	// New chunk, or null.
	)
	{
	if (olist && olist->npcs)	// Remove from old list.
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
	Map_chunk *olist = gwin->get_chunk_safely(get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Map_chunk *nlist = gwin->get_chunk_safely(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
	}

/*
 *	Delete.
 */

Dead_body::~Dead_body
	(
	)
	{
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
	Actor *party[8];
	int cnt = gwin->get_party(party);// Get companions.
	int found = 0;
	for (int i = 0; i < cnt; i++)
		{
		int npc_num = party[i]->get_npc_num();
		if (npc_num > 0)
			{
			list[found] = (Dead_body *) gwin->get_body(npc_num);
			if (list[found])
				found++;
			}
		}
	return found;
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
	Tile_coord& t			// Tz possibly updated.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(this);
					// Get dim. in tiles.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int ztiles = info.get_3d_height();
	Tile_coord cur = get_abs_tile_coord();
	return Map_chunk::is_blocked(xtiles, ytiles, ztiles,
			cur, t, get_type_flags());
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
 *	Initialize a new monster.
 */

void Monster_actor::init
	(
	)
	{
	if (in_world)
		in_world->prev_monster = this;
	next_monster = in_world;
	in_world = this;
	in_world_cnt++;
					// Check for animated shape.
	Shape_info& info = Game_window::get_game_window()->get_info(this);
	if (info.is_animated())
		animator = Animator::create(this, 1);
	}

/*
 *	Create monster.
 */

Monster_actor::Monster_actor
	(
	const std::string &nm, 
	int shapenum, 
	int num,			// Generally -1.
	int uc
	) : Npc_actor(nm, shapenum, num, uc), prev_monster(0), info(0),
	    animator(0)
	{
	init();
	}

/*
 *	Delete.
 */

Monster_actor::~Monster_actor
	(
	)
	{
	delete animator;
					// Remove from chain.
	if (next_monster)
		next_monster->prev_monster = prev_monster;
	if (prev_monster)
		prev_monster->next_monster = next_monster;
	else				// We're at start of list.
		in_world = next_monster;
	if (!is_dead())			// Dying decrements count.
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
 *	Render.
 */

void Monster_actor::paint
	(
	Game_window *gwin
	)
	{
	Npc_actor::paint(gwin);		// Draw on screen.
	if (animator)			// Be sure animation is on.
		animator->want_animation();
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
	Game_window *gwin = Game_window::get_game_window();
	
	// If move not allowed do I remove or change destination?
	// I'll do nothing for now
	if (!gwin->emulate_is_move_allowed(t.tx, t.ty))
		return (0);

					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
					// Get chunk.
	int cx = t.tx/c_tiles_per_chunk, cy = t.ty/c_tiles_per_chunk;
					// Get ->new chunk.
	Map_chunk *nlist = gwin->get_chunk(cx, cy);
	nlist->setup_cache();		// Setup cache if necessary.
					// Blocked?
	if (is_blocked(t))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		if (!gwin->add_dirty(this))
			dormant = true;	// Off-screen.
		return (0);		// Done.
		}
					// Check for scrolling.
	gwin->scroll_if_needed(this, t);
	add_dirty(gwin);		// Set to repaint old area.
					// Get old chunk.
	Map_chunk *olist = gwin->get_chunk(old_cx, old_cy);
					// Move it.
					// Get rel. tile coords.
	int tx = t.tx%c_tiles_per_chunk, ty = t.ty%c_tiles_per_chunk;
	movef(olist, nlist, tx, ty, frame, t.tz);
	if (!add_dirty(gwin, 1) &&
					// And > a screenful away?
	    distance(gwin->get_camera_actor()) > 1 + 320/c_tilesize)
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
	int lift,			// Lift.
	int sched,			// Schedule type.
	int align,			// Alignment.
	bool temporary
	)
	{
					// Usecode = shape.
	Monster_actor *monster = new Monster_actor("", shapenum, -1, shapenum);
	monster->set_info(this);
	if (align != -1)
		monster->set_alignment(align);

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

	// Set temporary
	if (temporary) monster->set_flag (Obj_flags::is_temporary);
					// ++++Armor?
					// Place in world.
	Game_window *gwin = Game_window::get_game_window();
	Map_chunk *olist = gwin->get_chunk(chunkx, chunky);
	monster->movef(0, olist, tilex, tiley, 0, lift);
					// Get equipment.
	if (equip_offset && equip_offset - 1 < equip_cnt)
		{
		Equip_record& rec = equip[equip_offset - 1];
		for (size_t i = 0;
			i < sizeof(equip->elements)/sizeof(equip->elements[0]);
							i++)
			{		// Give equipment.
			Equip_element& elem = rec.elements[i];
			if (!elem.shapenum || 1 + rand()%100 > 
							elem.probability)
				continue;// You lose.
			int frnum = (elem.shapenum == 377) ? 
					Find_monster_food(shapenum) : 0;
			monster->create_quantity(elem.quantity, elem.shapenum, 
						c_any_qual, frnum, temporary);
			}
		}
	if (sched < 0)			// Set sched. AFTER equipping.
		sched = (int) Schedule::loiter;
	monster->set_schedule_type(sched);
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


