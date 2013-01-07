/*
 *	Schedule.cc - Schedules for characters.
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

#include "SDL_timer.h"
#include "actors.h"
#include "schedule.h"
#include "Zombie.h"
#include "gamewin.h"
#include "gameclk.h"
#include "gamemap.h"
#include "actions.h"
#include "dir.h"
#include "items.h"
#include "game.h"
#include "paths.h"
#include "ucmachine.h"
#include "ucsched.h"
#include "ucscriptop.h"
#include "monstinf.h"
#include "frameseq.h"
#include "ucsymtbl.h"
#include "useval.h"

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::cerr;
using std::endl;
using std::rand;
#endif

vector<char *> Schedule_change::script_names;

/*
 *	Create. 
 */
Schedule::Schedule
	(
	Actor *n
	) : npc(n), blocked(-1, -1, -1), street_maintenance_failures(0),
	    street_maintenance_time(0)
	{
	prev_type = npc ? npc->get_schedule_type() : -1;
	}


/*
 *	Set up an action to get an actor to a location (via pathfinding), and
 *	then execute another action when he gets there.
 */

void Schedule::set_action_sequence
	(
	Actor *actor,			// Whom to activate.
	Tile_coord const& dest,		// Where to walk to.
	Actor_action *when_there,	// What to do when he gets there.
	bool from_off_screen,		// Have actor walk from off-screen.
	int delay			// Msecs. to delay.
	)
	{
	actor->set_action(Actor_action::create_action_sequence(
			actor, dest, when_there, from_off_screen));
	actor->start(250, delay);	// Get into time queue.
	}

/*
 *	Look for nearby enemies.
 *
 *	Output:	true if any are found, which means npc's schedule has changed!
 */

bool Schedule::seek_foes
	(
	)
	{
	Actor_vector vec;		// Look within 10 tiles (guess).
	npc->find_nearby_actors(vec, c_any_shapenum, 10, 0x28);
	int npc_align = npc->get_effective_alignment();
	Actor *foe = 0;
	Monster_info *minf = npc->get_info().get_monster_info();
	bool see_invisible = minf ?
		(minf->get_flags() & (1<<Monster_info::see_invisible))!=0 : false;
	for (Actor_vector::const_iterator it = vec.begin(); 
						it != vec.end(); ++it)
		{
		Actor *actor = *it;
		if (actor->is_dead() || actor->get_flag(Obj_flags::asleep) ||
		    (!see_invisible && actor->get_flag(Obj_flags::invisible)))
			continue;	// Dead, asleep or invisible and can't see invisible.
		if ((npc_align == Npc_actor::friendly &&
				actor->get_effective_alignment() >= Npc_actor::hostile) ||
			(npc_align == Npc_actor::hostile &&
				actor->get_effective_alignment() == Npc_actor::friendly))
			{
			foe = actor;
			break;
			}
		}
	if (foe)
		{
		Actor *safenpc = npc;
			// May delete us!
		safenpc->set_schedule_type(Schedule::combat, 0);
		safenpc->set_target(foe);
		return 1;
		}
	return 0;
	}

/*
 *	Look for lamps to light/unlight and shutters to open/close.
 *
 *	Output:	1 if successful, which means npc's schedule has changed!
 */

int Schedule::try_street_maintenance
	(
	)
	{
					// What to look for:
	static int night[] = {322, 372, 889};
	static int sinight[] = {290, 291, 889};
	static int day[] = {290, 291, 526};

	long curtime = Game::get_ticks();
	if (curtime < street_maintenance_time)
		return 0;		// Not time yet.
	if (npc->Actor::get_npc_num() <= 0 ||
	    npc == gwin->get_camera_actor())
		return 0;		// Only want normal NPC's.
					// At least 30secs. before next one.
	street_maintenance_time = curtime + 30000 + 
				street_maintenance_failures*5000;
	int *shapes;
	int hour = gclock->get_hour();
	bool bg = (Game::get_game_type() == BLACK_GATE);
	if (hour >= 9 && hour < 18)
		shapes = &day[0];
	else if (hour >= 18 || hour < 6)
		shapes = bg ? &night[0] : &sinight[0];
	else
		return 0;		// Dusk or dawn.
	Tile_coord npcpos = npc->get_tile();
					// Look at screen + 1/2.
	Rectangle winrect = gwin->get_win_tile_rect();
	winrect.enlarge(winrect.w/4);
	if (!winrect.has_world_point(npcpos.tx, npcpos.ty))
		return 0;
					// Get to within 1 tile.
	Actor_pathfinder_client cost(npc, 2);
	Game_object *found = 0;		// Find one we can get to.
	Actor_action *pact;		// Gets ->action to walk there.
	for (size_t i = 0; !found && i < sizeof(night)/sizeof(night[0]); i++)
		{
		Game_object_vector objs;// Find nearby.
		int cnt = npc->find_nearby(objs, shapes[i], 20, 0);
		int j;
		for (j = 0; j < cnt; j++)
			{
			Game_object *obj = objs[j];
			int shnum = obj->get_shapenum();
			if (!bg &&	// Serpent isle?  Shutters?
			    (shnum == 290 || shnum == 291))
					// Want closed during day.
				if ((shapes == day) !=
					(obj->get_framenum() <= 3))
					continue;
			if ((pact = Path_walking_actor_action::create_path(
			    npcpos, obj->get_tile(), cost)) != 0)
				{
				found = obj;
				break;
				}
			street_maintenance_failures++;
			}
		}
	if (!found)
		return 0;		// Failed.
					// Set actor to walk there.
	npc->set_schedule_type(Schedule::street_maintenance,
			new Street_maintenance_schedule(npc, pact, found));
	// Warning: we are deleted here
	return 1;
	}

void Schedule::im_dormant()
{
	npc->set_action(0);
}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Schedule::get_actual_type
	(
	Actor *npc
	) const
	{
	return npc->get_schedule_type();
	}

bool Schedule::try_proximity_usecode(int odds)
{
	if ((rand()%odds) == 0)
	{
		gwin->get_usecode()->call_usecode(npc->get_usecode(), npc,
					Usecode_machine::npc_proximity);
		npc->start(gwin->get_std_delay(), 500 + rand()%1000);
		return true;
	}
	return false;
}

/*
 *	Run usecode function.
 */

void Scripted_schedule::run
	(
	int id				// A Usecode function #.
	)
	{
	if (id)
		ucmachine->call_method(inst, id, npc);
	}

/*
 *	Lookup Usecode 'method'.
 */

static int find_method(Usecode_class_symbol *cls, const char *meth, bool noerr)
	{
	Usecode_symbol *ucsym = (*cls)[meth];
	if (!ucsym)
		{
		if (!noerr)
			cerr << "Failed to find method '" << meth
					<< "' in class '" << cls->get_name() << "'." << endl;
		return 0;
		}
	return ucsym->get_val();
	}

/*
 *	Create schedule that uses Usecode methods for the actions.
 */

Scripted_schedule::Scripted_schedule
	(
	Actor *n,
	int ty
	) : Schedule(n), type(ty)
	{
	char *nm = Schedule_change::get_script_name(ty);
	Usecode_class_symbol *cls = ucmachine->get_class(nm);
	if (!cls)
		{
		cerr << "Could not find scripted schedule '" << nm <<
				"'. Switching to 'Loiter' schedule instead." << endl;
		npc->set_schedule_type(loiter);
		return;
		}
	else
		{
		inst = new Usecode_value(0);
		inst->class_new(cls, cls->get_num_vars());
		now_what_id = find_method(cls, "now_what", false);
		im_dormant_id = find_method(cls, "im_dormant", true);
		ending_id = find_method(cls, "ending", true);
		set_weapon_id = find_method(cls, "set_weapon", true);
		set_bed_id = find_method(cls, "set_bed", true);
		notify_object_gone_id = find_method(cls, "notify_object_gone", true);
		}
	}

/*
 *	Cleanup.
 */

Scripted_schedule::~Scripted_schedule
	(
	)
	{
	ucmachine->call_method(inst, -1, NULL);	// Free 'inst'.
	}

/*
 *	Create schedule to turn lamps on/off, open/close shutters.
 */

Street_maintenance_schedule::Street_maintenance_schedule
	(
	Actor *n, 
	Actor_action *p, 
	Game_object *o
	) : Schedule(n), obj(o), shapenum(o->get_shapenum()),
				framenum(o->get_framenum()), paction(p)
	{
	}

/*
 *	Street-maintenance schedule.
 */

void Street_maintenance_schedule::now_what
	(
	)
	{
	if (paction)			// First time?
		{			// Set to follow given path.
		cout << npc->get_name() << 
			" walking for street maintenance" << endl;
		npc->set_action(paction);
		npc->start(250);
		paction = 0;
		return;
		}
	if (obj && npc->distance(obj) <= 2 &&	// We're there.
	    obj->get_shapenum() == shapenum && obj->get_framenum() == framenum)
		{
		cout << npc->get_name() << 
			" about to perform street maintenance" << endl;
		int dir = npc->get_direction(obj);
		signed char frames[2];
		frames[0] = npc->get_dir_framenum(dir, Actor::standing);
		frames[1] = npc->get_dir_framenum(dir, 3);
		signed char standframe = frames[0];
		npc->set_action(new Sequence_actor_action(
			new Frames_actor_action(frames, sizeof(frames)),
			new Activate_actor_action(obj),
			new Frames_actor_action(&standframe, 1)));
		add_client(obj);
		npc->start(250);
		switch (shapenum)
			{
		case 322:		// Closing shutters.
		case 372:
			npc->say(first_close_shutters, last_close_shutters);
			break;
		case 290:		// Open shutters (or both for SI).
		case 291:
			if (Game::get_game_type() == BLACK_GATE)
				npc->say(first_open_shutters, 
							last_open_shutters);
			else		// SI.
				if (framenum <= 3)
					npc->say(first_open_shutters, 
							last_open_shutters);
				else
					npc->say(first_close_shutters, 
							last_close_shutters);
			break;
		case 889:		// Turn on lamp.
			npc->say(first_lamp_on, last_lamp_on);
			break;
		case 526:		// Turn off lamp.
			npc->say(lamp_off, lamp_off);
			break;
			}
		shapenum = 0;		// Don't want to repeat.
		return;
		}
	cout << npc->get_name() << 
			" done with street maintenance" << endl;
				// Set back to old schedule.
	int period = gclock->get_hour()/3;
	npc->update_schedule(period, 7, 0);
	}

void Street_maintenance_schedule::ending(int newtype)
{
	remove_clients();
}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Street_maintenance_schedule::get_actual_type
	(
	Actor * /* npc */
	) const
	{
	return prev_type;
	}

/*
 *	Notify that an object is no longer present.
 */
void Street_maintenance_schedule::notify_object_gone(Game_object *o)
{
	if (o == obj)
	{
		obj = 0;
		// Stops a random segfault.
		npc->set_action(0);		
	}
}

/*
 *	What do we do now?
 */

void Follow_avatar_schedule::now_what
	(
	)
	{
	bool is_blocked = blocked.tx != -1;
	blocked = Tile_coord(-1, -1, -1);
	if (npc->get_flag(Obj_flags::asleep) || npc->is_dead() ||
	    npc->get_flag(Obj_flags::paralyzed) ||
	    gwin->main_actor_dont_move())	// Under Usecode control.
		return;			// Disabled.
	Actor *av = gwin->get_main_actor();
	Tile_coord leaderpos = av->get_tile();
	Tile_coord pos = npc->get_tile();
	int dist2lead = av->distance(pos);
	if (!av->is_moving() &&		// Avatar stopped.
	    dist2lead <= 12)		// And we're already close enough.
		return;
	uint32 curtime = SDL_GetTicks();// Want the REAL time here.
	if (!is_blocked)		// Not blocked?
		{
		npc->follow(av);	// Then continue following.
		return;
		}
	if (curtime < next_path_time)	// Failed pathfinding recently?
		{			// Wait a bit.
		npc->start(gwin->get_std_delay(), next_path_time - curtime);
		return;
		}
					// Find a free spot within 3 tiles.
	Map_chunk::Find_spot_where where = Map_chunk::anywhere;
					// And try to be inside/outside.
	where = gwin->is_main_actor_inside() ?
					Map_chunk::inside : Map_chunk::outside;
	Tile_coord goal = Map_chunk::find_spot(leaderpos, 3, npc, 0, where);
	if (goal.tx == -1)		// No free spot?  Give up.
		{
		cout << npc->get_name() << " can't find free spot" << endl;
		next_path_time = SDL_GetTicks() + 1000;
		return;
		}
					// Get his speed.
	int speed = av->get_frame_time();
	if (!speed)			// Avatar stopped?
		speed = gwin->get_std_delay();
	if (pos.distance(goal) <= 3)
		return;			// Already close enough!
#if 0
#ifdef DEBUG
	cout << npc->get_name() << " at distance " << dist2lead 
				<< " trying to catch up." << endl;
#endif
#endif
					// Succeed if within 3 tiles of goal.
	if (npc->walk_path_to_tile(goal, speed - speed/4, 0, 3, 1))
		return;			// Success.
	cout << "... but failed to find path." << endl;
					// On screen (roughly)?
	int ok;
					// Get window rect. in tiles.
	Rectangle wrect = gwin->get_win_tile_rect();
	if (wrect.has_world_point(pos.tx - pos.tz/2, pos.ty - pos.tz/2))
					// Try walking off-screen.
		ok = npc->walk_path_to_tile(Tile_coord(-1, -1, -1),
							speed - speed/4, 0);
	else				// Off screen already?
		ok = npc->approach_another(av);
	if (!ok)			// Failed? Don't try again for a bit.
		next_path_time = SDL_GetTicks() + 1000;
	}

/*
 *	Waiting...
 */

void Wait_schedule::now_what
	(
	)
	{
	}

/*
 *	Create a horizontal pace schedule.
 */

Pace_schedule *Pace_schedule::create_horiz
	(
	Actor *n
	)
	{
	Tile_coord t = n->get_tile();	// Get his position.
	return (new Pace_schedule(n, 1, t));
	}

/*
 *	Create a vertical pace schedule.
 */

Pace_schedule *Pace_schedule::create_vert
	(
	Actor *n
	)
	{
	Tile_coord t = n->get_tile();	// Get his position.
	return (new Pace_schedule(n, 0, t));
	}

void Pace_schedule::pace(Actor *npc, char& which, int& phase, Tile_coord& blocked, int delay)
	{
	int dir = npc->get_dir_facing();	// Use NPC facing for starting direction
	switch (phase)
		{
		case 1:
			{
			bool changedir = false;
			switch (dir)
				{
				case north:
				case south:
					if (which)
						changedir = true;
					break;
				case east:
				case west:
					if (!which)
						changedir = true;
					break;
				}
			if (changedir)
				{
				phase = 4;
				npc->start(delay, delay);
				return;
				}
			if (blocked.tx != -1)		// Blocked?
				{
				Game_object *obj = npc->find_blocking(blocked, dir);
				if (obj)
					{
					if (obj->as_actor())
						{
						Monster_info *minfo = npc->get_info().get_monster_info();
						if (!minfo || !minfo->cant_yell())
							{
							npc->say(first_move_aside, last_move_aside);
								// Ask NPC to move aside.
							if (obj->move_aside(npc, dir))
									// Wait longer.
								npc->start(3*delay, 3*delay);
							else
									// Wait longer.
								npc->start(delay, delay);
							return;
							}
						}
					blocked.tx = -1;
					changedir = true;
					}
				}
			
			if (changedir)
				phase++;
			else
				{
				Tile_coord p0 = npc->get_tile().get_neighbor(dir);
				Frames_sequence *frames = npc->get_frames(dir);
				int& step_index = npc->get_step_index();
				if (!step_index)		// First time?  Init.
					step_index = frames->find_unrotated(npc->get_framenum());
								// Get next (updates step_index).
				int frame = frames->get_next(step_index);
					// One step at a time.
				npc->step(p0, frame);
				}
			npc->start(delay, delay);
			break;
			}
		case 2:
			phase++;
			npc->change_frame(npc->get_dir_framenum(
				npc->get_dir_facing(), Actor::standing));
			npc->start(3*delay, 3*delay);
			break;
		case 3:
		case 4:
			{
			phase++;
			const int facedirs[] = {west, north, north, east, east, south, south, west};
			npc->change_frame(npc->get_dir_framenum(
				facedirs[dir], Actor::standing));
			npc->start(3*delay, 3*delay);
			break;
			}
		default:
			phase = 1;
			npc->start(2*delay, 2*delay);
			break;
		}
	}

/*
 *	Schedule change for pacing:
 */

void Pace_schedule::now_what
	(
	)
	{
	if (rand() % 6 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.

	int delay = gwin->get_std_delay();
	if (!phase)
		{
		phase++;
		if (loc != npc->get_tile())
			npc->walk_to_tile(loc, delay, delay);
		else
			npc->start(delay, delay);
		}
	else
		Pace_schedule::pace(npc, which, phase, blocked, delay);
	}

/*
 *  Just went dormant.
 */
void Eat_at_inn_schedule::im_dormant()
{
	// Force NPCs to sit down again after cache-out/cache-in.
	//npc->set_frame(npc->get_dir_framenum(Actor::standing));
	npc->set_action(0);
}

/*
 *	Eat at inn.
 */

void Eat_at_inn_schedule::now_what
	(
	)
	{
	int frnum = npc->get_framenum();
	if ((frnum&0xf) != Actor::sit_frame)
		{			// First have to sit down.
		if (!Sit_schedule::set_action(npc))
					// Try again in a while.
			npc->start(250, 5000);
		return;
		}
	Game_object_vector foods;			// Food nearby?
	int cnt = npc->find_nearby(foods, 377, 2, 0);
	if (cnt)			// Found?
		{			// Find closest.
		Game_object *food = 0;
		int dist = 500;
		for (Game_object_vector::const_iterator it = foods.begin();
					it != foods.end(); ++it)
			{
			Game_object *obj = *it;
			int odist = obj->distance(npc);
			if (odist < dist)
				{
				dist = odist;
				food = obj;
				}
			}
		if (rand()%5 == 0 && food)
			{
			gwin->add_dirty(food);
			food->remove_this();
			}
		if (rand()%4)
			npc->say(first_munch, last_munch);
		}
	else if (rand()%4)
		npc->say(first_more_food, last_more_food);
					// Wake up in a little while.
	npc->start(250, 5000 + rand()%12000);
	}

// TODO: This should be in a loop to remove food one at a time with a delay
void Eat_at_inn_schedule::ending (int new_type) // new schedule type
{
	Game_object_vector foods;			// Food nearby?
	int cnt = npc->find_nearby(foods, 377, 2, 0);
	if (cnt){			// Found?
		for (Game_object_vector::const_iterator it = foods.begin();
					it != foods.end(); ++it){
			Game_object *food = *it;
			if (food){
				gwin->add_dirty(food);
				food->remove_this();
				npc->say(first_munch, last_munch);
			}
		}
	}
}


/*
 *	Find someone listening to the preacher.
 */

Actor *Find_congregant
	(
	Actor *npc
	)
	{
	Actor_vector vec, vec2;
	if (!npc->find_nearby_actors(vec, c_any_shapenum, 16))
		return 0;
	vec2.reserve(vec.size());	// Get list of ones to consider.
	for (Actor_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
		Actor *act = *it;
		if (act->get_schedule_type() == Schedule::sit &&
							!act->is_in_party())
			vec2.push_back(act);
		}
	return vec2.size() ? vec2[rand()%vec2.size()] : 0;
	}

/*
 *	Preach:
 */

void Preach_schedule::now_what
	(
	)
	{
	switch (state)
		{
	case find_podium:
		{
		Game_object_vector vec;
		if (!npc->find_nearby(vec, 697, 17, 0))
			{
			npc->set_schedule_type(loiter);
			return;
			}
		Game_object *podium = vec[0];
		Tile_coord pos = podium->get_tile();
		static int deltas[4][2] = {{-1, 0},{1, 0},{0, -2},{0, 1}};
		int frnum = podium->get_framenum()%4;
		pos.tx += deltas[frnum][0];
		pos.ty += deltas[frnum][1];
		Actor_pathfinder_client cost(npc, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
			npc->get_tile(), pos, cost);
		if (pact)
			{
			state = at_podium;
			npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(podium, 200)));
			npc->start(gwin->get_std_delay());
			return;
			}
		npc->start(250, 5000 + rand()%5000);	// Try again later.
		return;
		}
	case at_podium:
		if (rand()%2)		// Just wait a little.
			npc->start(gwin->get_std_delay(), rand()%3000);
		else 
			{
			if (rand()%3)
				state = exhort;
			else if ((GAME_SI) || rand()%3)
				state = visit;
			else
				state = find_icon;
			npc->start(gwin->get_std_delay(), 2000 + rand()%2000);
			}
		return;
	case exhort:
		{
		signed char frames[8];		// Frames.
		int cnt = 1 + rand()%(sizeof(frames) - 1);
					// Frames to choose from:
		static char choices[3] = {0, 8, 9};
		for (int i = 0; i < cnt - 1; i++)
			frames[i] = npc->get_dir_framenum(
					choices[rand()%(sizeof(choices))]);
					// Make last one standing.
		frames[cnt - 1] = npc->get_dir_framenum(Actor::standing);
		npc->set_action(new Frames_actor_action(frames, cnt, 250));
		npc->start(gwin->get_std_delay());
		npc->say(first_preach, last_preach);
		state = at_podium;
		Actor *member = Find_congregant(npc);
		if (member)
			{
			Usecode_script *scr = new Usecode_script(member);
			scr->add(Ucscript::delay_ticks, 3);
			scr->add(Ucscript::face_dir, member->get_dir_facing());
			scr->add(Ucscript::npc_frame + Actor::standing);
			scr->add(Ucscript::say, text_msgs[first_amen +
					rand()%(last_amen - first_amen + 1)]);
			scr->add(Ucscript::delay_ticks, 2);
			scr->add(Ucscript::npc_frame + Actor::sit_frame);
			scr->start();	// Start next tick.
			}
		return;
		}
	case visit:
		{
		state = find_podium;
		npc->start(gwin->get_std_delay(), 1000 + rand()%2000);
		Actor *member = Find_congregant(npc);
		if (!member)
			return;
		Tile_coord pos = member->get_tile();
		Actor_pathfinder_client cost(npc, 1);
		Actor_action *pact = Path_walking_actor_action::create_path(
			npc->get_tile(), pos, cost);
		if (!pact)
			return;
		npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(member, 200)));
		state = talk_member;
		return;
		}
	case talk_member:
		state = find_podium;
		npc->say(first_preach2, last_preach2);
		npc->start(250, 2000);
		return;
	case find_icon:
		{
		state = find_podium;		// In case we fail.
		npc->start(2*gwin->get_std_delay());
		Game_object *icon = npc->find_closest(724);
		if (!icon)
			return;
		Tile_coord pos = icon->get_tile();
		pos.tx += 2;
		pos.ty -= 1;
		Actor_pathfinder_client cost(npc, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
			npc->get_tile(), pos, cost);
		if (pact)
			{
			npc->set_action(pact);
			state = pray;
			}
		return;
		}
	case pray:
		{
		Usecode_script *scr = new Usecode_script(npc);
		(*scr) << Ucscript::face_dir << 6	// Face west.
			<< (Ucscript::npc_frame + Actor::standing)
			<< (Ucscript::npc_frame + Actor::bow_frame)
			<< Ucscript::delay_ticks << 3
			<< (Ucscript::npc_frame + Actor::kneel_frame);
		scr->add(Ucscript::say, text_msgs[first_amen + rand()%2]);
		(*scr)	<< Ucscript::delay_ticks << 5
			<< (Ucscript::npc_frame + Actor::bow_frame)
			<< Ucscript::delay_ticks << 3
			<< (Ucscript::npc_frame + Actor::standing);
			scr->start();	// Start next tick.
		state = find_podium;
		npc->start(2*gwin->get_std_delay(), 4000);
		return;
		}
	default:
		state = find_podium;
		npc->start(250);
		return;
		}
	}

/*
 *	Schedule change for patroling:
 */

void Patrol_schedule::now_what
	(
	)
	{
	if (rand() % 8 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
	if (seek_combat && seek_foes())	// Check for nearby foes.
		return;
	int speed = gwin->get_std_delay();
	const int PATH_SHAPE = 607;
	Game_object *path;
	switch (state)
		{
		case 0:	// Find next path.
			{
			pathnum += dir;			// Find next path.
			if (pathnum == 0 && dir == -1)
				dir = 1;	// Start over from zero.
			if (pathnum >= (int)paths.size())
				paths.resize(pathnum + 1);
							// Already know its location?
			path = pathnum >= 0 ? paths[pathnum] : 0;
			if (!path)			// No, so look around.
				{
				Game_object_vector nearby;
				npc->find_nearby(nearby, PATH_SHAPE, 25, 0x10, c_any_qual, pathnum);
				int best_dist = 1000;	// Find closest.
				for (Game_object_vector::const_iterator it = nearby.begin();
								it != nearby.end(); ++it)
					{
					Game_object *obj = *it;
					int dist;
					if ((dist = obj->distance(npc)) < best_dist)
						{	// Found it.
						path = obj;
						best_dist = dist;
						}
					}
				if (path)		// Save it.
					{
					failures = 0;
					paths[pathnum] = path;
					}
				else			// Turn back if at end.
					{
					failures++;
					dir = 1;
					if (pathnum == 0)	// At start?  Retry.
						{
						if (failures < 4)
							{
							pathnum = -1;
							npc->start(speed, 500);
								// Makes some enemies more prone to attacking.
							seek_combat = true;
							return;
							}
							// After 4, fall through.
						}
					else		// At end, go back to start.
						{
						if (wrap)
							{
							pathnum = 0;
							path = paths.size() ? paths[0] : 0;
							}
						else
							{
							pathnum = pathnum-2 >= 0 ? pathnum-2 : 0;
							dir = -1;
							path = paths.size() ? paths[pathnum] : 0;
							}
						}
					}
				if (!path)		// Still failed?
					{
							// Wiggle a bit.
					Tile_coord pos = npc->get_tile();
					int dist = failures + 2;
					Tile_coord delta = Tile_coord(rand()%dist - dist/2,
							rand()%dist - dist, 0);
					npc->walk_to_tile(pos + delta, speed, 
										failures*300);
					int pathcnt = paths.size();
					pathnum = rand()%(pathcnt < 4 ? 4 : pathcnt);
						// Makes some enemies more prone to attacking.
					seek_combat = true;
					return;
					}
				}
			Tile_coord d = path->get_tile();
			if (!npc->walk_path_to_tile(d, speed, 2*speed + rand()%500))
				{		
							// Look for free tile within 1 square.
				d = Map_chunk::find_spot(d, 1, npc->get_shapenum(),
								npc->get_framenum(), 1);
				if (d.tx == -1 || !npc->walk_path_to_tile(d,
							speed, rand()%1000))
					{		// Failed.  Later.
					npc->start(speed, 2000);
					return;
					}
				}
			state = 1;	// Walking to path.
			break;
			}
		case 1:	// Walk to next path.
			if (pathnum >= 0 &&		// Arrived at path?
				(unsigned int)pathnum < paths.size() &&
				(path = paths[pathnum]) != 0 &&	npc->distance(path) < 2)
				{
				int delay = 2;
				// Scripts for all actions. At worst, display standing frame
				// once the path egg is reached.
				Usecode_script *scr = new Usecode_script(npc);
				(*scr) << Ucscript::npc_frame + Actor::standing;
							// Quality = type.  (I think high bits
							// are flags).
				int qual = path->get_quality();
				seek_combat = (qual&32)!=0;
				// TODO: Find out what flags 64 and 128 mean. It would seem
				// that they are 'Repeat Forever' and 'Exc. Reserved.', but
				// what does those mean?
				switch (qual&31)
					{
				case 0:			// None.
					break;
				case 25:		// 50% wrap to 0.
					if (rand()%2)
						break;
					// Fall through to wrap.
				case 1:			// Wrap to 0.
					pathnum = -1;
					dir = 1;
					break;
				case 2:			// Pause; guessing 3 ticks.
					{
					(*scr) << Ucscript::delay_ticks << 3;
					delay = 5;
					break;
					}
				case 24:		// Read
					// Find the book which will be read.
					book = npc->find_closest(642, 4);
				    add_client(book);
					// Fall through to sit.
				case 3:			// Sit.
					if (Sit_schedule::set_action(npc))
						{
						scr->start();	// Start next tick.
						state = 2;
						return;
						}
					break;
				case 4:			// Kneel at tombstone.
				case 5:			// Kneel
					{	// Both seem to work identically.
					(*scr) << Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::bow_frame <<
						Ucscript::delay_ticks << 4 <<
						Ucscript::npc_frame + Actor::kneel_frame <<
						Ucscript::delay_ticks << 20 <<
						Ucscript::npc_frame + Actor::bow_frame <<
						Ucscript::delay_ticks << 4 <<
						Ucscript::npc_frame + Actor::standing; 
					delay = 36;
					break;
					}
				case 6:			// Loiter.
					{
					scr->start();	// Start next tick.
					center = path->get_tile();
					state = 4;
					npc->start(speed, speed*delay);
					return;
					}
				case 7:			// Right about-face.
				case 8:			// Left about-face.
					{
					wrap = false;	// Guessing; seems to match the original.
					const int dirs_left[] = {west, north, north, east, east, south, south, west};
					const int dirs_right[] = {east, east, south, south, west, west, north, north};
					const int *face_dirs;
					if ((path->get_quality()&31) == 7)
						face_dirs = dirs_right;
					else
						face_dirs = dirs_left;
					int facing = npc->get_dir_facing();
					for (int i=0; i<2; i++)
						{
						(*scr) << Ucscript::delay_ticks << 2 <<
						Ucscript::face_dir << face_dirs[facing];
						facing = face_dirs[facing];
						}
					delay = 8;
					break;
					}
				// Both 9 and 10 appear to pace vertically in the originals.
				// I am having 9 as vert. pace for the guards in Fawn.
				case 9:			// Vert. pace.
				case 10:		// Horiz. pace.
					whichdir = (qual&1)^1;
					pace_count = -1;
					phase = 1;
					scr->start();	// Start next tick.
					center = path->get_tile();
					state = 5;
					npc->start(speed, speed*delay);
					return;
				case 11:		// 50% reverse.
					if (rand()%2)
						dir *= -1;
					break;
				case 12:		// 50% skip next.
					{
					if (rand()%2)
						pathnum += dir;
					break;
					}
				case 13:		// Hammer.
					{
					if (!hammer) {	// Create hammer if does not exist.
						hammer = new Ireg_game_object(623, 0, 0, 0);
						add_client(hammer);
					}
						// For safety, unready weapon first.
					npc->unready_weapon();
					npc->add_dirty();
						// Ready the hammer in the weapon hand.
					npc->add_readied(hammer, lhand, 0, 1);
					npc->add_dirty();

					int hammersfx= Game::get_game_type() == BLACK_GATE ? 45:49;
					(*scr) << Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::ready_frame <<
						Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::raise1_frame <<
						Ucscript::delay_ticks << 2 <<
						Ucscript::sfx << hammersfx <<
						Ucscript::npc_frame + Actor::out_frame <<
						Ucscript::repeat << -11 << 1 <<
						Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::ready_frame <<
						Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::standing;
					delay = 24;
					break;
					}
				case 15:	// Usecode.
					{		// Don't let this script halt others,
							//   as it messes up automaton in
							//   SI-Freedom.
					(*scr) << Ucscript::usecode2 << npc->get_usecode() <<
						  static_cast<int>(Usecode_machine::npc_proximity);
					delay = 3;
					break;
					}
				case 16:		// Bow to ground.
				case 17:		// Bow from ground, seems to work exactly like 16
					{
					(*scr) << Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::bow_frame <<
						Ucscript::delay_ticks << 2 <<
						Ucscript::npc_frame + Actor::standing;
					delay = 8;
					break;
					}
				case 20:		// Ready weapon
					npc->ready_best_weapon();
					break;
				case 14:		// Check area.
					// Maybe could be improved?
					delay += 2;
					(*scr) << Ucscript::delay_ticks << 2;
				case 21:		// Unready weapon
					npc->unready_weapon();
					break;
				case 22:		// One-handed swing.
				case 23:		// Two-handed swing.
					{
					int dir = npc->get_dir_facing();
					signed char frames[12];		// Get frames to show.
					Game_object *weap = npc->get_readied(rhand);
					int cnt = npc->get_attack_frames(weap ? weap->get_shapenum() : 0,
								0, dir, frames);
					if (cnt)
						npc->set_action(new Frames_actor_action(frames, cnt, speed));
					npc->start(speed, speed*(delay+1));		// Get back into time queue.
					break;
					}
				// What should these two do???
				case 18:		// Wait for semaphore+++++
				case 19:		// Release semaphore+++++
				default:
#ifdef DEBUG
	cout << "Unhandled path egg quality in patrol schedule: " << qual << endl;
#endif
					break;
					}
				scr->start();	// Start next tick.
				state = 0;	// THEN, find next path.
				npc->start(speed, speed*delay);
				}
			else
				{
				state = 0;	// Walking to path.
				npc->start(speed, speed);
				}
			break;
		case 2:	// Sitting/reading.
					// Stay 5-15 secs.
			if ((npc->get_framenum()&0xf) == Actor::sit_frame)
				{
				if (book)
					{	// Open book if reading.
					int frnum = book->get_framenum();
					if (frnum%3)
						book->change_frame(frnum - frnum%3);
					else	// Book already open; we shouldn't close it then.
						book = 0;
					}
				npc->start(250, 5000 + rand()%10000);
				}
			else			// Not sitting.
				npc->start(250, rand()%1000);
			state = 3;	// Continue on afterward.
			break;
		case 3:	// Stand up.
			if ((npc->get_framenum()&0xf) == Actor::sit_frame)
				{
				if (book)
					{		// Close book we opened it.
					int frnum = book->get_framenum();
					book->change_frame(frnum - frnum%3 + 1);
					book = 0;
					}
				// Standing up animation.
				Usecode_script *scr = new Usecode_script(npc);
				(*scr) << Ucscript::delay_ticks << 2 <<
					Ucscript::npc_frame + Actor::bow_frame <<
					Ucscript::delay_ticks << 2 <<
					Ucscript::npc_frame + Actor::standing;
				scr->start();	// Start next tick.
				npc->start(speed, speed*7);
				}
			state = 0;
			break;
		case 4:	// Loiter.
			if (rand()%5 == 0)
				{
				state = 0;
				npc->start(speed, speed);
				}
			else
				{
				int dist = 12;
				int newx = center.tx - dist + rand()%(2*dist);
				int newy = center.ty - dist + rand()%(2*dist);
								// Wait a bit.
				npc->walk_to_tile(newx, newy, center.tz, speed, 
											rand()%2000);
				}
			break;
		case 5:	// Pacing.
			if (npc->get_tile().distance(center) < 1)
				{
				pace_count++;
				if (pace_count == 6)
					{
					npc->change_frame(npc->get_dir_framenum(
						npc->get_dir_facing(), Actor::standing));
					npc->start(3*speed, 3*speed);
					state = 0;
					break;
					}
				if (phase >= 2 && phase <= 4)
					pace_count--;
				}
			Pace_schedule::pace(npc, whichdir, phase, blocked, speed);
			break;
		default:
			// Just in case.
			break;
		}
	}


/*
 *	End of patrol
 */
	
void Patrol_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	remove_clients();
	if (hammer)
		{
		hammer->remove_this();
		hammer = 0;
		}
	}

/*
 *	Notify that an object is no longer present.
 */
void Patrol_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == hammer)	/* I don't think this can happen, but just in case.*/
		hammer = 0;
	else if (obj == book)
		book = 0;
}

/*
 *	Schedule change for 'talk':
 */

void Talk_schedule::now_what
	(
	)
	{

	int speed = gwin->get_std_delay();
	// Switch to phase 3 if we are reasonable close
	if (phase < 3 && 
	    npc->distance(gwin->get_main_actor()) < 6)
		{
		phase = 3;
		npc->start(speed, 250);
		return;
		}
	switch (phase)
		{
	case 0:				// Start by approaching Avatar.
		{
		if (npc->distance(gwin->get_main_actor()) > 50)
			{		// Too far?  
					// Try a little later.
			npc->start(speed, 5000);
			return;
			}
					// Aim for within 5 tiles.
		Actor_pathfinder_client cost(npc, 5);
		Actor_action *pact = Approach_actor_action::create_path(
			npc->get_tile(),
			gwin->get_main_actor(), 5, cost);
		if (!pact)
			{
#ifdef DEBUG
			cout << "Talk: Failed to find path for " << 
						npc->get_name() << endl;
#endif
			// No path found; try again a little later.
			npc->start(speed, 500);
			return;
			}
		else
			{
			if (Fast_pathfinder_client::is_grabable(npc,
					gwin->get_main_actor()))
				{
				if (rand()%3 == 0)
					npc->say(first_talk, last_talk);
				}
					// Walk there, and retry if
					//   blocked.
			npc->set_action(pact);
			npc->start(speed);	// Start walking.
			}
		phase++;
		return;
		}
	case 1:				// Wait a second.
	case 2:
		{
		if (Fast_pathfinder_client::is_grabable(npc,
				gwin->get_main_actor()))
			{
			if (rand()%3 == 0)
				npc->say(first_talk, last_talk);
			}
		// Step towards Avatar.
		Tile_coord pos = npc->get_tile(), 
			   dest = gwin->get_main_actor()->get_tile();
		int dx = dest.tx > pos.tx ? 1 : (dest.tx < pos.tx ? -1 : 0);
		int dy = dest.ty > pos.ty ? 1 : (dest.ty < pos.ty ? -1 : 0);
		npc->walk_to_tile(pos + Tile_coord(dx, dy, 0), speed, 500);
		phase = 3;
		return;
		}
	case 3:				// Talk.
		{
		int dist = npc->distance(gwin->get_main_actor());
					// Got to be close & reachable.
		if (dist > 5 ||
			!Fast_pathfinder_client::is_grabable(npc,
				gwin->get_main_actor()))
			{
			phase = 0;
			npc->start(speed, 500);
			return;
			}
					// But first face Avatar.
		npc->change_frame(npc->get_dir_framenum(npc->get_direction(
				gwin->get_main_actor()), Actor::standing));
		phase++;
		npc->start(speed, 250);	// Wait another 1/4 sec.
		break;
		}
	case 4:
		npc->stop();		// Stop moving.
					// NOTE:  This could DESTROY us!
		if (Game::get_game_type() == SERPENT_ISLE)
			npc->activate(9);
		else
			npc->activate(1);
					// SO don't refer to any instance
					//   variables from here on.
		gwin->paint();
		return;
	default:
		break;
		}
	}

/*
 *	Create a loiter schedule.
 */

Loiter_schedule::Loiter_schedule
	(
	Actor *n,
	int d				// Distance in tiles to roam.
	) : Schedule(n), center(n->get_tile()), dist(d)
	{
	}

/*
 *	Schedule change for 'loiter':
 */

void Loiter_schedule::now_what
	(
	)
	{
	if (rand() % 3 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
		// Pure guess. Since only some schedules seem to call proximity
		// usecode, I guess I should put it in here.
		// Seems rare for pure loiter, quite frequent for tend shop.
	if ((npc->get_schedule_type() == Schedule::loiter && try_proximity_usecode(12)) ||
	    (npc->get_schedule_type() == Schedule::tend_shop && try_proximity_usecode(8)))
		return;
	int newx = center.tx - dist + rand()%(2*dist);
	int newy = center.ty - dist + rand()%(2*dist);
					// Wait a bit.
	npc->walk_to_tile(newx, newy, center.tz, 2*gwin->get_std_delay(), 
								rand()%2000);
	}

/*
 *	Schedule change for kid games.
 */

void Kid_games_schedule::now_what
	(
	)
	{
	Tile_coord pos = npc->get_tile();
	Actor *kid = 0;			// Get a kid to chase.
					// But don't run too far.
	while (!kids.empty())
	{
		kid = kids.back();
		kids.pop_back();
		if (npc->distance(kid) < 16)
			break;
		kid = 0;
	}

	if (kid)
	{
		Fast_pathfinder_client cost(npc, kid->get_tile(), 1);
		Actor_action *pact = Path_walking_actor_action::create_path(
				pos, kid->get_tile(), cost);
		if (pact)
		{
			npc->set_action(pact);
			npc->start(100, 250); // Run.
			return;
		}
	}
	else				// No more kids?  Search.
	{
		Actor_vector vec;
		npc->find_nearby_actors(vec, c_any_shapenum, 16);
		for (Actor_vector::const_iterator it = vec.begin();
						it != vec.end(); ++it)
		{
			Actor *act = *it;
			if (act->get_schedule_type() == kid_games)
				kids.push_back(act);
		}
	}
	Loiter_schedule::now_what();	// Wander around the start.
}

/*
 *	Schedule change for 'dance':
 */

void Dance_schedule::now_what
	(
	)
	{
	Tile_coord dest = center;	// Pick new spot to walk to.
	dest.tx += -dist + rand()%(2*dist);
	dest.ty += -dist + rand()%(2*dist);
	Tile_coord cur = npc->get_tile();
	int dir = static_cast<int>(Get_direction4(cur.ty - dest.ty, 
							dest.tx - cur.tx));
		// Pure guess. Since only some schedules seem to call proximity
		// usecode, I guess I should put it in here.
		// Seems quite rare.
	if (try_proximity_usecode(8))
		return;		
	signed char *frames;
	int nframes;
	static char base_frames[] = {Actor::standing, Actor::up_frame, Actor::out_frame};
	int danceroutine = rand() % 5;
	int speed = 2 * gwin->get_std_delay();
	switch (danceroutine)
		{
		default:
			{			// Spin in place using one of several frames.
			signed char basefr = base_frames[danceroutine];
			nframes = 6;
			frames = new signed char[nframes];
			int i;
			for (i = 0; i < nframes - 2; i++)
				frames[i] = npc->get_dir_framenum((dir + 2*i)%8, basefr);
			frames[i] = npc->get_dir_framenum((dir + 2*i)%8, basefr);
			frames[i+1] = npc->get_dir_framenum((dir + 2*i)%8, Actor::standing);
			break;
			}
		case 3: // Flap arms
			{
				// Seems to repeat a random number of times.
			static char flap_frames[] = {Actor::up_frame, Actor::out_frame};
			nframes = 5 + 4 * (rand() % 4);
			frames = new signed char[nframes];
			int i;
			for (i = 0; i < nframes - 1; i++)
				frames[i] = npc->get_dir_framenum(dir, flap_frames[i % 2]);
			frames[i] = npc->get_dir_framenum(dir, Actor::standing);
			break;
			}
		case 4: // Punch
			{
			static signed char swing_frames[] = {Actor::ready_frame, Actor::raise1_frame,
					Actor::reach1_frame, Actor::strike1_frame, Actor::ready_frame};
			nframes = sizeof(swing_frames);
			frames = new signed char[nframes];
			for (int i = 0; i < nframes; i++)
				frames[i] = npc->get_dir_framenum(dir, swing_frames[i]);
			break;
			}
		}
							// Create action to walk.
	Actor_action *walk = new Path_walking_actor_action(new Zombie());
	walk->walk_to_tile(npc, cur, dest);
					// Walk, then spin.
	npc->set_action(new Sequence_actor_action(walk,
		new Frames_actor_action(frames, nframes, speed)));
	delete [] frames;
	npc->start(gwin->get_std_delay(), 500);		// Start in 1/2 sec.
	}

void Tool_schedule::get_tool
	(
	)
	{
	tool = new Ireg_game_object(toolshape, 0, 0, 0, 0);
					// Free up both hands.
	Game_object *obj = npc->get_readied(rhand);
	if (obj)
		obj->remove_this();
	if ((obj = npc->get_readied(lhand)) != 0)
		obj->remove_this();
	npc->add_readied(tool, lhand);
	}

/*
 *	Schedule change for tool-user (currently only farming).
 */

void Tool_schedule::now_what
	(
	)
	{
	if (!tool)			// First time?
		get_tool();

	if (rand()%4 == 0)		// 1/4 time, walk somewhere.
		{
		Loiter_schedule::now_what();
		return;
		}
	if (rand()%10 == 0)
		{
		Schedule_types ty = (Schedule_types) npc->get_schedule_type();
		if (ty == Schedule::farm) {
			if (rand()%2)
				npc->say(first_farmer, last_farmer);
			else
				npc->say(first_farmer2, last_farmer2);
			}
		}
	signed char frames[12];		// Use tool.
	int cnt = npc->get_attack_frames(toolshape, false, rand()%8, frames);
	if (cnt)
		npc->set_action(new Frames_actor_action(frames, cnt));
	npc->start();			// Get back into time queue.
	}

/*
 *	End of mining/farming:
 */

void Tool_schedule::ending
	(
	int
	)
	{
	if (tool)
		{
		tool->remove_this();	// Should safely remove from NPC.
		tool = 0;
		}
	}

/*
 *	Miners attack ore.
 */

void Miner_schedule::now_what
	(
	)
	{
	int delay = 0;
	if (!tool)			// First time?
		get_tool();
	switch (state) {
	case find_ore:
		{
		static int oreshapes[] = {915, 916};
		Game_object_vector ores;
		npc->find_closest(ores, oreshapes,
					sizeof(ores)/sizeof(ores[0]));
		int from, to, cnt = ores.size();
		// Filter out frame #3 (dust).
		for (from = to = 0; from < cnt; ++from)
			if (ores[from]->get_framenum() < 3)
				ores[to++] = ores[from];
		cnt = to;
		if (cnt) {
			ore = ores[rand()%cnt];
			Actor_pathfinder_client cost(npc, 2);
			Actor_action *pact = 
				Path_walking_actor_action::create_path(
				     npc->get_tile(), ore->get_tile(), cost);
			if (pact)
				{
				state = attack_ore;
				npc->set_action(new Sequence_actor_action(pact,
				    new Face_pos_actor_action(ore, 200)));
				break;
				}
		}
		ore = 0;
		state = wander;
		delay = 1000 + rand()%1000;	// Try again later.
		break;
		}
	case attack_ore:
		{
		if (ore->is_pos_invalid() || npc->distance(ore) > 2) {
			state = find_ore;
			break;
		}
		signed char frames[20];		// Use pick.
		int dir = npc->get_direction(ore);
		int cnt = npc->get_attack_frames(toolshape, false, dir,
								frames);
		if (cnt) {
			frames[cnt++] = npc->get_dir_framenum(
						dir, Actor::standing);
			npc->set_action(new Frames_actor_action(frames, cnt));
			state = ore_attacked;
		} else
			state = wander;
		break;
		}
	case ore_attacked:
		if (ore->is_pos_invalid()) {
			state = find_ore;
			break;
		}
		state = attack_ore;
		if (rand()%6 == 0) {		// Break up piece.
			int shnum, frnum = ore->get_framenum();
			if (frnum == 3)
				state = find_ore;	// Dust.
			else if (rand()%(4+2*frnum) == 0) {
				npc->say(first_miner_gold, last_miner_gold);
				Tile_coord pos = ore->get_tile();
				ore->remove_this();
				ore = 0;
				if (frnum == 0) {	// Gold.
					shnum = 645;
					frnum = rand()%2;
				} else {		// Gem.
					shnum = 760;
					frnum = rand()%10;
				}
				Game_object *newobj = new Ireg_game_object(
							shnum, frnum, 0, 0);
				newobj->move(pos);
				newobj->set_flag(Obj_flags::is_temporary);
				state = find_ore;
				break;
			} else {
				ore->change_frame(frnum + 1);
				if (ore->get_framenum() == 3)
					state = find_ore;// Dust.
			}
		}
		if (rand()%4 == 0) {
			npc->say(first_miner, last_miner);
		}
		delay = 500 + rand()%2000;
		break;
	case wander:
		if (rand()%2 == 0) {
			Loiter_schedule::now_what();
			return;
		} else
			state = find_ore;
		break;
	}
	npc->start(gwin->get_std_delay(), delay);
	}

/*
 *	Schedule change for hounding the Avatar.
 */

void Hound_schedule::now_what
	(
	)
	{
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_tile(),
		   npcpos = npc->get_tile();
					// How far away is Avatar?
	int dist = npc->distance(av);
	if (dist > 20 || dist < 3)	// Too far, or close enough?
		{			// Check again in a few seconds.
		npc->start(gwin->get_std_delay(), 500 + rand()%1000);
		return;
		}
		// Pure guess. Since only some schedules seem to call proximity
		// usecode, I guess I should put it in here.
		// Seems quite rare.
	if (try_proximity_usecode(12))
		return;
	int newdist = 1 + rand()%2;	// Aim for about 3 tiles from Avatar.
	avpos.tx += rand()%3 - 1;	// Vary a bit randomly.
	avpos.ty += rand()%3 - 1;
	Fast_pathfinder_client cost(npc, avpos, newdist);
	Actor_action *pact = Path_walking_actor_action::create_path(npcpos,
							avpos, cost);
	if (pact)
		{
		npc->set_action(pact);
		npc->start(gwin->get_std_delay(), 50);
		}
	else				// Try again.
		npc->start(gwin->get_std_delay(), 2000 + rand()%3000);
	}

/*
 *	Schedule change for 'wander':
 */

void Wander_schedule::now_what
	(
	)
	{
	if (rand() % 2)			// 1/2 time, check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
	Tile_coord pos = npc->get_tile();
	const int legdist = 32;
					// Go a ways from current pos.
	pos.tx += -legdist + rand()%(2*legdist);
	pos.ty += -legdist + rand()%(2*legdist);
					// Don't go too far from center.
	if (pos.tx - center.tx > dist)
		pos.tx = center.tx + dist;
	else if (center.tx - pos.tx > dist)
		pos.tx = center.tx - dist;
	if (pos.ty - center.ty > dist)
		pos.ty = center.ty + dist;
	else if (center.ty - pos.ty > dist)
		pos.ty = center.ty - dist;
					// Find a free spot.
	Tile_coord dest = Map_chunk::find_spot(pos, 4, npc->get_shapenum(), 0,
									1);
	if (dest.tx == -1 || !npc->walk_path_to_tile(dest,
					gwin->get_std_delay(), rand()%2000))
					// Failed?  Try again a little later.
		npc->start(250, rand()%3000);
	}

/*
 *	Stand up if not already.
 */

static void Stand_up
	(
	Actor *npc
	)
	{
	if ((npc->get_framenum()&0xf) != Actor::standing)
					// Stand.
		npc->change_frame(Actor::standing);
	}

/*
 *	Create a sleep schedule.
 */

Sleep_schedule::Sleep_schedule
	(
	Actor *n
	) : Schedule(n), bed(0), state(0), for_nap_time(false)
	{
	floorloc.tx = -1;		// Set to invalid loc.
	if (Game::get_game_type() == BLACK_GATE)
		{
		spread0 = 3;
		spread1 = 16;
		}
	else				// Serpent Isle.
		{
		spread0 = 7;
		spread1 = 20;
		}
	}

/*
 * Checks if a bed is being used by a sleeping NPC. The second
 * parameter is a 'whitelist' NPC which will be ignored, and
 * should be used for preventing an NPC from detecting himself
 * as occupying the bed.
 */

bool Sleep_schedule::is_bed_occupied
	(
	Game_object *bed,
	Actor *npc
	)
	{
			// Check if someone is already in the bed.
	Actor_vector occ;	// Unless there's another occupant.
	bed->find_nearby_actors(occ, c_any_shapenum, 2);
	Rectangle foot = bed->get_footprint();
	int bedz = bed->get_tile().tz/5;
	for (Actor_vector::iterator it = occ.begin(); it != occ.end(); ++it)
		{
		if (npc == *it)
			continue;
		Tile_coord tn = (*it)->get_tile();
		if (tn.tz/5 == bedz && ((*it)->get_framenum()&0xf) == Actor::sleep_frame &&
				foot.has_world_point(tn.tx, tn.ty))
			return true;
		}
	return false;
	}

/*
 *	Schedule change for 'sleep':
 */

void Sleep_schedule::now_what
	(
	)
	{
	if (bed && state <= 1 && is_bed_occupied(bed, npc))
		{
		bed = 0;
		state = 0;
		npc->stop();
		}
	if (!bed && state == 0 &&		// Always find bed.
			!npc->get_flag(Obj_flags::asleep))	// Unless flag already set
		{			// Find closest EW or NS bed.
		vector<int> bedshapes;
		bedshapes.push_back(696);
		bedshapes.push_back(1011);
		if (GAME_BG)
			{
			bedshapes.push_back(363);
			bedshapes.push_back(312);
			}
		Game_object_vector beds;	// Want to find top of bed.
		for (vector<int>::iterator it = bedshapes.begin();
		     it != bedshapes.end(); ++it)
			npc->find_nearby(beds, *it, 24, 0);
		int best_dist = 100;
		for (Game_object_vector::iterator it = beds.begin();
		     it != beds.end(); ++it)
			{
			Game_object *newbed = *it;
			int newdist = npc->distance(newbed);
			if (newdist < best_dist && !is_bed_occupied(newbed, npc))
				{
				best_dist = newdist;
				bed = newbed;
				}
			}
		}
	add_client(bed);
	int frnum = npc->get_framenum();
	if ((frnum&0xf) == Actor::sleep_frame)
		return;			// Already sleeping.

	switch (state)
		{
	case 0:				// Find path to bed.
		{
		if (!bed)
			{		// Just lie down at current spot.
			int dirbits = npc->get_framenum()&(0x30);
			npc->change_frame(Actor::sleep_frame|dirbits);
			npc->force_sleep();
			return;
			}
		state = 1;
		Game_object_vector tops;	// Want to find top of bed.
		bed->find_nearby(tops, bed->get_shapenum(), 1, 0);
		int floor = (bed->get_tile().tz) / 5;
		for (Game_object_vector::const_iterator it = tops.begin(); 
						it != tops.end(); ++it)
			{
			Game_object *top = *it;
			int frnum = top->get_framenum();
			Tile_coord tpos = top->get_tile();
					// Restrict to sheets on the same floor as the selected bed.
			if (frnum >= spread0 && frnum <= spread1 && floor == (tpos.tz/5))
				{
				bed = top;
				break;
				}
			}
		Tile_coord bloc = bed->get_tile();
		bloc.tz -= bloc.tz%5;	// Round down to floor level.
		Shape_info& info = bed->get_info();
		bloc.tx -= info.get_3d_xtiles(bed->get_framenum())/2;
		bloc.ty -= info.get_3d_ytiles(bed->get_framenum())/2;
					// Get within 3 tiles.
		Actor_pathfinder_client cost(npc, 3);
		Actor_action *pact = Path_walking_actor_action::create_path(
				npc->get_tile(), bloc, cost);
		if (pact)
			npc->set_action(pact);
		npc->start(200);	// Start walking.
		break;
		}
	case 1:				// Go to bed.
		{
		npc->stop();		// Just to be sure.
		int bedshape = bed->get_shapenum();
		int dir = (bedshape == 696 || bedshape == 363) ? west : north;
		npc->set_frame(npc->get_dir_framenum(dir, Actor::sleep_frame));
					// Get bed info.
		Shape_info& info = bed->get_info();
		Tile_coord bedloc = bed->get_tile();
		floorloc = npc->get_tile();
		Usecode_script::terminate(bed);
		int bedframe = bed->get_framenum();// Unmake bed.
		if (bedframe >= spread0 && bedframe < spread1 && (bedframe%2))
			{
			bedframe++;
			bed->change_frame(bedframe);
			}
		int bedspread = (bedframe >= spread0 && !(bedframe%2));
					// Put NPC on top of bed, making sure that children are
					// not completely covered by sheets.
		int delta = npc->get_info().get_3d_height();
		delta = (delta < 4) ? (delta - 4) : 0;
		npc->move(bedloc.tx + delta, bedloc.ty + delta, bedloc.tz + 
				(bedspread ? 0 : info.get_3d_height()));
					// Either call bed usecode for avatar or put NPC to sleep.
		npc->force_sleep();
		state = 2;
		if (for_nap_time)		// Usecode 622 handles sleeping.
			{					// Calling it may delete us, though.
			ucmachine->call_usecode(0x622, bed, Usecode_machine::double_click);
			return; 			// So leave nothing to chance.
			}
		break;
		}
	default:
		break;
		}
	}

/*
 *  Just went dormant.
 */
void Sleep_schedule::im_dormant()
{
	// Force NPCs to lay down again after cache-out/cache-in.
	//npc->set_frame(npc->get_dir_framenum(Actor::standing));
	npc->set_action(0);
}

/*
 *	Wakeup time.
 */

void Sleep_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	remove_clients();
					// Needed for Skara Brae.
	if (new_type == static_cast<int>(wait) ||	
					// Not time to get up, Penumbra!
	    new_type == static_cast<int>(sleep))
		return;			// ++++Does this leave NPC's stuck?++++

	bool makebed = false;
	int dir;
	if (bed &&			// Still in bed?
	    (npc->get_framenum()&0xf) == Actor::sleep_frame &&
	    npc->distance(bed) < 8)
		{			// Locate free spot.
		if (floorloc.tx == -1)
			{
					// Want spot on floor.
			floorloc = npc->get_tile();
					// Note: doing this in all cases may cause incorrect
					// placement when getting out of bed (e.g., top floor
					// bed in Spark's house.
			floorloc.tz -= floorloc.tz%5;
			}
		Tile_coord pos = Map_chunk::find_spot(floorloc, 
				6, npc->get_shapenum(), 
				static_cast<int>(Actor::standing), 0);
		if (pos.tx == -1)	// Failed?  Allow change in lift.
			pos = Map_chunk::find_spot(floorloc,
				6, npc->get_shapenum(), 
				static_cast<int>(Actor::standing), 1);
		floorloc = pos;
					// Make bed.
		int frnum = bed->get_framenum();
		if (new_type != static_cast<int>(combat) &&	 // Not if going into combat
		      frnum >= spread0 && frnum <= spread1 && !(frnum%2) &&
			  is_bed_occupied(bed, npc)) // And not if there is another occupant.
			{				// Make the bed set itself after a small delay.
			makebed = true;
			Usecode_script *scr = new Usecode_script(bed);
			(*scr) << Ucscript::finish
			       << Ucscript::delay_ticks << 3 << Ucscript::frame << frnum - 1;
			scr->start();
			Tile_coord bloc = bed->get_center_tile();
							// Treat as cartesian coords.
			dir = static_cast<int>(Get_direction(floorloc.ty - bloc.ty, bloc.tx - floorloc.tx));
			}
		}
	if (floorloc.tx >= 0)		// Get back on floor.
		npc->move(floorloc);
	npc->clear_sleep();
	npc->set_frame(npc->get_dir_framenum(Actor::standing));
	if (makebed)
		{				// Animation for making bed.
		Usecode_script *scr = new Usecode_script(npc);
		(*scr) << Ucscript::dont_halt << Ucscript::face_dir << dir
		       << Ucscript::npc_frame + Actor::ready_frame
		       << Ucscript::delay_ticks << 1 << Ucscript::npc_frame + Actor::raise1_frame
		       << Ucscript::delay_ticks << 1 << Ucscript::npc_frame + Actor::ready_frame
		       << Ucscript::delay_ticks << 1 << Ucscript::npc_frame + Actor::standing;
		scr->start();
		}
	gwin->set_all_dirty();		// Update all, since Av. stands up.
	state = 0;			// In case we go back to sleep.
	}

/*
 *	Notify that an object is no longer present.
 */
void Sleep_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == bed) {
		bed = 0;
		state = 0;
	}
}

void Sleep_schedule::set_bed(Game_object *b)
{
	if (bed)
		remove_clients();
	npc->set_action(0);
	bed = b;
	state = 0;
	for_nap_time = true;
}

/*
 *	Create a 'sit' schedule.
 */

Sit_schedule::Sit_schedule
	(
	Actor *n,
	Game_object *ch			// Chair, or null to find one.
	) : Schedule(n), chair(ch), sat(false), did_barge_usecode(false)
	{
	}

/*
 *	Schedule change for 'sit':
 */

void Sit_schedule::now_what
	(
	)
	{
	int frnum = npc->get_framenum();
	if (chair && (frnum&0xf) == Actor::sit_frame && 
	    npc->distance(chair) <= 1)
		{			// Already sitting.
					// Seat on barge?
		if (chair->get_info().get_barge_type() != Shape_info::barge_seat)
			return;
		if (did_barge_usecode)
			return;		// But NOT more than once for party.
		did_barge_usecode = true;
		if (gwin->get_moving_barge())
			return;		// Already moving.
		if (!npc->is_in_party())
			return;		// Not a party member.
		Actor *party[9];	// See if all sitting.
		int cnt = gwin->get_party(&party[0], 1);
		for (int i = 0; i < cnt; i++)
			if ((party[i]->get_framenum()&0xf) != Actor::sit_frame)
				return;	// Nope.
					// Find barge.
		Game_object *barge = chair->find_closest(961);
		if (!barge)
			return;
		int usefun = 0x634;	// I hate using constants like this.
		did_barge_usecode = true;
					// Special usecode for barge pieces:
					// (Call with item=Avatar to avoid
					//   running nearby barges.)
		ucmachine->call_usecode(usefun, gwin->get_main_actor(),
					Usecode_machine::double_click);
		return;
		}
					// Wait a while if we got up.
	if (!set_action(npc, chair, sat ? (1000 + rand()%1000) : 0, &chair))
		npc->start(200, 1000);	// Failed?  Try again later.
	else {
		add_client(chair);
		sat = true;
	}
}

/*
 *  Just went dormant.
 */
void Sit_schedule::im_dormant()
{
	// Force NPCs to sit down again after cache-out/cache-in.
	//npc->set_frame(npc->get_dir_framenum(Actor::standing));
	npc->set_action(0);
}

/*
 *	Notify that an object is no longer present.
 */
void Sit_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == chair) {
		chair = 0;
		sat = false;
	}
}

/*
 *	Action to sit in the chair NPC is in front of.
 */

class Sit_actor_action : public Frames_actor_action, public Game_singletons
	{
	Game_object *chair;		// Chair.
	Tile_coord chairloc;		// Original chair location.
	Tile_coord sitloc;		// Actually where NPC sits.
	signed char frames[2];
	static short offsets[8];	// Offsets where NPC should sit.
	signed char *init(Game_object *chairobj, Actor *actor)
		{
					// Frame 0 faces N, 1 E, etc.
		int dir = 2*(chairobj->get_framenum()%4);
		frames[0] = actor->get_dir_framenum(dir, Actor::bow_frame);
		frames[1] = actor->get_dir_framenum(dir, Actor::sit_frame);
		return frames;
		}
	static bool is_occupied(Tile_coord const& sitloc, Actor *actor)
		{
		Game_object_vector occ;	// See if occupied.
		if (Game_object::find_nearby(occ, sitloc,c_any_shapenum, 0, 8))
			for (Game_object_vector::const_iterator it = 
					occ.begin(); it != occ.end(); ++it)
				{
				Game_object *npc = *it;
				if (npc == actor)
					continue;
				int frnum = npc->get_framenum() & 15;
				if (frnum == Actor::sit_frame ||
				    frnum == Actor::bow_frame)
					return true;
				}
#if 1	/* Seems to work.  Added Nov. 2, 2001 */
		if (actor->get_tile() == sitloc)
			return false;	// We're standing there.
					// See if spot is blocked.
		Tile_coord pos = sitloc;// Careful, .tz gets updated.
		if (Map_chunk::is_blocked(pos, 
			    actor->get_info().get_3d_height(), MOVE_WALK, 0))
			return true;
#endif
		return false;
		}
public:
	Sit_actor_action(Game_object *o, Actor *actor)
		: Frames_actor_action(init(o, actor), 2), chair(o)
		{
		sitloc = chairloc = o->get_tile();
					// Frame 0 faces N, 1 E, etc.
		int nsew = o->get_framenum()%4;
		sitloc.tx += offsets[2*nsew];
		sitloc.ty += offsets[2*nsew + 1];
		}
	Tile_coord get_sitloc() const
		{ return sitloc; }
	static bool is_occupied(Game_object *chair, Actor *actor)
		{
		int dir = 2*(chair->get_framenum()%4);
		return is_occupied(chair->get_tile() +
			Tile_coord(offsets[dir], offsets[dir + 1], 0), actor);
		}
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

					// Offsets where NPC should sit:
short Sit_actor_action::offsets[8] = {0,-1, 1,0, 0,1, -1,0};

/*
 *	Show frame for sitting down.
 */

int Sit_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (get_index() == 0)		// First time?
		{
		if (is_occupied(sitloc, actor))
			return 0;	// Abort.
		if (chair->get_tile() != chairloc)
			{		// Chair was moved!
			actor->say(first_chair_thief, last_chair_thief);
			return 0;
			}
		}
	return Frames_actor_action::handle_event(actor);
	}

/*
 *	Is chair occupied, other than by 'actor'?
 */

bool Sit_schedule::is_occupied
	(
	Game_object *chairobj,
	Actor *actor
	)
	{
	return Sit_actor_action::is_occupied(chairobj, actor);
	}

/*
 *	Set up action.
 *
 *	Output:	false if couldn't find a free chair.
 */

bool Sit_schedule::set_action
	(
	Actor *actor,
	Game_object *chairobj,		// May be 0 to find it.
	int delay,			// Msecs. to delay.
	Game_object **chair_found	// ->chair ret'd if not NULL.
	)
	{
	static int chairshapes[] = {873,292};
	Game_object_vector chairs;
	if (chair_found)
		*chair_found = 0;	// Init. in case we fail.
	if (!chairobj)			// Find chair if not given.
		{
		actor->find_closest(chairs, chairshapes,
					sizeof(chairs)/sizeof(chairs[0]));
		for (Game_object_vector::const_iterator it = chairs.begin();
						it != chairs.end(); ++it)
			if (!Sit_actor_action::is_occupied((*it), actor))
				{	// Found an unused one.
				chairobj = *it;
				break;
				}
		if (!chairobj)
			return false;
		}
	else if (Sit_actor_action::is_occupied(chairobj, actor))
		return false;		// Given chair is occupied.
	Sit_actor_action *act = new Sit_actor_action(chairobj, actor);
					// Walk there, then sit.
	set_action_sequence(actor, act->get_sitloc(), act, false, delay);
	if (chair_found)
		*chair_found = chairobj;
	return true;
	}

/*
 *	Desk work.
 */

Desk_schedule::Desk_schedule
	(
	Actor *n
	) : Schedule(n), chair(0)
	{
	}

/*
 *	Schedule change for 'desk work':
 */

void Desk_schedule::now_what
	(
	)
	{
	if (!chair)			// No chair found yet.
		{
		static int desks[2] = {283, 407};
		static int chairs[2] = {873,292};
		Stand_up(npc);
		Game_object *desk = npc->find_closest(desks, 2);
		if (desk)
			chair = desk->find_closest(chairs, 2);
		if (!chair)		// Failed.
			{		// Try again in a few seconds.
			npc->start(200, 5000);
			return;	
			}
		add_client(chair);
		}
	int frnum = npc->get_framenum();
	if ((frnum&0xf) != Actor::sit_frame)
		{
		if (!Sit_schedule::set_action(npc, chair, 0))
			{
			chair = 0;	// Look for any nearby chair.
			npc->start(200, 5000);	// Failed?  Try again later.
			}
		else
			npc->start(250, 0);
		}
	else				// Stand up a second.
		{
		signed char frames[3];
		frames[0] = npc->get_dir_framenum(Actor::standing);
		frames[1] = npc->get_dir_framenum(Actor::bow_frame);
		frames[2] = npc->get_dir_framenum(Actor::sit_frame);
		npc->set_action(new Frames_actor_action(frames,
					sizeof(frames)/sizeof(frames[0])));
		npc->start(250, 10000 + rand()%5000);
		}
	}

/*
 *  Just went dormant.
 */
void Desk_schedule::im_dormant()
{
	// Force NPCs to sit down again after cache-out/cache-in.
	//npc->set_frame(npc->get_dir_framenum(Actor::standing));
	npc->set_action(0);
}

/*
 *	Notify that an object is no longer present.
 */
void Desk_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == chair) {
		chair = 0;
	}
}

/*
 *	A class for indexing the perimeter of a rectangle.
 */
class Perimeter
	{
	Rectangle perim;		// Outside given rect.
	int sz;				// # squares.
public:
	Perimeter(Rectangle &r) : sz(2*r.w + 2*r.h + 4)
		{
		perim = r;
		perim.enlarge(1);
		}
	int size() { return sz; }
					// Get i'th tile.
	void get(int i, Tile_coord& ptile, Tile_coord& atile);
#if 0
	static void test()
		{
		Rectangle r(10, 10, 3, 4);
		Perimeter p(r);
		int cnt = p.size();
		for (int i = 0; i < cnt; i++)
			{
			Tile_coord pt, at;
			p.get(i, pt, at);
			cout << "pt.tx = " << pt.tx << ", pt.ty = " << pt.ty
				<< ", at.tx = " << at.tx << ", at.ty = " <<
				at.ty << endl;
			}
		}
#endif
	};

/*
 *	Get the i'th perimeter tile and the tile in the original rect.
 *	that's adjacent.
 */

void Perimeter::get
	(
	int i,
	Tile_coord& ptile,		// Perim. tile returned.
	Tile_coord& atile		// Adjacent tile returned.
	)
	{
	if (i < perim.w - 1)		// Spiral around from top-left.
		{
		ptile = Tile_coord(perim.x + i, perim.y, 0);
		atile = ptile + Tile_coord(!i ? 1 : 0, 1, 0);
		return;
		}
	i -= perim.w - 1;
	if (i < perim.h - 1)
		{
		ptile = Tile_coord(perim.x + perim.w - 1, perim.y + i, 0);
		atile = ptile + Tile_coord(-1, !i ? 1 : 0, 0);
		return;
		}
	i -= perim.h - 1;
	if (i < perim.w - 1)
		{
		ptile = Tile_coord(perim.x + perim.w - 1 - i,
					perim.y + perim.h - 1, 0);
		atile = ptile + Tile_coord(!i ? -1 : 0, -1, 0);
		return;
		}
	i -= perim.w - 1;
		{
		ptile = Tile_coord(perim.x, perim.y + perim.h - 1 - i, 0);
		atile = ptile + Tile_coord(1, !i ? -1 : 0, 0);
		return;
		}
					// Bad index if here.
	get(i%sz, ptile, atile);
	}

/*
 *	Initialize.
 */

void Lab_schedule::init
	(
	)
	{
	chair = book = 0;
	cauldron = npc->find_closest(995, 20);
	add_client(cauldron);
					// Find 'lab' tables.
	npc->find_nearby(tables, 1003, 20, 0);
	npc->find_nearby(tables, 1018, 20, 0);
	add_client(tables);
	int cnt = tables.size();	// Look for book, chair.
	for (int i = 0; (!book || !chair) && i < cnt; i++) {
		static int chairs[2] = {873,292};
		Game_object *table = tables[i];
		Rectangle foot = table->get_footprint();
					// Book on table?
		if (!book && (book = table->find_closest(642, 4)) != 0)
			{
			Tile_coord p = book->get_tile();
			if (!foot.has_world_point(p.tx, p.ty))
				book = 0;
			else
				add_client(book);
			}
		if (!chair) {
			chair = table->find_closest(chairs, 2, 4);
			add_client(chair);
		}
	}
}

/*
 *	Create lab schedule.
 */

Lab_schedule::Lab_schedule
	(
	Actor *n
	) : Schedule(n), state(start)
	{
	init();
	}

/*
 *	Lab work.
 */

void Lab_schedule::now_what
	(
	)
	{
	Tile_coord npcpos = npc->get_tile();
	int delay = 100;		// 1/10 sec. to next action.
					// Often want to get within 1 tile.
	Actor_pathfinder_client cost(npc, 1);
	switch (state)
		{
	case start:
	default:
		{
		if (!cauldron)
			{		// Try looking again.
			init();
			if (!cauldron)	// Again a little later.
				delay = 6000;
			break;
			}
		int r = rand()%5;	// Pick a state.
		if (!r)			// Sit less often.
			state = sit_down;
		else
			state = r <= 2 ? walk_to_cauldron : walk_to_table;
		break;
		}
	case walk_to_cauldron:
		{
		state = start;		// In case we fail.
		if (!cauldron)
			break;
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, cauldron->get_tile(), cost);
		if (pact)
			{
			npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(cauldron, 200)));
			state = use_cauldron;
			}
		break;
		}
	case use_cauldron:
		{
		int dir = npc->get_direction(cauldron);
					// Set random frame (skip last frame).
		cauldron->change_frame(rand()%(cauldron->get_num_frames() -1));
		npc->change_frame(
			npc->get_dir_framenum(dir, Actor::bow_frame));
		int r = rand()%5;
		state = !r ? use_cauldron : (r <= 2 ? sit_down
						: walk_to_table);
		break;
		}
	case sit_down:
		if (!chair || !Sit_schedule::set_action(npc, chair, 200))
			state = start;
		else
			state = read_book;
		break;
	case read_book:
		{
		state = stand_up;
		if (!book || npc->distance(book) > 4)
			break;
					// Read a little while.
		delay = 1000 + 1000*(rand()%5);
					// Open book.
		int frnum = book->get_framenum();
		book->change_frame(frnum - frnum%3);
		break;
		}
	case stand_up:
		{
		if (book && npc->distance(book) < 4)
			{		// Close book.
			int frnum = book->get_framenum();
			book->change_frame(frnum - frnum%3 + 1);
			}
		state = start;
		break;
		}
	case walk_to_table:
		{
		state = start;		// In case we fail.
		int ntables = tables.size();
		if (!ntables)
			break;
		Game_object *table = tables[rand()%ntables];
		Rectangle r = table->get_footprint();
		Perimeter p(r);		// Find spot adjacent to table.
		Tile_coord spot;	// Also get closest spot on table.
		p.get(rand()%p.size(), spot, spot_on_table);
		Actor_pathfinder_client cost0(npc, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
							npcpos, spot, cost0);
		if (!pact)
			break;		// Failed.
		Shape_info& info = table->get_info();
		spot_on_table.tz += info.get_3d_height();
		npc->set_action(new Sequence_actor_action(pact,
			new Face_pos_actor_action(spot_on_table, 200)));
		state = use_potion;
		break;
		}
	case use_potion:
		{
		state = start;
		vector<Game_object *> potions;
		Game_object::find_nearby(potions, spot_on_table, 340, 0, 0);
		if (!potions.empty())	// Found a potion.  Remove it.
			{
			gwin->add_dirty(potions[0]);
			potions[0]->remove_this();
			}
		else			// Create potion if spot is empty.
			{
			Tile_coord t = Map_chunk::find_spot(spot_on_table, 0,
							340, 0, 0);
			if (t.tx != -1 && t.tz == spot_on_table.tz)
				{
				// create a potion randomly, but don't use the last frame
				int nframes = ShapeID(340, 0).get_num_frames() - 1;
				Game_object *p = gmap->create_ireg_object(
					ShapeID::get_info(340), 340,
					rand()%nframes, 0, 0, 0);
				p->move(t);
				}
			}
		signed char frames[2];
		int dir = npc->get_direction(spot_on_table);
					// Reach out hand:
		frames[0] = npc->get_dir_framenum(dir, 1);
		frames[1] = npc->get_dir_framenum(dir, Actor::standing);
		npc->set_action(new Frames_actor_action(frames, 2));
		break;
		}
		}
	npc->start(gwin->get_std_delay(), delay);	// Back in queue.
	}

/*
 *	Notify that an object is no longer present.
 */
void Lab_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == chair) {
		chair = 0;
	} else if (obj == book) {
		book = 0;
	} else if (obj == cauldron) {
		cauldron = 0;
		state = start;
	} else {
	    for (vector<Game_object*>::iterator it = tables.begin();
											it != tables.end(); ++it) {
		    if (*it == obj) {
			    tables.erase(it);
				break;
			}
		}
	}
}

/*
 *	Schedule change for 'shy':
 */

void Shy_schedule::now_what
	(
	)
	{
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_tile(),
		   npcpos = npc->get_tile();
					// How far away is Avatar?
	int dist = npc->distance(av);
	if (dist > 10)			// Far enough?
		{			// Check again in a few seconds.
		if (rand()%3)		// Just wait.
			npc->start(250, 1000 + rand()%1000);
		else			// Sometimes wander.
			npc->walk_to_tile(
				Tile_coord(npcpos.tx + rand()%6 - 3,
					npcpos.ty + rand()%6 - 3, npcpos.tz));
		return;
		}
					// Get deltas.
	int dx = npcpos.tx - avpos.tx, dy = npcpos.ty - avpos.ty;
	int adx = dx < 0 ? -dx : dx;
	int ady = dy < 0 ? -dy : dy;
					// Which is farthest?
	int farthest = adx < ady ? ady : adx;
	int factor = farthest < 2 ? 9 : farthest < 4 ? 4 
				: farthest < 7 ? 2 : 1;
					// Walk away.
	Tile_coord dest = npcpos + Tile_coord(dx*factor, dy*factor, 0);
	Tile_coord delta = Tile_coord(rand()%3, rand()%3, 0);
	dest = dest + delta;
	Monster_pathfinder_client cost(npc, dest, 4);
	Actor_action *pact = Path_walking_actor_action::create_path(
							npcpos, dest, cost);
	if (pact)			// Found path?
		{
		npc->set_action(pact);
		npc->start(200, 100 + rand()%200);	// Start walking.
		}
	else					// Try again in a couple secs.
		npc->start(250, 500 + rand()%1000);
	}

/*
 *	Steal.
 */

void Thief_schedule::steal
	(
	Actor *from
	)
	{
	npc->say(first_thief, last_thief);
	int shnum = rand()%3;		// Gold coin, nugget, bar.
	Game_object *obj = 0;
	for (int i = 0; !obj && i < 3; ++i)
		{
		obj = from->find_item(644+shnum, c_any_qual, c_any_framenum);
		shnum = (shnum + 1)%3;
		}
	if (obj)
		{
		obj->remove_this(true);
		npc->add(obj, false);
		}
	}

/*
 *	Next decision for thief.
 */

void Thief_schedule::now_what
	(
	)
	{
	unsigned long curtime = SDL_GetTicks();
	if (curtime < next_steal_time)
		{			// Not time?  Wander.
		Tile_coord center = gwin->get_main_actor()->get_tile();
		const int dist = 6;
		int newx = center.tx - dist + rand()%(2*dist);
		int newy = center.ty - dist + rand()%(2*dist);
					// Wait a bit.
		npc->walk_to_tile(newx, newy, center.tz, 
				2*gwin->get_std_delay(), rand()%4000);
		return;
		}
	if (npc->distance(gwin->get_main_actor()) <= 1)
		{			// Next to Avatar.
		if (rand()%3)
			steal(gwin->get_main_actor());

		next_steal_time = curtime+8000+rand()%8000;
		npc->start(250, 1000 + rand()%2000);
		}
	else
		{			// Get within 1 tile of Avatar.
		Tile_coord dest = gwin->get_main_actor()->get_tile();
		Monster_pathfinder_client cost(npc, dest, 1);
		Actor_action *pact = Path_walking_actor_action::create_path(
						npc->get_tile(), dest, cost);
		if (pact)		// Found path?
			{
			npc->set_action(pact);
			npc->start(gwin->get_std_delay(), 1000 + rand()%1000);
			}
		else			// Try again in a couple secs.
			npc->start(250, 2000 + rand()%2000);
		}
	}

/*
 *	Create a 'waiter' schedule.
 */

Waiter_schedule::Waiter_schedule
	(
	Actor *n
	) : Schedule(n), startpos(n->get_tile()), customer(0), prep_table(0),
	    state(waiter_setup)
	{
	}

/*
 *	Find a new customer.
 */

bool Waiter_schedule::find_customer
	(
	)
{
	if (customers.empty())			// Got to search?
	{
		Actor_vector vec;		// Look within 32 tiles;
		npc->find_nearby_actors(vec, c_any_shapenum, 32);
		for (Actor_vector::const_iterator it = vec.begin();
							it != vec.end(); ++it)
		{		// Filter them.
			Actor *each = (Actor *) *it;
			if (each->get_schedule_type() == Schedule::eat_at_inn)
				customers.push_back(each);
		}
	}

	if (!customers.empty())
		{
		customer = customers.back();
		customers.pop_back();
		}
	return customer != 0;
}

/*
 *	Walk to prep. table.
 *	Output:	false if failed.
 */
bool Waiter_schedule::walk_to_prep
	(
	)
{
	if (prep_tables.size())	{	// Walk to a 'prep' table.
		prep_table = prep_tables[rand()%prep_tables.size()];
		add_client(prep_table);
		Tile_coord pos = Map_chunk::find_spot(
			prep_table->get_tile(), 1, npc);
		if (pos.tx != -1 &&
		    npc->walk_path_to_tile(pos, gwin->get_std_delay(), 
							1000 + rand()%1000))
			return true;
	} else
		prep_table = 0;
	const int dist = 8;		// Bad luck?  Walk randomly.
	int newx = startpos.tx - dist + rand()%(2*dist);
	int newy = startpos.ty - dist + rand()%(2*dist);
	npc->walk_to_tile(newx, newy, startpos.tz, 2*gwin->get_std_delay(), 
								rand()%2000);
	return false;
}

/*
 *	Walk to customer
 *	Output:	false if failed.
 */
bool Waiter_schedule::walk_to_customer
	(
	int min_delay			// Min. delay in msecs.
	)
	{
	if (customer)
		{
		if (customer->get_schedule_type() != Schedule::eat_at_inn)
			// Customer schedule changed. Tell schedule to refresh the list
			// (this happens with Hawk & others in SI).
			customers.clear();
		else
			{
			Tile_coord dest = Map_chunk::find_spot(customer->get_tile(),
									3, npc);
			if (dest.tx != -1 && npc->walk_path_to_tile(dest,
				gwin->get_std_delay(), min_delay + rand()%1000))
				return true;		// Walking there.
			}
		}
	npc->start(200, 2000 + rand()%4000);	// Failed so try again later.
	return false;
	}

/*
 *	Find tables and categorize them.
 */

void Waiter_schedule::find_tables
	(
	int shapenum
	)
	{
	Game_object_vector vec;
	npc->find_nearby(vec, shapenum, 32, 0);
	int floor = npc->get_lift()/5;	// Make sure it's on same floor.
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end();
								++it)
		{
		Game_object *table = *it;
		if (table->get_lift()/5 != floor)
			continue;
		Game_object_vector chairs;		// No chairs by it?
		if (!table->find_nearby(chairs, 873, 3, 0) &&
		    !table->find_nearby(chairs, 292, 3, 0))
			prep_tables.push_back(table);
		else
			eating_tables.push_back(table);
		add_client(table);
		}
	}

/*
 *	Find serving spot for a customer.
 *
 *	Output:	Plate if found, with spot set.  Plate is created if needed.
 */

Game_object *Waiter_schedule::find_serving_spot
	(
	Tile_coord& spot
	)
	{
	Game_object_vector plates;	// First look for a nearby plate.
	Game_object *plate = 0;
	Game_object_vector::const_iterator it;
	int cnt = npc->find_nearby(plates, 717, 1, 0);
	if (!cnt)
		cnt = npc->find_nearby(plates, 717, 2, 0);
	int floor = npc->get_lift()/5;	// Make sure it's on same floor.
	for (it = plates.begin(); it != plates.end(); ++it)
		{
		plate = *it;
		if (plate->get_lift()/5 == floor)
			{
			spot = plate->get_tile();
			spot.tz++;	// Just above plate.
			return plate;
			}
		}
	Tile_coord cpos = customer->get_tile();
	
		// Go through tables.
	for (it = eating_tables.begin(); it != eating_tables.end(); ++it)
		{
		Game_object *table = *it;
		Rectangle foot = table->get_footprint();
		if (foot.distance(cpos.tx, cpos.ty) > 2)
			continue;
					// Found it.
		spot = cpos;		// Start here.
					// East/West of table?
		if (cpos.ty >= foot.y && cpos.ty < foot.y + foot.h)
			spot.tx = cpos.tx <= foot.x ? foot.x
							: foot.x + foot.w - 1;
		else			// North/south.
			spot.ty = cpos.ty <= foot.y ? foot.y
						: foot.y + foot.h - 1;
		if (foot.has_world_point(spot.tx, spot.ty))
			{		// Passes test.
			Shape_info& info = table->get_info();
			spot.tz = table->get_lift() + info.get_3d_height();
			plate = gmap->create_ireg_object(717, 0);
			plate->move(spot);
			spot.tz++;	// Food goes above plate.
			return plate;
			}
		}
	return 0;			// Failed.
	}

/*
 *	Schedule change for 'waiter':
 */

void Waiter_schedule::now_what
	(
	)
	{
	Game_object *food;

	if (state == get_customer &&
	    rand() % 4 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
	if (state == get_order || state == serve_food) {
		int dist = customer ? npc->distance(customer) : 5000;
		if (dist > 32) {	// Need a new customer?
			state = get_customer;
			npc->start(200, 1000 + rand()%1000);
			return;
		}
					// Not close enough, so try again.
		if (dist >= 3 && !walk_to_customer()) {
			state = get_customer;
			return;
		}
	}
	switch (state) {
	case waiter_setup:
		find_tables(971);
		find_tables(633);
		find_tables(847);
		find_tables(1003);
		find_tables(1018);
		find_tables(890);
		find_tables(964);
		find_tables(333);
		state = get_customer;
		/* FALL THROUGH */
	case get_customer:
		if (!find_customer()) {
			walk_to_prep();
			state = prep_food;
		} else if (walk_to_customer())
			state = get_order;
		break;
	case get_order: {
		Game_object_vector foods;
					// Close enough to customer?
		if (customer->find_nearby(foods, 377, 2, 0) > 0) {
			if (rand()%4)
				npc->say(first_waiter_banter, 
							last_waiter_banter);
			state = get_customer;
			npc->start(200, 1000 + rand()%2000);
			break;
		}
					// Ask for order.
		npc->say(first_waiter_ask, last_waiter_ask);
		walk_to_prep();
		state = prep_food;
		break;
	}
	case prep_food:
		if (prep_table && npc->distance(prep_table) <= 3) {
			npc->change_frame(npc->get_dir_framenum(
				npc->get_facing_direction(prep_table),
							Actor::standing));
			Usecode_script *scr = new Usecode_script(npc);
			(*scr) << Ucscript::face_dir << npc->get_dir_facing();
			for (int cnt = 1 + rand()%3; cnt; --cnt) {
			  (*scr) << (Ucscript::npc_frame + Actor::ready_frame)
				 << Ucscript::delay_ticks << 1
				 << (Ucscript::npc_frame + Actor::raise1_frame)
				 << Ucscript::delay_ticks << 1;
			}
			(*scr) << (Ucscript::npc_frame + Actor::standing);
			scr->start();	// Start next tick.
		}
		if (!npc->get_readied(lhand)) {
					// Acquire some food.
			int nfoods = ShapeID(377, 0).get_num_frames();
			int frame = rand()%nfoods;
			food = new Ireg_game_object(377, frame, 0, 0, 0);
			npc->add_readied(food, lhand);
		}
		if (!walk_to_customer(3000)) {
			state = get_customer;
			if (rand()%3 == 0)
				gwin->get_usecode()->call_usecode(
					npc->get_usecode(), npc,
					Usecode_machine::npc_proximity);
		} else {
			state = serve_food;
		}
		break;
	case serve_food:
		food = npc->get_readied(lhand);
		Tile_coord spot;
		if (food && food->get_shapenum() == 377 &&
		    find_serving_spot(spot)) {
			npc->change_frame(npc->get_dir_framenum(
				npc->get_direction(customer),
							Actor::standing));
			npc->remove(food);
			food->set_invalid();
			food->move(spot);
			if (rand()%3)
				npc->say(first_waiter_serve,
					 last_waiter_serve);
			Usecode_script *scr = new Usecode_script(npc);
			(*scr) << Ucscript::face_dir << npc->get_dir_facing()
				<< (Ucscript::npc_frame + Actor::ready_frame)
				<< Ucscript::delay_ticks << 2
				<< (Ucscript::npc_frame + Actor::standing);
			scr->start();	// Start next tick.
		}
		state = get_customer;
		customer = 0;		// Done with this one.
		npc->start(250, 1000 + rand()%2000);
		return;
	}
}

/*
 *	Waiter schedule is done.
 */

void Waiter_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	remove_clients();
					// Remove what he/she is carrying.
	Game_object *obj = npc->get_readied(lhand);
	if (obj)
		obj->remove_this();
	obj = npc->get_readied(rhand);
	if (obj)
		obj->remove_this();
	}

/*
 *	Notify that an object is no longer present.
 */
void Waiter_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == prep_table) {
		prep_table = 0;
	}
	vector<Game_object*>::iterator it;
	for (it = prep_tables.begin(); it != prep_tables.end(); ++it) {
		if (*it == obj) {
			prep_tables.erase(it);
			break;
		}
	}
	for (it = eating_tables.begin(); it != eating_tables.end(); ++it) {
		if (*it == obj) {
			eating_tables.erase(it);
			break;
		}
	}
}

/*
 *	Sew/weave schedule.
 */

Sew_schedule::Sew_schedule
	(
	Actor *n
	) : Schedule(n), bale(0), spinwheel(0), chair(0), spindle(0), loom(0),
	    cloth(0), work_table(0), wares_table(0), sew_clothes_cnt(0),
	    state(get_wool)
	{
	}

/*
 *	Sew/weave.
 */

void Sew_schedule::now_what
	(
	)
	{
	Tile_coord npcpos = npc->get_tile();
					// Often want to get within 1 tile.
	Actor_pathfinder_client cost(npc, 1);
		// Pure guess. Since only some schedules seem to call proximity
		// usecode, I guess I should put it in here.
		// Seems quite rare.
	if (try_proximity_usecode(12))
		return;
	switch (state)
		{
	case get_wool:
		{
		if (spindle)		// Clean up any remainders.
			spindle->remove_this();
		if (cloth)
			cloth->remove_this();
		cloth = spindle = 0;
		npc->remove_quantity(2, 654, c_any_qual, c_any_framenum);
		npc->remove_quantity(2, 851, c_any_qual, c_any_framenum);

		bale = npc->find_closest(653);
		if (!bale)		// Just skip this step.
			{
			state = sit_at_wheel;
			break;
			}
		add_client(bale);
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, bale->get_tile(), cost);
		if (pact)
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(bale, 250),
				new Pickup_actor_action(bale,
					bale->get_tile(), 250)));
		state = sit_at_wheel;
		break;
		}
	case sit_at_wheel:
		chair = npc->find_closest(873);
		add_client(chair);
		if (!chair || !Sit_schedule::set_action(npc, chair, 200)) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			return;
		}
		state = spin_wool;
		break;
	case spin_wool:			// Cycle spinning wheel 8 times.
		spinwheel = npc->find_closest(651);
		if (!spinwheel) {
			// uh-oh... try again in a few seconds?
			npc->start(250, 2500);
			return;
		}
		add_client(spinwheel);
		npc->set_action(new Object_animate_actor_action(spinwheel,
								8, 200));
		state = get_thread;
		break;
	case get_thread:
		{
		Tile_coord t = Map_chunk::find_spot(
				spinwheel->get_tile(), 1, 654, 0);
		if (t.tx != -1)		// Space to create thread?
			{
			spindle = new Ireg_game_object(654, 0, 0, 0);
			add_client(spindle);
			spindle->move(t);
			gwin->add_dirty(spindle);
			npc->set_action(
				new Pickup_actor_action(spindle, 250));
			}
		state = weave_cloth;
		break;
		}
	case weave_cloth:
		{
		if (spindle)		// Should be held by NPC.
			spindle->remove_this();
		spindle = 0;
		loom = npc->find_closest(261);
		if (!loom)		// No loom found?
			{
			state = get_wool;
			break;
			}
		add_client(loom);
		Tile_coord lpos = loom->get_tile() +
						Tile_coord(-1, 0, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, lpos, cost);
		if (pact)
			npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(loom, 250),
				new Object_animate_actor_action(loom,
								4, 200)));
		state = get_cloth;
		break;
		}
	case get_cloth:
		{
		Tile_coord t = Map_chunk::find_spot(loom->get_tile(),
							1, 851, 0);
		if (t.tx != -1)		// Space to create it?
			{
			cloth = new Ireg_game_object(851, rand()%2, 0, 0);
			add_client(cloth);
			cloth->move(t);
			gwin->add_dirty(cloth);
			npc->set_action(
				new Pickup_actor_action(cloth, 250));
			}
		state = to_work_table;
		break;
		}
	case to_work_table:
		{
		work_table = npc->find_closest(971);
		add_client(work_table);
		if (!work_table || !cloth)
			{
			state = get_wool;
			break;
			}
		Tile_coord tpos = work_table->get_tile() +
						Tile_coord(1, -2, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
					// Find where to put cloth.
		Rectangle foot = work_table->get_footprint();
		Shape_info& info = work_table->get_info();
		Tile_coord cpos(foot.x + foot.w/2, foot.y + foot.h/2,
			work_table->get_lift() + info.get_3d_height());
		if (pact)
			npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(
						work_table, 250),
				new Pickup_actor_action(cloth, cpos, 250)));
		state = set_to_sew;
		break;
		}
	case set_to_sew:
		{
		Game_object *shears = npc->get_readied(lhand);
		if (shears && shears->get_shapenum() != 698)
			{		// Something's not right.
			shears->remove_this();
			shears = 0;
			}
		if (!shears)
			{		// Shears on table?
			Game_object_vector vec;
			if (npc->find_nearby(vec, 698, 3, 0))
				{
				shears = vec[0];
				gwin->add_dirty(shears);
				shears->remove_this(1);
				}
			else
				shears = new Ireg_game_object(698, 0, 0, 0);
			npc->add_readied(shears, lhand);
			}
		state = sew_clothes;
		sew_clothes_cnt = 0;
		break;
		}
	case sew_clothes:
		{
		int dir = npc->get_direction(cloth);
		signed char frames[5];
		int nframes = npc->get_attack_frames(698, false, 
								dir, frames);
		if (nframes)
			npc->set_action(new Frames_actor_action(frames, nframes));
		sew_clothes_cnt++;
		if (sew_clothes_cnt > 1 && sew_clothes_cnt < 5)
			{
			int num_cloth_frames = ShapeID(851, 0).get_num_frames();
			cloth->change_frame(rand()%num_cloth_frames);
			}
		else if (sew_clothes_cnt == 5)
			{
			gwin->add_dirty(cloth);
			Tile_coord pos = cloth->get_tile();
			cloth->remove_this(1);
					// Top or pants.
			int shnum = GAME_SI ? 403 : (rand()%2 ? 738 : 249);
			cloth->set_shape(shnum);
			int nframes = ShapeID(shnum, 0).get_num_frames();
			cloth->set_frame(rand()%nframes);
			cloth->move(pos);
			state = get_clothes;
			}
		break;
		}
	case get_clothes:
		{
		Game_object *shears = npc->get_readied(lhand);
		if (shears) {
			Tile_coord pos = cloth->get_tile();
			npc->set_action(new Sequence_actor_action(
								  new Pickup_actor_action(cloth, 250),
								  new Pickup_actor_action(shears, pos, 250)));
		} else {
			// ++++ maybe create shears? anyway, leaving this till after
			// possible/probable schedule system rewrite
			npc->set_action(new Pickup_actor_action(cloth, 250));
		}
		state = display_clothes;
		break;
		}
	case display_clothes:
		{
		state = done;
		wares_table = npc->find_closest(890);
		add_client(wares_table);
		if (!wares_table)
			{
			cloth->remove_this();
			cloth = 0;
			break;
			}
		Tile_coord tpos = wares_table->get_tile() +
						Tile_coord(1, -2, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
					// Find where to put cloth.
		Rectangle foot = wares_table->get_footprint();
		Shape_info& info = wares_table->get_info();
		Tile_coord cpos(foot.x + rand()%foot.w, foot.y + rand()%foot.h,
			wares_table->get_lift() + info.get_3d_height());
		if (pact)
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(cloth, cpos, 250, true)));
		cloth = 0;			// Leave it be.
		break;
		}
	case done:				// Just put down clothing.
		{
		state = get_wool;
		Game_object_vector vec;		// Don't create too many.
		int cnt = 0;
		if (GAME_SI)
			cnt += npc->find_nearby(vec, 403, 5, 0);
		else				// BG shapes.
			{
			cnt += npc->find_nearby(vec, 738, 5, 0);
			cnt += npc->find_nearby(vec, 249, 5, 0);
			}
		if (cnt >= 3)
			{
			Game_object *obj = vec[rand()%cnt];
			gwin->add_dirty(obj);
			obj->remove_this();
			}
		break;
		}
	default:			// Back to start.
		state = get_wool;
		break;
		}
	npc->start(250, 100);		// Back in queue.
	}

/*
 *	Sewing schedule is done.
 */

void Sew_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	remove_clients();
					// Remove shears.
	Game_object *obj = npc->get_readied(lhand);
	if (obj)
		obj->remove_this();
	if (cloth)			// Don't leave cloth lying around.
		{
		if (!cloth->get_owner())
			gwin->add_dirty(cloth);
		cloth->remove_this();
		cloth = 0;
		}
	}

/*
 *	Notify that an object is no longer present.
 */
void Sew_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == bale) {
		bale = 0;
	} else if (obj == spinwheel) {
	    spinwheel = 0;
	} else if (obj == chair) {
	    chair = 0;
	} else if (obj == spindle) {
	    spindle = 0;
	} else if (obj == loom) {
	    loom = 0;
	} else if (obj == cloth) {
	    cloth = 0;
	} else if (obj == work_table) {
	    work_table = 0;
	} else if (obj == wares_table) {
	    wares_table = 0;
	}
}

/*
 *	Bake bread/pastries
 *
 *	Dough gets set to quality 50 and dough placed in oven is quality 51
 */

Bake_schedule::Bake_schedule(Actor *n) : Schedule(n),
	oven(0), worktable(0), displaytable(0), flourbag(0),
	dough(0), dough_in_oven(0), state(find_leftovers)
	{ }

void Bake_schedule::now_what()
{
	Tile_coord npcpos = npc->get_tile();
	Actor_pathfinder_client cost(npc, 1);
	Actor_pathfinder_client cost2(npc, 2);
	int delay = 100;
	int dough_shp (GAME_SI ? 863 : 658);
	Game_object *stove = npc->find_closest(664);
		// Pure guess. Since only some schedules seem to call proximity
		// usecode, I guess I should put it in here.
		// Seems quite common.
	if (try_proximity_usecode(8))
		return;

	switch (state) {
	case find_leftovers:	// Look for misplaced dough already made by this schedule
	{
		state = to_flour;
		if (!dough_in_oven)
		{
			// look for baking dough
			Game_object_vector baking_dough;
			int frnum (GAME_SI ? 18 : 2);
			npc->find_nearby(baking_dough, npcpos, dough_shp, 20, 0, 51, frnum, false);
			if (!baking_dough.empty() && baking_dough[0] != dough)	// found dough
			{
				dough_in_oven = baking_dough[0];
				dough_in_oven->clear_flag(Obj_flags::okay_to_take); // doesn't save okay_to_take
				add_client(dough_in_oven);
				state = remove_from_oven;
				break;
			}
			// looking for cooked food left in oven
			oven = npc->find_closest(831);
			if (!oven)
				oven = stove;
			if (oven)
			{
				add_client(oven);
				Game_object_vector food;
				Tile_coord Opos = oven->get_tile();
				npc->find_nearby(food, Opos, 377, 2, 0, c_any_qual, c_any_framenum, true);
				if (!food.empty())	// found food
				{
					dough_in_oven = food[0];
					dough_in_oven->clear_flag(Obj_flags::okay_to_take); // doesn't save okay_to_take
					add_client(dough_in_oven);
					state = remove_from_oven;
					break;
				}
			}
		}
		if (!dough)		// Looking for unused dough on tables
		{
			Game_object_vector leftovers;
			if (GAME_SI)
			{
				npc->find_nearby(leftovers, npcpos, dough_shp, 20, 0, 50, 16, false);
				npc->find_nearby(leftovers, npcpos, dough_shp, 20, 0, 50, 17, false);
				npc->find_nearby(leftovers, npcpos, dough_shp, 20, 0, 50, 18, false);
			}
			else
				npc->find_nearby(leftovers, npcpos, dough_shp, 20, 0, 50, c_any_framenum, false);
			if (!leftovers.empty() && leftovers[0] != dough_in_oven)	// found dough
			{
				dough = leftovers[0];
				dough->clear_flag(Obj_flags::okay_to_take); // doesn't save okay_to_take
				add_client(dough);
				state = make_dough;
				delay = 0;
				Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, dough->get_tile(), cost2);
				if (pact)	// walk to dough if we can
					npc->set_action(pact);
			}
		}
		break;
	}
	case to_flour:		// Looks for flourbag and walks to it if found
	{
		Game_object_vector items;
		npc->find_nearby(items, npcpos, 863, -1, 0, c_any_qual, 0);
		npc->find_nearby(items, npcpos, 863, -1, 0, c_any_qual, 13);
		npc->find_nearby(items, npcpos, 863, -1, 0, c_any_qual, 14);

		if (items.empty()) {
			state = to_table;
			break;
		}

		int nr = rand()%items.size();
		flourbag = items[nr];
		add_client(flourbag);

		Tile_coord tpos = flourbag->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
		if (pact) {
			npc->set_action(pact);
		} else {
			// just ignore it
			state = to_table;
			break;
		}

		state = get_flour;
		break;
	}
	case get_flour:		// Bend over flourbag and change the frame to zero if nonzero
	{
		if (!flourbag) {
			// what are we doing here then? back to start
			state = to_flour;
			break;
		}

		int dir = npc->get_direction(flourbag);
		npc->change_frame(
			npc->get_dir_framenum(dir,Actor::bow_frame));

		if (flourbag->get_framenum() != 0)
			flourbag->change_frame(0);

		delay = 750;
		state = to_table;
		break;
	}
	case to_table:		// Walk over to worktable and create flour
	{
		Game_object *table1 = npc->find_closest(1003);
		Game_object *table2 = npc->find_closest(1018);
		if (stove)
		{
			Game_object_vector table;
			npc->find_nearby(table, npcpos, 890, -1, 0, c_any_qual, 5);
			if (table.size() == 1)
				worktable = table[0];
			else if (table.size() > 1)
			{
				if (rand()%2)
					worktable = table[0];
				else
					worktable = table[1];
			}
		}
		else if (!table1)
			worktable = table2;
		else if (!table2)
			worktable = table1;
		else if (table1->distance(npc) < table2->distance(npc))
			worktable = table1;
		else
			worktable = table2;

		if (!worktable)
			worktable = npc->find_closest(1018);
		if (!worktable) {
			// problem... try again in a few seconds
			delay = 2500;
			state = to_flour;
			break;
		}
		add_client(worktable);
					// Find where to put dough.
		Rectangle foot = worktable->get_footprint();
		Shape_info& info = worktable->get_info();
		Tile_coord cpos(foot.x + rand()%foot.w, foot.y + rand()%foot.h,
			worktable->get_lift() + info.get_3d_height());
		Tile_coord tablepos = cpos;
		cpos.tz = 0;

		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, cpos, cost);
		if (pact) {
			if (dough) {
				dough->remove_this();
				dough = 0;
			}
			if (Game::get_game_type() == SERPENT_ISLE)
				dough = new Ireg_game_object(dough_shp, 16, 0, 0);
			else
				dough = new Ireg_game_object(dough_shp, 0, 0, 0);
			add_client(dough);
			dough->set_quality(50); // doesn't save okay_to_take
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(dough,tablepos,250)));
		} else {
			// not good... try again
			delay = 2500;
			state = to_flour;
			break;
		}

		state = make_dough;
		break;
	}
	case make_dough:	// Changes flour to flat dough then dough ball
	{
		if (!dough) {
			// better try again...
			delay = 2500;
			state = to_table;
			break;
		}
		dough->clear_flag(Obj_flags::okay_to_take); // doesn't save okay_to_take

		int dir = npc->get_direction(dough);
		signed char fr[2];
		fr[0] = npc->get_dir_framenum(dir, 3);
		fr[1] = npc->get_dir_framenum(dir, 0);

		int frame = dough->get_framenum();
		if (GAME_SI ? frame == 16: frame == 0)
			npc->set_action(new Sequence_actor_action(
				new Frames_actor_action(fr, 2, 500),
			((GAME_SI) ?
				new Frames_actor_action(0x11,250,dough) :
				new Frames_actor_action(0x01,250,dough)),
				new Frames_actor_action(fr, 2, 500),
			((GAME_SI) ?
				new Frames_actor_action(0x12,250,dough) :
				new Frames_actor_action(0x02,250,dough))
			));
		else if (GAME_SI ? frame == 17: frame == 1)
			npc->set_action(new Sequence_actor_action(
				new Frames_actor_action(fr, 2, 500),
				((GAME_SI) ?
				new Frames_actor_action(0x12,250,dough) :
				new Frames_actor_action(0x02,250,dough))
				));
		
		state = remove_from_oven;
		break;
	}
	case remove_from_oven:	// Changes dough in oven to food %7 and picks it up
	{
		if (!dough_in_oven) {
			// nothing in oven yet
			state = get_dough;
			break;
		}
		if (stove)
			oven = stove;
		else
			oven = npc->find_closest(831);
		if (!oven) {
			// this really shouldn't happen...
			dough_in_oven->remove_this();
			dough_in_oven = 0;

			delay = 2500;
			state = to_table;
			break;
		}
		add_client(oven);
		if (dough_in_oven->get_shapenum() != 377){
			gwin->add_dirty(dough_in_oven);
			dough_in_oven->set_shape(377);
			dough_in_oven->set_frame(rand()%7);
			dough_in_oven->clear_flag(Obj_flags::okay_to_take);  // doesn't save okay_to_take
			gwin->add_dirty(dough_in_oven);
		}

		Tile_coord tpos = oven->get_tile() + 
						Tile_coord(1, 1, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
		if (pact) {
			npc->set_action(new Sequence_actor_action(
				pact,
				new Pickup_actor_action(dough_in_oven, 250)));
		} else {
			// just pick it up
			npc->set_action(
				new Pickup_actor_action(dough_in_oven, 250));
		}

		state = display_wares;
		break;
	}
	case display_wares:		// Walk to displaytable. Put food on it. If table full, go to
	{						// clear_display which eventualy comes back here to place food
		if (!dough_in_oven) {
			// try again
			delay = 2500;
			state = find_leftovers;
			break;
		}
		displaytable = npc->find_closest(633); // Britain
		if (!displaytable)
		{
			Game_object_vector table;
			int table_shp = 890; // Moonshade table
			int table_frm = 1;
			if (stove)
			{
				table_shp = 1003;
				table_frm = 2;
			}
			npc->find_nearby(table, npcpos, table_shp, -1, 0, c_any_qual, table_frm);
			if (table.size() == 1)
				displaytable = table[0];
			else if (table.size() > 1)
			{
				if (rand()%2)
					displaytable = table[0];
				else
					displaytable = table[1];
			}
		}
		if (!displaytable) {
			// uh-oh...
			dough_in_oven->remove_this();
			dough_in_oven = 0;

			delay = 2500;
			state = find_leftovers;
			break;
		}
		add_client(displaytable);

		Rectangle r = displaytable->get_footprint();
		Perimeter p(r);		// Find spot adjacent to table.
		Tile_coord spot;	// Also get closest spot on table.
		Tile_coord spot_on_table;
		p.get(rand()%p.size(), spot, spot_on_table);
		Actor_pathfinder_client COST = (GAME_SI ? cost : cost2);
		Actor_action *pact = Path_walking_actor_action::create_path(
							npcpos, spot, COST);
		Shape_info& info = displaytable->get_info();
		spot_on_table.tz += info.get_3d_height();

		// Place baked goods if spot is empty.
		Tile_coord t = Map_chunk::find_spot(spot_on_table, 0,
						377, 0, 0);
		if (t.tx != -1 && t.tz == spot_on_table.tz)
		{
			npc->set_action(new Sequence_actor_action(pact,
					new Pickup_actor_action(dough_in_oven,
							 spot_on_table, 250)));
			dough_in_oven = 0;
			state = get_dough;
		}
		else
		{
			Actor_action *pact = Path_walking_actor_action::create_path(
							npcpos, displaytable->get_tile(), cost);
			npc->set_action(new Sequence_actor_action(pact,
				new Face_pos_actor_action(displaytable->get_tile(), 250)));
			delay = 250;
			state = clear_display;
		}		
		clearing = false;
		break;
	}
	case clear_display:		// Mark food for deletion by remove_food
	{
		Game_object_vector food;
		npc->find_nearby(food, npcpos, 377, 4, 0, c_any_qual, c_any_framenum, true);
		if (!food.size() && !clearing) // none of our food on the table
		{							   // so we can't clear it
			if (dough_in_oven)
				dough_in_oven->remove_this();
			dough_in_oven = 0;
			state = get_dough;
			break;
		}
		clearing = true;
		for (size_t i = 0; i < food.size(); i++)
		{
			delay = 500;
			state = remove_food;
			break;
		}
		if (!food.size())
			state = display_wares;
		break;
	}
	case remove_food:	// Delete food on display table one by one with a slight delay
	{
		Game_object_vector food;
		npc->find_nearby(food, npcpos, 377, 4, 0, c_any_qual, c_any_framenum, true);
		if (food.size())
		{
			delay = 500;
			state = clear_display;
			gwin->add_dirty(food[0]);
			food[0]->remove_this();
		}
		break;
	}
	case get_dough:		// Walk to work table and pick up dough
	{
		if (!dough) {
			// try again
			delay = 2500;
			state = find_leftovers;
			break;
		}
		if (stove)
			oven = stove;
		else
			oven = npc->find_closest(831);
		if (!oven) {
			// wait a while
			delay = 2500;
			state = find_leftovers;
			break;
		}
		add_client(oven);
		Tile_coord tpos = dough->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost2);
		if (pact) {
			npc->set_action(new Sequence_actor_action(
				pact,
				new Pickup_actor_action(dough, 250)));
		} else {
			// just pick it up
			npc->set_action(new Pickup_actor_action(dough, 250));
		}

		state = put_in_oven;
		break;
	}
	case put_in_oven:	// Walk to oven and put dough on in.
	{
		if (!dough) {
			// try again
			delay = 2500;
			state = find_leftovers;
			break;
		}
		if (stove)
			oven = stove;
		else
			oven = npc->find_closest(831);
		if (!oven) {
			// oops... retry
			dough->remove_this();
			dough = 0;

			delay = 2500;
			state = to_table;
			break;
		}
		add_client(oven);
		Tile_coord tpos = oven->get_tile() + 
						Tile_coord(1, 1, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);

		// offsets for oven placement
		int offX = +1, offY = 0, offZ = 0;
		if (stove) // hide dough
			offX = -3, offY = 0, offZ = -2;

		Rectangle foot = oven->get_footprint();
		Shape_info& info = oven->get_info();
		Tile_coord cpos(foot.x + offX, foot.y + offY, 
				oven->get_lift() + info.get_3d_height() + offZ);

		if (pact) {
			npc->set_action(new Sequence_actor_action(
				pact,
				new Pickup_actor_action(dough, cpos, 250)));

			dough->set_quality(51); // doesn't save okay_to_take
			dough_in_oven = dough;
			dough = 0;
		} else {
			dough->remove_this();
			dough = 0;
		}

		state = find_leftovers;
		break;
	}
	}
	npc->start(250, delay);		// Back in queue.
}

/*
 *	TODO: Eventually, this should be used to finish baking all
 *	leftovers which is basically duplicating the regular schedule
 *	minus to_flour, get_flour, and any future added kitchen items
 *	usage. Special checks for when to just give up should be added.
 */

void Bake_schedule::ending(int new_type)
{
	remove_clients();
	if (dough) {
		dough->remove_this();
		dough = 0;
	}
	if (dough_in_oven) {
		dough_in_oven->remove_this();
		dough_in_oven = 0;
	}
}

/*
 *	Notify that an object is no longer present.
 */
void Bake_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == oven)
	    oven = 0;
	else if (obj == worktable)
		worktable = 0;
	else if (obj == displaytable)
		displaytable = 0;
	else if (obj == flourbag)
		flourbag = 0;
	else if (obj == dough)		// Someone stole the dough!
		dough = 0;
	else if (obj == dough_in_oven)
		dough_in_oven = 0;
}

/*
 *	Blacksmith.
 *
 * Note: the original kept the tongs & hammer, and put them on a nearby table
 */

Forge_schedule::Forge_schedule(Actor *n) : Schedule(n), tongs(0), hammer(0),
	blank(0), firepit(0), anvil(0), trough(0), bellows(0), 
	state(put_sword_on_firepit)
	{ }

void Forge_schedule::now_what
	(
	)
	{
	Tile_coord npcpos = npc->get_tile();
					// Often want to get within 1 tile.
	Actor_pathfinder_client cost(npc, 1);

	switch (state) {
	case put_sword_on_firepit:
	{
		if (!blank) {
			blank = npc->find_closest(668);
			//TODO: go and get it...

		}
		if (!blank)
			blank = new Ireg_game_object(668, 0, 0, 0);
		add_client(blank);
		firepit = npc->find_closest(739);
		if (!firepit) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			return;
		}
		add_client(firepit);
		Tile_coord tpos = firepit->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, tpos, cost);

		Rectangle foot = firepit->get_footprint();
		Shape_info& info = firepit->get_info();
		Tile_coord bpos(foot.x + foot.w/2 + 1, foot.y + foot.h/2,
			firepit->get_lift() + info.get_3d_height());
		if (pact) {
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(blank, bpos, 250)));
		} else {
			npc->set_action(
				new Pickup_actor_action(blank, bpos, 250));
		}

		state = use_bellows;
		break;
	}
	case use_bellows:
	{
		bellows = npc->find_closest(431);
		firepit = npc->find_closest(739);
		add_client(bellows);
		add_client(firepit);
		if (!bellows || !firepit || !blank) {
			// uh-oh... try again in a few second
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}

		Tile_coord tpos = bellows->get_tile() +
					Tile_coord(3,0,0);
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, tpos, cost);

		Actor_action **a = new Actor_action*[35];
		a[0] = pact;
		a[1] = new Face_pos_actor_action(bellows, 250);
		a[2] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[3] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[4] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[5] = new Frames_actor_action(0x01, 0, firepit);
		a[6] = new Frames_actor_action(0x01, 0, blank);
		a[7] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[8] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[9] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[10] = new Frames_actor_action(0x02, 0, blank);
		a[11] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[12] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[13] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[14] = new Frames_actor_action(0x02, 0, firepit);
		a[15] = new Frames_actor_action(0x03, 0, blank);
		a[16] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[17] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[18] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[19] = new Frames_actor_action(0x03, 0, firepit);
		a[20] = new Frames_actor_action(0x04, 0, blank);
		a[21] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[22] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[23] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[24] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[25] =	new Object_animate_actor_action(bellows, 3, 1, 300);
		a[26] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[27] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[28] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[29] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[30] = new Frames_actor_action(0x20|Actor::bow_frame, 0);
		a[31] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[32] = new Frames_actor_action(0x20|Actor::standing, 0);
		a[33] = new Frames_actor_action(0x00, 0, bellows);
		a[34] = 0;


		npc->set_action(new Sequence_actor_action(a));
		state = get_tongs;
		break;
	}
	case get_tongs:
	{
#if 0
		if (!tongs) {
			tongs = npc->find_closest(994);
			//TODO: go and get it...
		}
#endif
		if (!tongs) {
			tongs = new Ireg_game_object(994, 0, 0, 0);
			add_client(tongs);
		}
		npc->add_dirty();
		npc->unready_weapon(); // make sure the tongs can be equipped
		npc->add_readied(tongs, lhand);
		npc->add_dirty();

		state = sword_on_anvil;
		break;
	}
	case sword_on_anvil:
	{
		anvil = npc->find_closest(991);
		firepit = npc->find_closest(739);
		add_client(anvil);
		add_client(firepit);
		if (!anvil || !firepit || !blank) {
			// uh-oh... try again in a few second
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}

		Tile_coord tpos = firepit->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, tpos, cost);
		Tile_coord tpos2 = anvil->get_tile() + 
					Tile_coord(0,1,0);
		Actor_action *pact2 = Path_walking_actor_action::create_path(
				tpos, tpos2, cost);

		Rectangle foot = anvil->get_footprint();
		Shape_info& info = anvil->get_info();
		Tile_coord bpos(foot.x + 2, foot.y,
			anvil->get_lift() + info.get_3d_height());
		if (pact && pact2) {
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(blank, 250),
				pact2,
				new Pickup_actor_action(blank, bpos, 250)));
		} else {
			npc->set_action(new Sequence_actor_action(
				new Pickup_actor_action(blank, 250),
				new Pickup_actor_action(blank, bpos, 250)));
		}
		state = get_hammer;
		break;
	}
	case get_hammer:
	{
#if 0
		if (!hammer) {
			hammer = npc->find_closest(623);
			//TODO: go and get it...
		}
#endif

		if (!hammer) {
			hammer = new Ireg_game_object(623, 0, 0, 0);
			add_client(hammer);
		}
		npc->add_dirty();
		if (tongs) {
			tongs->remove_this();
			tongs = 0;
		}
		npc->unready_weapon(); // make sure the hammer can be equipped
		npc->add_readied(hammer, lhand);
		npc->add_dirty();

		state = use_hammer;
		break;
	}
	case use_hammer:
	{
		anvil = npc->find_closest(991);
		firepit = npc->find_closest(739);
		add_client(anvil);
		add_client(firepit);
		if (!anvil || !firepit || !blank) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}

		signed char frames[12];
		int cnt = npc->get_attack_frames(623, false, 0, frames);
		if (cnt)
			npc->set_action(new Frames_actor_action(frames, cnt));
		
		Actor_action **a = new Actor_action*[10];
		a[0] = new Frames_actor_action(frames, cnt);
		a[1] = new Frames_actor_action(0x03, 0, blank);
		a[2] = new Frames_actor_action(0x02, 0, firepit);
		a[3] = new Frames_actor_action(frames, cnt);
		a[4] = new Frames_actor_action(0x02, 0, blank);
		a[5] = new Frames_actor_action(0x01, 0, firepit);
		a[6] = new Frames_actor_action(frames, cnt);
		a[7] = new Frames_actor_action(0x01, 0, blank);
		a[8] = new Frames_actor_action(0x00, 0, firepit);
		a[9] = 0;
		npc->set_action(new Sequence_actor_action(a));

		state = walk_to_trough;
		break;
	}
	case walk_to_trough:
	{
		npc->add_dirty();
		if (hammer) {
			hammer->remove_this();
			hammer = 0;
		}
		npc->add_dirty();

		trough = npc->find_closest(719);
		add_client(trough);
		if (!trough) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}

		if (trough->get_framenum() == 0) {
			Tile_coord tpos = trough->get_tile() +
						Tile_coord(0,2,0);
			Actor_action *pact = Path_walking_actor_action::
					create_path(npcpos, tpos, cost);
			npc->set_action(pact);
			state = fill_trough;
			break;
		}
		
		state = get_tongs2;
		break;
	}
	case fill_trough:
	{
		trough = npc->find_closest(719);
		if (!trough) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}
		add_client(trough);
		int dir = npc->get_direction(trough);
		trough->change_frame(3);
		npc->change_frame(
			npc->get_dir_framenum(dir, Actor::bow_frame));

		state = get_tongs2;
		break;
	}
	case get_tongs2:
	{
#if 0
		if (!tongs) {
			tongs = npc->find_closest(994);
			//TODO: go and get it...
		}
#endif
		if (!tongs) {
			tongs = new Ireg_game_object(994, 0, 0, 0);
			add_client(tongs);
		}
		npc->add_dirty();
		npc->unready_weapon(); // make sure the tongs can be equipped
		npc->add_readied(tongs, lhand);
		npc->add_dirty();

		state = use_trough;
		break;
	}
	case use_trough:
	{
		trough = npc->find_closest(719);
		anvil = npc->find_closest(991);
		add_client(trough);
		add_client(anvil);
		if (!trough || !anvil || !blank) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}
		
		Tile_coord tpos = anvil->get_tile() +
				Tile_coord(0,1,0);
		Actor_action *pact = Path_walking_actor_action::
				create_path(npcpos, tpos, cost);

		Tile_coord tpos2 = trough->get_tile() +
				Tile_coord(0,2,0);
		Actor_action *pact2 = Path_walking_actor_action::
				create_path(tpos, tpos2, cost);

		if (pact && pact2) {
			signed char troughframe = trough->get_framenum() - 1;
			if (troughframe < 0) troughframe = 0;

			int dir = npc->get_direction(trough);
			signed char npcframe = npc->get_dir_framenum(dir, Actor::bow_frame);

			Actor_action **a = new Actor_action*[7];
			a[0] = pact;
			a[1] = new Pickup_actor_action(blank, 250);
			a[2] = pact2;
			a[3] = new Frames_actor_action(&npcframe, 1, 250);
			a[4] = new Frames_actor_action(&troughframe, 1, 0, trough);
			a[5] = new Frames_actor_action(0x00, 0, blank);
			a[6] = 0;
			npc->set_action(new Sequence_actor_action(a));
		} else {
			// no path found, just pick up sword blank
			npc->set_action(new Sequence_actor_action(
				new Pickup_actor_action(blank, 250),
				new Frames_actor_action(0, 0, blank)));
		}	

		state = done;
		break;
	}
	case done:
	{
		npc->add_dirty();
		if (tongs) {
			tongs->remove_this();
			tongs = 0;
		}
		npc->add_dirty();
		state = put_sword_on_firepit;
	}
	}
	npc->start(250, 100);		// Back in queue.
	}

/*
 *	Forge schedule is done.
 */

void Forge_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
	remove_clients();
					// Remove any tools.
	if (tongs) {
		tongs->remove_this();
		tongs = 0;
	}
	if (hammer) {
		hammer->remove_this();
		hammer = 0;
	}

	firepit = npc->find_closest(739);
	bellows = npc->find_closest(431);

	if (firepit && firepit->get_framenum() != 0)
		firepit->change_frame(0);
	if (bellows && bellows->get_framenum() != 0)
		bellows->change_frame(0);
	if (blank && blank->get_framenum() != 0)
		blank->change_frame(0);

	}

/*
 *	Notify that an object is no longer present.
 */
void Forge_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == tongs)
	    tongs = 0;
	else if (obj == hammer)
		hammer = 0;
	else if (obj == blank)
		blank = 0;
	else if (obj == firepit)
		firepit = 0;
	else if (obj == anvil)
		anvil = 0;
	else if (obj == trough)
		trough = 0;
	else if (obj == bellows)
		bellows = 0;
}

/*
 *	Eat without a server
 *	The original seems to pathfind to a plate, then place the food on
 *	the plate, sit down, and then eat it if the food is close enough.
 *	They only seemed to eat one food item and then sit there doing nothing.
 *	If there is no plate, they will wander around in a loiter like
 *	schedule saying 0x410-0x413 randomly until a plate becomes available.
 *	When eating, there are no barks.
 *	TODO: Add plate pathfinding
 */
Eat_schedule::Eat_schedule(Actor *n): Schedule(n), plate(0),
	state(find_plate){}

/*
 *  Just went dormant.
 */
void Eat_schedule::im_dormant()
{
	// Force NPCs to sit down again after cache-out/cache-in.
	//npc->set_frame(npc->get_dir_framenum(Actor::standing));
	npc->set_action(0);
}

void Eat_schedule::now_what()
{
	int delay = 5000 + rand()%12000;
	int frnum = npc->get_framenum();
	if ((frnum&0xf) != Actor::sit_frame){
		if (!Sit_schedule::set_action(npc)) // First have to sit down.
			npc->start(250, 5000); // Try again in a while.
		return;
	}
	switch(state){
	case eat:{
		Game_object_vector foods;			// Food nearby?
		int cnt = npc->find_nearby(foods, 377, 2, 0);
		if (cnt){			// Found?
			Game_object *food = 0; // Find closest.
			int dist = 500;
			for (Game_object_vector::const_iterator it = foods.begin();
						it != foods.end(); ++it){
				Game_object *obj = *it;
				int odist = obj->distance(npc);
				if (odist < dist){
					dist = odist;
					food = obj;
				}
			}
			if (rand()%5 == 0 && food) {
				gwin->add_dirty(food);
				food->remove_this();
			}
		}		// loops back to itself since npc can be pushed 
		break;	// out of their chair and not eat right away
	}
	case find_plate:{
		state = serve_food;
		delay = 0;
		plate = 0; // make sure moved plate doesn't get food sent to it
		Game_object_vector plates;	// First look for a nearby plate.
		npc->find_nearby(plates, 717, 1, 0);
		Game_object_vector::const_iterator it;
		int floor = npc->get_lift()/5;	// Make sure it's on same floor.
		for (it = plates.begin(); it != plates.end(); ++it){
			plate = *it;
			if (plate->get_lift()/5 == floor)
				break;
		}
		add_client(plate);
		break;
	}
	case serve_food:{
		state = eat;
		if (!plate){
			state = find_plate;
			break;
		}
		Tile_coord t = plate->get_tile();
		Game_object *food = gmap->create_ireg_object(377, rand()%31);
		food->move(t.tx, t.ty, t.tz +1);
		break;
	}
	}
	npc->start(250, delay);
}

// TODO: This should be in a loop to remove food one at a time with a delay
void Eat_schedule::ending (int new_type) // new schedule type
{
	remove_clients();
	Game_object_vector foods;			// Food nearby?
	int cnt = npc->find_nearby(foods, 377, 2, 0);
	if (cnt){			// Found?
		for (Game_object_vector::const_iterator it = foods.begin();
					it != foods.end(); ++it){
			Game_object *food = *it;
			if (food){
				gwin->add_dirty(food);
				food->remove_this();
			}
		}
	}
}

/*
 *	Notify that an object is no longer present.
 */
void Eat_schedule::notify_object_gone(Game_object *obj)
{
	if (obj == plate)		// Someone stole the plate!
		plate = 0;
}

/*
 *	Modify goal to walk off the screen.
 */

void Walk_to_schedule::walk_off_screen
	(
	Rectangle& screen,		// In tiles, area slightly larger than
					//   actual screen.
	Tile_coord& goal		// Modified for path offscreen.
	)
	{
					// Destination.
	if (goal.tx >= screen.x + screen.w)
		{
		goal.tx = screen.x + screen.w - 1;
		goal.ty = -1;
		}
	else if (goal.tx < screen.x)
		{
		goal.tx = screen.x;
		goal.ty = -1;
		}
	else if (goal.ty >= screen.y + screen.h)
		{
		goal.ty = screen.y + screen.h - 1;
		goal.tx = -1;
		}
	else if (goal.ty < screen.y)
		{
		goal.ty = screen.y;
		goal.tx = -1;
		}
	}

/*
 *	Create schedule for walking to the next schedule's destination.
 */

Walk_to_schedule::Walk_to_schedule
	(
	Actor *n,
	Tile_coord const& d,			// Destination.
	int new_sched,			// Schedule when we get there.
	int delay			// Msecs, or -1 for random delay.
	) : Schedule(n), dest(d), new_schedule(new_sched), retries(0), legs(0)
	{
					// Delay 0-20 secs.
	first_delay = delay >= 0 ? delay : 2*(rand()%10000);
	}

/*
 *	Schedule change for walking to another schedule destination.
 */

void Walk_to_schedule::now_what
	(
	)
	{
	if (npc->distance(dest) <= 3)
		{			// Close enough!
		npc->set_schedule_type(new_schedule);
		return;
		}
	if (legs >= 40 || retries >= 2)	// Trying too hard?  (Following
					//   Patterson takes about 30.)
		{			// Going to jump there.
		npc->move(dest.tx, dest.ty, dest.tz);
					// Force actor to sit down/lie down again
		npc->set_frame(npc->get_dir_framenum(Actor::standing));
		npc->set_schedule_type(new_schedule);
		return;
		}
					// Get screen rect. in tiles.
	Rectangle screen = gwin->get_win_tile_rect();
	screen.enlarge(6);		// Enlarge in all dirs.
					// Might do part of it first.
	Tile_coord from = npc->get_tile(),
		   to = dest;
					// Destination off the screen?
	if (!screen.has_world_point(to.tx, to.ty))
		{
		if (!screen.has_world_point(from.tx, from.ty))
			{		// Force teleport on next tick.
			retries = 100;
			npc->start(200, 100);
			return;
			}
					// Don't walk off screen if close, or
					//   if lots of legs, indicating that
					//   Avatar is following this NPC.
		if (from.distance(to) > 80 || legs < 10)
					// Modify 'dest'. to walk off.
			walk_off_screen(screen, to);
		}
	else if (!screen.has_world_point(from.tx, from.ty))
					// Modify src. to walk from off-screen.
					// Causes NPCs to teleport to on-screen far too often
					// (e.g., Blue Boar bartenders)
		//walk_off_screen(screen, from);
		from = Tile_coord(-1, -1, -1);
	blocked = Tile_coord(-1, -1, -1);
	cout << "Finding path to schedule for " << npc->get_name() << endl;
					// Create path to dest., delaying
					//   0 to 1 seconds.
	if (!npc->walk_path_to_tile(from, to, gwin->get_std_delay(),
						first_delay + rand()%1000))
		{			// Wait 1 sec., then try again.
#ifdef DEBUG
		cout << "Failed to find path for " << npc->get_name() << endl;
#endif
		npc->walk_to_tile(dest, gwin->get_std_delay(), 1000);
		retries++;		// Failed.  Try again next tick.
		}
	else				// Okay.  He's walking there.
		{
		legs++;
		retries = 0;
		}
	first_delay = 0;
	}

/*
 *	Don't go dormant when walking to a schedule.
 */

void Walk_to_schedule::im_dormant
	(
	)
	{
	npc->set_action(0); // Prevent segfault when opening/closing doors.
	Walk_to_schedule::now_what();	// Get there by any means.
	}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Walk_to_schedule::get_actual_type
	(
	Actor * /* npc */
	) const
	{
	return new_schedule;
	}

/*
 *	Clear out names.
 */

void Schedule_change::clear
	(
	)
	{
	for (vector<char *>::iterator it = script_names.begin();
				it != script_names.end(); ++it)
		delete *it;
	script_names.resize(0);
	}

/*
 *	Set a schedule from a U7 'schedule.dat' entry.
 */

void Schedule_change::set4
	(
	unsigned char *entry		// 4 bytes read from schedule.dat.
	)
	{
	time = entry[0]&7;
	type = entry[0]>>3;
	days = 0x7f;			// All days of the week.
	unsigned char schunk = entry[3];
	unsigned char x = entry[1], y = entry[2];
	int sx = schunk%c_num_schunks,
	    sy = schunk/c_num_schunks;
	pos = Tile_coord(sx*c_tiles_per_schunk + x, 
			 sy*c_tiles_per_schunk + y, 0);
	}

/*
 *	Set a schedule from an Exult 'schedule.dat' entry (vers. -1).
 */

void Schedule_change::set8
	(
	unsigned char *entry		// 8 bytes read from schedule.dat.
	)
	{
	pos.tx = Read2(entry);
	pos.ty = Read2(entry);
	pos.tz = *entry++;
	time = *entry++;
	type = *entry++;
	days = *entry++;
	}

/*
 *	Write out schedule for Exult's 'schedule.dat'.
 */

void Schedule_change::write8
	(
	unsigned char *entry		// 8 bytes to write to schedule.dat.
	) const
	{
	Write2(entry, pos.tx);
	Write2(entry, pos.ty);		// 4
	*entry++ = (uint8) pos.tz;		// 5
	*entry++ = time;		// 6
	*entry++ = type;		// 7
	*entry++ = days;		// 8
	}

/*
 *	Set a schedule.
 */

void Schedule_change::set
	(
	int ax,
	int ay,
	int az,
	unsigned char stype,
	unsigned char stime
	)
	{
	time = stime;
	type = stype;
	pos = Tile_coord(ax, ay, az);
	}




