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

#include <iostream>			/* Debugging. */
#include <stdlib.h>
#include <string.h>
#include "gamewin.h"
#include "imagewin.h"
#include "usecode.h"
#include "actions.h"
#include "ready.h"
#include "combat.h"
#include "Zombie.h"
#include "Astar.h"
#include "dir.h"
#include "items.h"
#include "egg.h"

Frames_sequence *Actor::frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const char Actor::attack_frames1[4] = {3, 4, 5, 6};
const char Actor::attack_frames2[4] = {3, 7, 8, 9};
Equip_record *Monster_info::equip = 0;
int Monster_info::equip_cnt = 0;
Monster_actor *Monster_actor::in_world = 0;
int Monster_actor::in_world_cnt = 0;

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
 *	Create character.
 */

Actor::Actor
	(
	char *nm, 
	int shapenum, 
	int num,			// NPC # from npc.dat.
	int uc				// Usecode #.
	) : Container_game_object(), name(nm==0?0:strdup(nm)),usecode(uc), 
	    npc_num(num), party_id(-1), attack_mode(nearest),
	    schedule_type((int) Schedule::loiter), schedule(0), dormant(1),
	    two_handed(0), two_fingered(false), light_sources(0),
	    usecode_dir(0), flags(0), action(0), frame_time(0),
	    next_path_time(0)
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
	delete name;
	delete action;
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
	set_action(action->walk_to_tile(get_abs_tile_coord(), dest));
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
	set_action(action->walk_to_tile(src, dest));
	if (action)			// Successful at setting path?
		{
		start(speed, delay);
		return (1);
		}
	frame_time = 0;			// Not moving.
	return (0);
	}

/*
 *	Walk to destination point.
 */

void Actor::walk_to_point
	(
					// Point in world:
	unsigned long destx, unsigned long desty,
	int speed			// Delay between frames.
	)
	{
	int liftpixels = 4*get_lift();
	int tx = (destx + liftpixels)/tilesize, 
	    ty = (desty + liftpixels)/tilesize;
	walk_to_tile(tx, ty, get_lift(), speed, 0);
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
		unsigned long curtime = SDL_GetTicks();
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
		Game_window::get_game_window()->add_dirty(this);
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
	if (schedule_type == Schedule::combat)
		return;			// Not when fighting.
	int delay = 0;
					// How close to aim for.
	int dist = 3 + Actor::get_party_id()/3;
	Tile_coord leaderpos = leader->get_abs_tile_coord();
					// Aim for leader's dest.
	Tile_coord goal = leader->is_moving() ? leader->get_dest() : leaderpos;
	Tile_coord pos = get_abs_tile_coord();
					// Tiles to goal.
	int goaldist = goal.distance(pos);
	if (goaldist < dist)		// Already close enough?
		return;
					// Get leader's distance from goal.
	int leaderdist = goal.distance(leaderpos);
					// Figure where to aim.
	goal.tx = Approach(pos.tx, goal.tx, dist);
	goal.ty = Approach(pos.ty, goal.ty, dist);
	if (!leader->is_moving())	// Leader stopped?
		{
		goal.tx += 1 - rand()%3;// Jiggle a bit.
		goal.ty += 1 - rand()%3;
		}
					// Get his speed.
	int speed = leader->get_frame_time();
	if (!speed)			// Not moving?
		speed = 125;
	if (goaldist <= leaderdist)	// Closer than leader?
		{			// Delay a bit.
		delay = (1 + leaderdist - goaldist)*100;
//		speed += 10;		// And slow a bit.
		}
	else if (goaldist - leaderdist > 5)
		speed -= 20;		// Speed up if too far.
	if (goaldist > 32 &&		// Getting kind of far away?
	    get_party_id() >= 0)	// And a member of the party.
		{			// Teleport.
		int pixels = goaldist*tilesize;
		Game_window *gwin = Game_window::get_game_window();
		if (pixels > gwin->get_width() + 16)
			{
			move(goal.tx, goal.ty, goal.tz);
			say("Thou shan't lose me so easily!");
			gwin->paint();
			return;
			}
		}
	unsigned long curtime = SDL_GetTicks();
	if (pos.distance(leaderpos) >= 8 && get_party_id() >= 0 &&
	    curtime >= next_path_time)
		{			// A little stuck?
		cout << get_name() << " trying to catch up." << endl;
					// Don't try again for a few seconds.
		next_path_time = SDL_GetTicks() + 4000;
		if (walk_path_to_tile(goal, speed - speed/20, 0))
			return;
		}
	walk_to_tile(goal, speed, delay);
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
	if (info.get_shape_class() == Shape_info::container)
		return !spots[back] ? back : free_hand();
	Ready_type type = (Ready_type) info.get_ready_type();
	switch (type)
		{
	case spell:
	case other_spell:
	case one_handed_weapon:
	case tongs:
		return free_hand();
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
	case gloves:
		// Gloves occupy both finger spots
		return (!spots[lfinger] && !spots[rfinger]) ? lrfinger : free_hand();
					// ++++++What about belt?
	case other:
	default:
		return free_hand();
		}
	}

/*
 *	Set new schedule by type.
 */

void Actor::set_schedule_type
	(
	int new_schedule_type
	)
	{
	stop();				// Stop moving.
	if (schedule)
		schedule->ending();	// Finish up old if necessary.
	schedule_type = new_schedule_type;
	delete schedule;		// Done with the old.
	schedule = 0;
	switch ((Schedule::Schedule_types) schedule_type)
		{
	case Schedule::combat:
		schedule = new Combat_schedule(this);
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
	case Schedule::loiter:
	case Schedule::hound:		// For now.
	case Schedule::graze:
		schedule = new Loiter_schedule(this);
		break;
	case Schedule::sleep:
		schedule = new Sleep_schedule(this);
		break;
	case Schedule::eat:		// For now.
	case Schedule::eat_at_inn:	// For now.
	case Schedule::sit:
		schedule = new Sit_schedule(this);
		break;
	case Schedule::patrol:
		schedule = new Patrol_schedule(this);
		break;
	case Schedule::wait:		// Loiter just a little
//+++++Figure out why this messes up Mayor's talk at intro.
//		schedule = new Loiter_schedule(this, 1);
		break;
	default:
		break;
		}
	if (schedule)			// Try to start it.
		{
		dormant = 0;
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
	if (!(flags & (1L << dont_render)))
		Container_game_object::paint(gwin);
	paint_weapon(gwin);
	}

// Utility function for paint_weapon()
static inline void swap(unsigned char & a, unsigned char & b)
	{
	unsigned char temp = a;
	a = b;
	b = temp;
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
	unsigned char actor_x, actor_y;
	unsigned char weapon_x, weapon_y;
	Game_object * weapon = spots[lhand];

	if(weapon == 0)
		return;
	// Get offsets for actor shape
	gwin->get_info(this).get_weapon_offset(get_framenum() & 0x1f, actor_x,
			actor_y);
	// Get offsets for weapon shape
	// NOTE: when combat is implemented, weapon frame should depend on
	// actor's current attacking frame
	int weapon_frame = 1;
	gwin->get_info(weapon).get_weapon_offset(weapon_frame, weapon_x,
			weapon_y);
	// actor_x will be 255 if (for example) the actor is lying down
	// weapon_x will be 255 if the actor is not holding a proper weapon
	if(actor_x != 255 && weapon_x != 255)
		{
		// Need to swap offsets if actor's shape is reflected
		if(get_framenum() & 32)
		{
			swap(actor_x, actor_y);
			swap(weapon_x, weapon_y);
			weapon_frame |= 32;
		}
		// Paint the weapon shape using the actor's coordinates
		int tx, ty, tz;
		get_abs_tile(tx, ty, tz);
		int liftpix = 4*tz;
		gwin->paint_shape(
			(tx + 1 - gwin->get_scrolltx())*tilesize
				- 1 - liftpix - actor_x + weapon_x,
			(ty + 1 - gwin->get_scrollty())*tilesize
				- 1 - liftpix - actor_y + weapon_y,
			weapon->get_shapenum(), weapon_frame);
		}
	}

/*
 *	Run usecode when double-clicked.
 */

void Actor::activate
	(
	Usecode_machine *umachine
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// In gump mode?  Or Avatar?
	if (!npc_num)			// Avatar?
		gwin->show_gump(this, ACTOR_FIRST_GUMP);// ++++58 if female.
	else if (gwin->get_mode() == Game_window::gump &&
		 get_party_id() >= 0 &&
		 npc_num >= 1 && npc_num <= 10)
					// Show companions' pictures.
			gwin->show_gump(this, ACTOR_FIRST_GUMP + 1 + npc_num);
	else if (get_schedule_type() == (int) Schedule::sleep)
		return;			// Asleep.  +++++Check flag too?
	else if (usecode == -1)
		umachine->call_usecode(get_shapenum(), this,
				Usecode_machine::double_click);
	else
		umachine->call_usecode(usecode, this, 
					Usecode_machine::double_click);
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
	return name ? string(name) : Game_object::get_name();
	}

/*
 *	Set flag.
 */

void Actor::set_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags |= ((unsigned long) 1 << flag);
	}

/*
 *	Clear flag.
 */

void Actor::clear_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags &= ~((unsigned long) 1 << flag);
	}

/*
 *	Get flag.
 */

int Actor::get_flag
	(
	int flag
	) const
	{
	return (flag >= 0 && flag < 32) ? (flags & ((unsigned long) 1 << flag))
			!= 0 : 0;
	}

/*
 *	Remove an object.
 */

void Actor::remove
	(
	Game_object *obj
	)
	{
	Container_game_object::remove(obj);
	int index = Actor::find_readied(obj);	// Remove from spot.
	if (index >= 0)
		{			// Update light-source count.
		if (Game_window::get_game_window()->get_info(obj).
							is_light_source())
			light_sources--;
		spots[index] = 0;
		if (index == rhand || index == lhand)
			two_handed = 0;
		if (index == rfinger || index == lfinger)
			two_fingered = 0;
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
	int dont_check			// 1 to skip volume check.
	)
	{
	int index = find_best_spot(obj);// Where should it go?
	if (index < 0)			// No free spot?  Look for a bag.
		{
		if (spots[back] && spots[back]->drop(obj))
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
		two_handed = 1;
		index = lhand;
		}
	if (index == lrfinger)		// Gloves?
		{
		two_fingered = 1;
		index = lfinger;
		}
	spots[index] = obj;		// Store in correct spot.
	obj->cx = obj->cy = 0;		// Clear coords. (set by gump).
	if (Game_window::get_game_window()->get_info(obj).is_light_source())
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
	if (index == best_index || (!two_handed && index == lhand)
			|| (!two_handed && index == rhand
				&& best_index != lrhand))
		{			// Okay.
		if (!Container_game_object::add(obj))
			return (0);	// No room, or too heavy.
		spots[index] = obj;
		obj->cx = obj->cy = 0;	// Clear coords. (set by gump).
		if (best_index == lrhand)
			two_handed = 1;	// Must be a two-handed weapon.
		if (best_index == lrfinger)
			two_fingered = 1;	// Must be gloves
		if (Game_window::get_game_window()->get_info(obj).
							is_light_source())
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
 *	Get total value of armor being worn.
 */

int Actor::get_armor_points
	(
	)
	{
	int points = 0;
	static enum Spots aspots[] = {neck, torso, lfinger, rfinger, head,
					legs, feet};
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

int Actor::get_weapon_points
	(
	)
	{
	int points = 0;
	Game_window *gwin = Game_window::get_game_window();
	Game_object *weapon = spots[(int) lhand];
	if (weapon)
		points = gwin->get_info(weapon).get_weapon();
					// Try both hands.
	weapon = spots[(int) rhand];
	if (weapon)
		{
		int rpoints = gwin->get_info(weapon).get_weapon();
		if (rpoints > points)
			points = rpoints;
		}
	return points;
	}

/*
 *	Figure hit points lost from an attack, and subtract from total.
 *
 *	Output:	# of hit points lost (already subtracted).
 */

int Actor::figure_hit_points
	(
	Actor *attacker
	)
	{
	int armor = get_armor_points();
	int weapon = attacker->get_weapon_points();
	if (!weapon)
		weapon = 1;
	int attacker_level = attacker->get_level();
	int prob = 30 + attacker_level + 
			attacker->get_property((int) combat) +
			attacker->get_property((int) dexterity) -
			get_property((int) dexterity) +
			weapon - armor;
	cout << "Hit probability is " << prob << endl;
	if (rand()%100 > prob)
		return 0;		// Missed.
					// Compute hit points to lose.
	int hp = attacker->get_property((int) strength)/4 +
			(rand()%attacker_level) +
			weapon - armor;
	if (hp < 1)
		hp = 1;
	int oldhealth = properties[(int) health];
	int maxhealth = properties[(int) strength];
	properties[(int) health] -= hp;	// Subtract from health.
	if (oldhealth >= maxhealth/2 && properties[(int) health] <
					maxhealth/2 && rand()%3 != 0)
					// A little oomph.
		say(first_ouch, last_ouch);
	cout << "Attack damage was " << hp << " hit points, leaving " << 
		properties[(int) health] << " remaining" << endl;
	return hp;
	}

/*
 *	Being attacked.
 */

void Actor::attacked
	(
	Actor *attacker
	)
	{
	figure_hit_points(attacker);
	if (is_dead_npc())
		die();			// +++++++Wimp.
	}

/*
 *	We're dead.  We're removed from the world, but not deleted.
 */

void Actor::die
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	properties[(int) health] = -50;
	gwin->add_dirty(this);		// Want to repaint area.
					// Get location.
	Tile_coord pos = get_abs_tile_coord();
	set_action(0);
	remove_this(1);			// Remove (but don't delete this).
	int shnum = 400;		// +++++Figure out correct shape.
	int frnum = 3;			// +++++
					// Put body here.
	Container_game_object *body = new Container_game_object(
							shnum, frnum, 0, 0);
	body->move(pos);
	Game_object *item;		// Move all the items.
	while ((item = get_first_object()) != 0)
		{
		remove(item);
		body->add(item, 1);	// Always succeed at adding.
		}
	gwin->add_dirty(body);
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
#if 0
					// Not doing an animation?
	if (!gwin->get_usecode()->in_usecode())
		get_followers();	// Get party to follow.
#endif
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
		if (npc)
			npc->follow(this);
		}
	}

/*
 *	Step onto an adjacent tile.
 *
 *	Output:	Delay for next frame, or 0 to stop.
 */

int Main_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get chunk.
	int cx = t.tx/tiles_per_chunk, cy = t.ty/tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%tiles_per_chunk, ty = t.ty%tiles_per_chunk;
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	int old_lift = get_lift();
	int new_lift;			// Might climb/descend.
					// Just assume height==3.
	if (nlist->is_blocked(3, old_lift, tx, ty, new_lift))
		{
		stop();
		return (0);
		}
					// Check for scrolling.
	gwin->scroll_if_needed(t);
	gwin->add_dirty(this);		/// Set to update old location.
					// Get old chunk, old tile.
	Chunk_object_list *olist = gwin->get_objects(get_cx(), get_cy());
	Tile_coord oldtile = get_abs_tile_coord();
					// Move it.
	Game_object::move(olist, cx, cy, nlist, tx, ty, frame, new_lift);
	gwin->add_dirty(this);		// Set to update new.
					// Near an egg?
	nlist->activate_eggs(this, t.tx, t.ty, oldtile.tx, oldtile.ty);
					// In a new chunk?
	if (olist != nlist)
		{
		switched_chunks(olist, nlist);
		if (gwin->set_above_main_actor(nlist->is_roof(), new_lift))
					// Repaint all.
			gwin->set_all_dirty();
		}
	else if (old_lift != new_lift &&
		 gwin->set_above_main_actor(new_lift))
			gwin->set_all_dirty();
	return (frame_time);		// Add back to queue for next time.
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
		xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
		yfrom = newcy > 0 ? newcy - 1 : newcy;
		yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
		}
	else
		{
		int oldcx = olist->get_cx(), oldcy = olist->get_cy();
		if (newcx == oldcx + 1)
			{
			xfrom = newcx;
			xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
			}
		else if (newcx == oldcx - 1)
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx;
			}
		else
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
			}
		if (newcy == oldcy + 1)
			{
			yfrom = newcy;
			yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
			}
		else if (newcy == oldcy - 1)
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy;
			}
		else
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
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
	Chunk_object_list *olist = gwin->get_objects(get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Chunk_object_list *nlist = gwin->get_objects(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
	gwin->set_above_main_actor(nlist->is_roof(), newlift);
	}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	char *nm, 			// Name.  A copy is made.
	int shapenum, 
	int fshape, 
	int uc
	) : Actor(nm, shapenum, fshape, uc), next(0), nearby(0),
		num_schedules(0), 
		schedules(0), alignment(0)
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
 *	Update schedule at a 3-hour time change.
 */

void Npc_actor::update_schedule
	(
	Game_window *gwin,
	int hour3			// 0=midnight, 1=3am, etc.
	)
	{
	if (Npc_actor::get_party_id() >= 0)
		return;			// Skip if a party member.
	for (int i = 0; i < num_schedules; i++)
		if (schedules[i].get_time() == hour3)
			{		// Found entry.
			stop();		// Stop moving.
					// Going to walk there.
			schedule_type = Schedule::walk_to_schedule;
			delete schedule;
			schedule = new Walk_to_schedule(this, 
						schedules[i].get_pos(),
						schedules[i].get_type());
			dormant = 0;
			schedule->now_what();
			return;
			}
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
		dormant = 0;		// But clear out old entries first.??
		gwin->get_tqueue()->remove(this);
		schedule->now_what();	// Ask scheduler what to do.
		}
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
		dormant = 1;
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
 *	Output:	Delay for next frame, or 0 to stop.
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
	int cx = t.tx/tiles_per_chunk, cy = t.ty/tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%tiles_per_chunk, ty = t.ty%tiles_per_chunk;
					// Get ->new chunk.
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	nlist->setup_cache();		// Setup cache if necessary.
	int new_lift;			// Might climb/descend.
					// Just assume height==3.
	if (nlist->is_blocked(3, get_lift(), tx, ty, new_lift))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		return (0);		// Done.
		}
	gwin->add_dirty(this);		// Set to repaint old area.
					// Get old chunk.
	Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
	move(olist, cx, cy, nlist, tx, ty, frame, new_lift);
	if (!gwin->add_dirty(this))
		{			// No longer on screen.
		stop();
		dormant = 1;
		return (0);
		}
	return (frame_time);		// Add back to queue for next time.
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
	Chunk_object_list *olist = gwin->get_objects(get_cx(), get_cy());
					// Move it.
	Game_object::move(newtx, newty, newlift);
	Chunk_object_list *nlist = gwin->get_objects(get_cx(), get_cy());
	if (nlist != olist)
		switched_chunks(olist, nlist);
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
	int tx, ty, lift;		// Get current position.
	get_abs_tile(tx, ty, lift);
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(this);
					// Get dim. in tiles.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int height = info.get_3d_height();
	int vertx0, vertx1;		// Get x-coords. of vert. block
					//   to right/left.
	int horizx0, horizx1;		// Get x-coords of horiz. block
					//   above/below.
	int verty0, verty1;		// Get y-coords of horiz. block
					//   above/below.
	int horizy0, horizy1;		// Get y-coords of vert. block
					//   to right/left.
	horizx0 = destx + 1 - xtiles;
	horizx1 = destx;
	if (destx >= tx)		// Moving right?
		{
		vertx0 = tx + 1;	// Start to right of hot spot.
		vertx1 = destx;		// End at dest.
		}
	else				// Moving left?
		{
		vertx0 = destx + 1 - xtiles;
		vertx1 = tx - xtiles;
		}
	verty0 = desty + 1 - ytiles;
	verty1 = desty;
	if (desty >= ty)		// Moving down?
		{
		horizy0 = ty + 1;	// Start below hot spot.
		horizy1 = desty;	// End at dest.
		verty1--;		// Includes bottom of vert. area.
		}
	else				// Moving up?
		{
		horizy0 = desty + 1 - ytiles;
		horizy1 = ty - ytiles;
		verty0++;		// Includes top of vert. area.
		}
	int x, y;			// Go through horiz. part.
	for (y = horizy0; y <= horizy1; y++)
		{			// Get y chunk, tile-in-chunk.
		int cy = y/tiles_per_chunk, rty = y%tiles_per_chunk;
		for (x = horizx0; x <= horizx1; x++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					x/tiles_per_chunk, cy);
			olist->setup_cache();
			if (olist->is_blocked(height, lift, x%tiles_per_chunk,
							rty, new_lift) ||
			    new_lift != lift)
				return (1);
			}
		}
					// Do vert. block.
	for (x = vertx0; x <= vertx1; x++)
		{			// Get x chunk, tile-in-chunk.
		int cx = x/tiles_per_chunk, rtx = x%tiles_per_chunk;
		for (y = verty0; y <= verty1; y++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					cx, y/tiles_per_chunk);
			olist->setup_cache();
			if (olist->is_blocked(height, lift, rtx,
					y%tiles_per_chunk, new_lift) ||
			    new_lift != lift)
				return (1);
			}
		}
	return (0);			// All clear.
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
					// Remove from chain.
	if (next_monster)
		next_monster->prev_monster = prev_monster;
	if (prev_monster)
		prev_monster->next_monster = next_monster;
	else				// We're at start of list.
		in_world = next_monster;
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
 *	Output:	Delay for next frame, or 0 to stop.
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
	int cx = t.tx/tiles_per_chunk, cy = t.ty/tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%tiles_per_chunk, ty = t.ty%tiles_per_chunk;
					// Get ->new chunk.
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	nlist->setup_cache();		// Setup cache if necessary.
					// Blocked.
	if (is_blocked(cx*tiles_per_chunk + tx, cy*tiles_per_chunk + ty))
		{
		if (schedule)		// Tell scheduler.
			schedule->set_blocked(t);
		stop();
		return (0);		// Done.
		}
	gwin->add_dirty(this);		// Set to repaint old area.
					// Get old chunk.
	Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
	move(olist, cx, cy, nlist, tx, ty, frame, -1);
	if (!gwin->add_dirty(this))
		{			// No longer on screen.
		stop();
		dormant = 1;
		return (0);
		}
	return (frame_time);		// Add back to queue for next time.
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

int Monster_actor::get_weapon_points
	(
	)
	{
	Monster_info *inf = get_info();
					// Kind of guessing here.
	int points = Actor::get_weapon_points();
	if (!points)			// No readied weapon?
		return inf ? inf->weapon/4 : 0;
	else
		return points;
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
		creator->monster_died();
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
	Monster_actor *monster = new Monster_actor(0, shapenum);
	monster->set_info(this);
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
	monster->move(0, chunkx, chunky, olist, tilex, tiley, 0, lift);
					// ++++++For now:
	monster->set_schedule_type(Schedule::loiter);
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
		int frnum = 0;		// Frame #???
		monster->add_quantity(elem.quantity, elem.shapenum, -359,
								frnum);
		}
	return (monster);
	}

