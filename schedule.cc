/*
 *	Schedule.cc - Schedules for characters.
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "schedule.h"
#include "actors.h"
#include "Zombie.h"
#include "gamewin.h"
#include "actions.h"
#include "dir.h"
#include "items.h"
#include "game.h"
#include "paths.h"
#include "ucmachine.h"
#include "ucsched.h"
#include "ucscriptop.h"

using std::cout;
using std::endl;
using std::rand;

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
	Tile_coord dest,		// Where to walk to.
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
	Game_window *gwin = Game_window::get_game_window();
	if (npc->Actor::get_npc_num() <= 0 ||
	    npc == gwin->get_camera_actor())
		return 0;		// Only want normal NPC's.
					// At least 30secs. before next one.
	street_maintenance_time = curtime + 30000 + 
				street_maintenance_failures*5000;
	int *shapes;
	int hour = gwin->get_hour();
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
	if (!winrect.has_point(npcpos.tx, npcpos.ty))
		return 0;
					// Get to within 1 tile.
	Actor_pathfinder_client cost(npc, 1);
	Game_object *found = 0;		// Find one we can get to.
	Actor_action *pact;		// Gets ->action to walk there.
	for (int i = 0; !found && i < sizeof(night)/sizeof(night[0]); i++)
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
	street_maintenance_failures = 0;// We did it, so clear failures.
	return 1;
	}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Schedule::get_actual_type
	(
	Actor *npc
	)
	{
	return npc->get_schedule_type();
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
	if (npc->distance(obj) == 1 &&	// We're there.
	    obj->get_shapenum() == shapenum && obj->get_framenum() == framenum)
		{
		cout << npc->get_name() << 
			" about to perform street maintenance" << endl;
		int dir = npc->get_direction(obj);
		char frames[2];
		frames[0] = npc->get_dir_framenum(dir, Actor::standing);
		frames[1] = npc->get_dir_framenum(dir, 3);
		char standframe = frames[0];
		npc->set_action(new Sequence_actor_action(
			new Frames_actor_action(frames, sizeof(frames)),
			new Activate_actor_action(obj),
			new Frames_actor_action(&standframe, 1)));
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
	Game_window *gwin = Game_window::get_game_window();
	int period = gwin->get_hour()/3;
	Npc_actor *nnpc = dynamic_cast<Npc_actor *> (npc);
	if (nnpc)
		nnpc->update_schedule(gwin, period, 7, 0);
	}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Street_maintenance_schedule::get_actual_type
	(
	Actor * /* npc */
	)
	{
	return prev_type;
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
	return (new Pace_schedule(n, Tile_coord(t.tx - 4, t.ty, t.tz),
					Tile_coord(t.tx + 4, t.ty, t.tz)));
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
	return (new Pace_schedule(n, Tile_coord(t.tx, t.ty - 4, t.tz),
					Tile_coord(t.tx, t.ty + 4, t.tz)));
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
	which = !which;			// Flip direction.
	int delay = 750;		// Delay .75 secs.
	Game_window *gwin = Game_window::get_game_window();
	if (blocked.tx != -1 &&		// Blocked?
	    !npc->is_monster())
		{
		Game_object *obj = Game_object::find_blocking(blocked);
		blocked.tx = -1;
		if (obj && (obj->get_npc_num() > 0 || 
					obj == gwin->get_main_actor()))
			{
			npc->say(first_move_aside, last_move_aside);
			delay = 1200;	// Wait longer.
			}
		}
					// Wait 1 sec. before moving.
	npc->walk_to_tile(which ? p1 : p0, gwin->get_std_delay(), delay);
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
			{		// Try again in a while.
			npc->start(250, 5000);
			return;
			}
		}
	Game_window *gwin = Game_window::get_game_window();
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
		if (rand()%5 == 0)
			{
			gwin->add_dirty(food);
			food->remove_this();
			}
		const char *msgs[] = {"Gulp!", "Mmmmm.", "Yum!", ""};
		int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
		npc->say(msgs[n]);
		}
					// Wake up in a little while.
	npc->start(250, 5000 + rand()%12000);
	}

/*
 *	Preach:
 */

void Preach_schedule::now_what
	(
	)
	{
	if (first)			// Find podium.
		{
		first = 0;
		Game_object_vector vec;
		if (npc->find_nearby(vec, 697, 5, 0))
			{
			Game_object *podium = vec[0];
			npc->walk_to_tile(podium->get_tile());
			return;
			}
		}
	char frames[8];			// Frames.
	int cnt = 1 + rand()%(sizeof(frames) - 1);
					// Frames to choose from:
	static char choices[3] = {0, 9, 14};
	for (int i = 0; i < cnt - 1; i++)
		frames[i] = npc->get_dir_framenum(
					choices[rand()%(sizeof(choices))]);
					// Make last one standing.
	frames[cnt - 1] = npc->get_dir_framenum(Actor::standing);
	npc->set_action(new Frames_actor_action(frames, cnt, 250));
					// Do it in 3-?? seconds.
	npc->start(250, 3000 + rand()%8000);
	}

/*
 *	Schedule change for patroling:
 */

void Patrol_schedule::now_what
	(
	)
	{
	if (!dir)			// Sitting?
		{
		dir = 1;		// Activate if we come here again.
					// Stay 5-10 secs.
		if ((npc->get_framenum()&0xf) == Actor::sit_frame)
			npc->start(250, 5000 + rand()%5000);
		else			// Not sitting.
			npc->start(250, rand()%1000);
		return;
		}
	if (rand() % 8 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
	const int PATH_SHAPE = 607;
	Game_object *path;
	if (pathnum >= 0 &&		// Arrived at path?
	    pathnum < paths.size() && (path = paths[pathnum]) != 0 &&
	    npc->distance(path) < 2)
					// Quality = type.  (I think high bits
					//   are flags.
		switch (path->get_quality()&31)
			{
		case 0:			// None.
			break;
		case 1:			// Wrap to 0.
			pathnum = -1;
			dir = 1;
			break;
		case 2:			// Pause.
			break;		//++++++++
		case 3:			// Sit.
			if (Sit_schedule::set_action(npc))
				{
				dir = 0;	// Flag sitting.
				return;
				}
			break;
		case 4:			// Kneel at tombstone.+++++++
		case 5:			// Kneel+++++++++++
		case 6:			// Loiter.++++++
			break;
		case 7:			// Left about-face.
		case 8:			// Right about-face.
			if (pathnum == 0 && dir == -1)
				dir = 1;
			else if (pathnum > 0 && dir == 1)
				dir = -1;
					// ++++Turn animations.
			break;
		case 9:			// Horiz. pace.+++++++
		case 10:		// Vert. pace+++++++
		case 11:		// 50% reverse.
		case 12:		// 50% skip next.
		case 13:		// Hammer.++++++
		case 14:		// Check area.
			break;		//++++++Implement above.
		case 15:		// Usecode.
			{		// Schedule so we don't get deleted.
			Usecode_script *scr = new Usecode_script(npc);
			(*scr) << Ucscript::usecode2 << npc->get_usecode() <<
					static_cast<int>(Usecode_machine::npc_proximity);
			scr->start();	// Start next tick.
#if 0
			Actor *safenpc = npc;
			safenpc->activate(gwin->get_usecode(),
					Usecode_machine::npc_proximity);
			if (safenpc->get_schedule() != this)
				return;	// We're gone.
#endif
			}
		case 16:		// Bow to ground.  ++++Implement.
		case 17:		// Bow from ground.+++++
		case 18:		// Wait for semaphore+++++
		case 19:		// Release semaphore+++++
		case 20:		// Ready weapon++++++++
		case 21:		// Unready weapon+++++++
		case 22:		// One-handed swing.+++++
		case 23:		// Two-handed swing.
		case 24:		// Read++++++
		case 25:		// 50% wrap to 0.++++++
		case 26:		// Combat near??
		case 27:		// Repeat forever??
		default:
			break;
			}
	pathnum += dir;			// Find next path.
					// Already know its location?
	path =  pathnum >= 0 && pathnum < paths.size() ? paths[pathnum] : 0;
	Game_window *gwin = Game_window::get_game_window();
	if (!path)			// No, so look around.
		{
		Game_object_vector nearby;
		npc->find_nearby(nearby, PATH_SHAPE, 25, 0);
		int best_dist = 1000;	// Find closest.
		for (Game_object_vector::const_iterator it = nearby.begin();
						it != nearby.end(); ++it)
			{
			Game_object *obj = *it;
			int framenum = obj->get_framenum();
			int dist;
			if (framenum == pathnum && 
			    (dist = obj->distance(npc)) < best_dist)
				{	// Found it.
				path = obj;
				best_dist = dist;
				}
			}
		if (path)		// Save it.
			{
			failures = 0;	// Found marker.
			paths.put(pathnum, path);
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
					npc->start(200, 500);
					return;
					}
					// After 4, fall through.
				}
			else		// At end, go back to start.
				{
				pathnum = 0;
				path = paths.size() ? paths[0] : 0;
				}
			}
		if (!path)		// Still failed?
			{
					// Wiggle a bit.
			Tile_coord pos = npc->get_tile();
			int dist = failures + 2;
			Tile_coord delta = Tile_coord(rand()%dist - dist/2,
					rand()%dist - dist, 0);
			npc->walk_to_tile(pos + delta, gwin->get_std_delay(), 
								failures*300);
			int pathcnt = paths.size();
			pathnum = rand()%(pathcnt < 4 ? 4 : pathcnt);
			return;
			}
		}
	Tile_coord d = path->get_tile();
    	if (!npc->walk_path_to_tile(d, gwin->get_std_delay(), rand()%1000))
		{			// Look for free tile within 1 square.
		d = Map_chunk::find_spot(d, 1, npc->get_shapenum(),
						npc->get_framenum(), 1);
		if (d.tx == -1 || !npc->walk_path_to_tile(d,
					gwin->get_std_delay(), rand()%1000))
			{		// Failed.  Later.
			npc->start(200, 2000);
			return;
			}
		}
	}

/*
 *	Schedule change for 'talk':
 */

void Talk_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();

	// Switch to phase 3 if we are reasonable close
	if (phase != 0 && phase != 4 &&
	    npc->distance(gwin->get_main_actor()) < 8)
		phase = 3;

	switch (phase)
		{
	case 0:				// Start by approaching Avatar.
		{
		if (npc->distance(gwin->get_main_actor()) > 50)
			{		// Too far?  
					// Try a little later.
			npc->start(250, 5000);
			return;
			}
					// Aim for within 5 tiles.
		Actor_pathfinder_client cost(npc, 5);
		Actor_action *pact = Path_walking_actor_action::create_path(
			npc->get_tile(),
			gwin->get_main_actor()->get_tile(), cost);
		if (!pact)
			{
#ifdef DEBUG
			cout << "Talk: Failed to find path for " << 
						npc->get_name() << endl;
#endif
			npc->follow(gwin->get_main_actor());
			}
		else
			{
					// Walk there, and retry if
					//   blocked.
			npc->set_action(pact);
			npc->start(400, 250);	// Start walking.
			}
		phase++;
		return;
		}
	case 1:				// Wait a second.
	case 2:
		{
		int dx = 1 - 2*(rand()%2);
		npc->walk_to_tile(npc->get_tile() +
			Tile_coord(dx, -dx, 0), 300, 500);
					// Wait til conversation is over.
//		if (gwin->get_usecode()->get_num_faces_on_screen() == 0)
//			phase++;
			phase = 3;
		return;
		}
	case 3:				// Talk.
					// Got to be reachable.
		if (!Fast_pathfinder_client::is_grabable(
			npc->get_tile(),
			gwin->get_main_actor()->get_tile()))
			{
			phase = 0;
			npc->start(250, 1000);
			return;
			}
		gwin->add_dirty(npc);	// But first face Avatar.
		npc->set_frame(npc->get_dir_framenum(npc->get_direction(
				gwin->get_main_actor()), Actor::standing));
		gwin->add_dirty(npc);
		phase++;
		npc->start(250, 250);	// Wait another 1/4 sec.
		break;
	case 4:
		npc->stop();		// Stop moving.
					// NOTE:  This could DESTROY us!
		if (Game::get_game_type() == SERPENT_ISLE)
			npc->activate(gwin->get_usecode(), 9);
		else
			npc->activate(gwin->get_usecode(), 1);
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
	int newx = center.tx - dist + rand()%(2*dist);
	int newy = center.ty - dist + rand()%(2*dist);
					// Wait a bit.
	Game_window *gwin = Game_window::get_game_window();
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
		kid = kids.pop();
		if (npc->distance(kid) < 16)
			break;
		kid = 0;
	}

	if (kid)
	{
		Fast_pathfinder_client cost(1);
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
				kids.push(act);
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
	int dir = static_cast<int>(Get_direction4(cur.ty - dest.ty, dest.tx - cur.tx));
	char frames[4];
	for (int i = 0; i < 4; i++)
					// Spin with 'hands outstretched'.
		frames[i] = npc->get_dir_framenum((2*(dir + i))%8, 
										  npc->get_shapenum() == 846 ? 15 : 9);
					// Create action to walk.
	Actor_action *walk = new Path_walking_actor_action(new Zombie());
	walk->walk_to_tile(npc, cur, dest);
					// Walk, then spin.
	npc->set_action(new Sequence_actor_action(walk,
		new Frames_actor_action(frames, sizeof(frames), 100)));
	Game_window *gwin = Game_window::get_game_window();
	npc->start(gwin->get_std_delay(), 500);		// Start in 1/2 sec.
	}

/*
 *	Schedule change for mining/farming:
 */

void Tool_schedule::now_what
	(
	)
	{
	if (!tool)			// First time?
		{
		tool = new Ireg_game_object(toolshape, 0, 0, 0, 0);
					// Free up both hands.
		Game_object *obj = npc->get_readied(Actor::rhand);
		if (obj)
			obj->remove_this();
		if ((obj = npc->get_readied(Actor::lhand)) != 0)
			obj->remove_this();
		if (!npc->add_readied(tool, Actor::lrhand))
			npc->add_readied(tool, Actor::lhand);
		}
	if (rand()%4 == 0)		// 1/4 time, walk somewhere.
		{
		Loiter_schedule::now_what();
		return;
		}
	char frames[12];		// Use pick.
	int cnt = npc->get_attack_frames(rand()%8, frames);
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
		tool->remove_this();	// Should safely remove from NPC.
	}

/*
 *	Schedule change for hounding the Avatar.
 */

void Hound_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_tile(),
		   npcpos = npc->get_tile();
					// How far away is Avatar?
	int dist = npcpos.distance(avpos);
	if (dist > 20 || dist < 3)	// Too far, or close enough?
		{			// Check again in a few seconds.
		npc->start(gwin->get_std_delay(), 500 + rand()%2000);
		return;
		}
	int newdist = 1 + rand()%2;	// Aim for about 3 tiles from Avatar.
	Fast_pathfinder_client cost(newdist);
	avpos.tx += rand()%3 - 1;	// Vary a bit randomly.
	avpos.ty += rand()%3 - 1;
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
	Game_window *gwin = Game_window::get_game_window();
	if (dest.tx == -1 || !npc->walk_path_to_tile(dest,
					gwin->get_std_delay(), rand()%2000))
					// Failed?  Try again a little later.
		npc->start(250, rand()%3000);
	}

/*
 *	Create a sleep schedule.
 */

Sleep_schedule::Sleep_schedule
	(
	Actor *n
	) : Schedule(n), bed(0), state(0)
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
 *	Schedule change for 'sleep':
 */

void Sleep_schedule::now_what
	(
	)
	{
	if (!bed && state == 0)		// Always find bed.
		{			// Find closest EW or NS bed.
		static int bedshapes[2] = {696, 1011};
		bed = npc->find_closest(bedshapes, 2);
		}
	int frnum = npc->get_framenum();
	if ((frnum&0xf) == Actor::sleep_frame)
		return;			// Already sleeping.
	Game_window *gwin = Game_window::get_game_window();
	switch (state)
		{
	case 0:				// Find path to bed.
		{
		if (!bed)
			{		// Just lie down at current spot.
			gwin->add_dirty(npc);
			int dirbits = npc->get_framenum()&(0x30);
			npc->set_frame(Actor::sleep_frame|dirbits);
			gwin->add_dirty(npc);
			return;
			}
		state = 1;
		Game_object_vector tops;	// Want to find top of bed.
		bed->find_nearby(tops, bed->get_shapenum(), 1, 0);
		for (Game_object_vector::const_iterator it = tops.begin(); 
						it != tops.end(); ++it)
			{
			Game_object *top = *it;
			int frnum = top->get_framenum();
			if (frnum >= spread0 && frnum <= spread1)
				{
				bed = top;
				break;
				}
			}
		Tile_coord bloc = bed->get_tile();
		bloc.tz -= bloc.tx%5;	// Round down to floor level.
		Shape_info& info = gwin->get_info(bed);
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
		int dir = bed->get_shapenum() == 696 ? west : north;
		npc->set_frame(npc->get_dir_framenum(dir, Actor::sleep_frame));
					// Get bed info.
		Shape_info& info = gwin->get_info(bed);
		Tile_coord bedloc = bed->get_tile();
		floorloc = npc->get_tile();
		int bedframe = bed->get_framenum();// Unmake bed.
		if (bedframe >= spread0 && bedframe < spread1 && (bedframe%2))
			{
			bedframe++;
			bed->set_frame(bedframe);
			gwin->add_dirty(bed);
			}
		int bedspread = (bedframe >= spread0 && !(bedframe%2));
					// Put NPC on top of bed.
		npc->move(bedloc.tx, bedloc.ty, bedloc.tz + 
				(bedspread ? 0 : info.get_3d_height()));
		state = 2;
		break;
		}
	default:
		break;
		}
	}

/*
 *	Wakeup time.
 */

void Sleep_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
					// Needed for Skara Brae.
	if (new_type == static_cast<int>(wait) ||	
					// Not time to get up, Penumbra!
	    new_type == static_cast<int>(sleep))
		return;			// ++++Does this leave NPC's stuck?++++
	if (bed &&			// Locate free spot.
	    (npc->get_framenum()&0xf) == Actor::sleep_frame)

		{
		if (floorloc.tx == -1)
					// Want spot on floor.
			floorloc = npc->get_tile();
		floorloc.tz -= floorloc.tz%5;
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
		Actor_vector occ;	// Unless there's another occupant.
		if (frnum >= spread0 && frnum <= spread1 && !(frnum%2) &&
			  bed->find_nearby_actors(occ, c_any_shapenum, 0) < 2)
			bed->set_frame(frnum - 1);
		}
	if (floorloc.tx >= 0)		// Get back on floor.
		npc->move(floorloc);
	npc->set_frame(Actor::standing);
	Game_window *gwin = Game_window::get_game_window();
	gwin->set_all_dirty();		// Update all, since Av. stands up.
	state = 0;			// In case we go back to sleep.
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
		if (!chair || chair->get_shapenum() != 292)
			return;
		if (did_barge_usecode)
			return;		// But NOT more than once for party.
		did_barge_usecode = true;
		Game_window *gwin = Game_window::get_game_window();
		if (gwin->get_moving_barge())
			return;		// Already moving.
		if (npc->get_party_id() < 0 && npc != gwin->get_main_actor())
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
		Game_object_vector fman;// See if Ferryman nearby.
		int usefun = 0x634;	// I hate using constants like this.
		if (chair->find_nearby(fman, 155, 8, 0) == 1)
			{
			usefun = 0x61c;
			}
		did_barge_usecode = true;
					// Special usecode for barge pieces:
		gwin->get_usecode()->call_usecode(usefun, barge,
					Usecode_machine::double_click);
		return;
		}
					// Wait a while if we got up.
	if (!set_action(npc, chair, sat ? (2000 + rand()%3000) : 0, &chair))
		npc->start(200, 5000);	// Failed?  Try again later.
	else
		sat = true;
	}

/*
 *	Action to sit in the chair NPC is in front of.
 */

class Sit_actor_action : public Frames_actor_action
	{
	Game_object *chair;		// Chair.
	Tile_coord chairloc;		// Original chair location.
	Tile_coord sitloc;		// Actually where NPC sits.
	char frames[2];
	static short offsets[8];	// Offsets where NPC should sit.
	char *init(Game_object *chairobj, Actor *actor)
		{
					// Frame 0 faces N, 1 E, etc.
		int dir = 2*(chairobj->get_framenum()%4);
		frames[0] = actor->get_dir_framenum(dir, Actor::to_sit_frame);
		frames[1] = actor->get_dir_framenum(dir, Actor::sit_frame);
		return frames;
		}
	static bool is_occupied(Tile_coord sitloc, Actor *actor)
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
				    frnum == Actor::to_sit_frame)
					return true;
				}
#if 1	/* Seems to work.  Added Nov. 2, 2001 */
		if (actor->get_tile() == sitloc)
			return false;	// We're standing there.
					// See if spot is blocked.
		Game_window *gwin = Game_window::get_game_window();
		Map_chunk *ch = gwin->get_chunk(sitloc.tx/c_tiles_per_chunk,
						sitloc.ty/c_tiles_per_chunk);
		int new_lift;
		ch->setup_cache();	// ++++Maybe a simpler interface??
		if (ch->is_blocked(3, sitloc.tz, sitloc.tx%c_tiles_per_chunk,
				sitloc.ty%c_tiles_per_chunk, new_lift, 
				MOVE_WALK, 0))
			return true;
#endif
		return false;
		}
public:
	Sit_actor_action(Game_object *o, Actor *actor) : chair(o),
			Frames_actor_action(init(o, actor), 2)
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
			static char *msgs[] = {"Put that chair back!",
					"Thief!!", "Thou scoundrel!!",
					"Not funny!", "Who moved my chair??"};
			const int nmsgs = sizeof(msgs)/sizeof(msgs[0]);
			actor->say(msgs[rand()%nmsgs]);
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
	static int desks[2] = {283, 407};
	static int chairs[2] = {873,292};
	Game_object *desk = npc->find_closest(desks, 2);
	if (desk)
		chair = desk->find_closest(chairs, 2);
	}

/*
 *	Schedule change for 'desk work':
 */

void Desk_schedule::now_what
	(
	)
	{
	if (!chair)
		return;			// Failed.
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
		char frames[3];
		frames[0] = npc->get_dir_framenum(Actor::standing);
		frames[1] = npc->get_dir_framenum(Actor::to_sit_frame);
		frames[2] = npc->get_dir_framenum(Actor::sit_frame);
		npc->set_action(new Frames_actor_action(frames,
					sizeof(frames)/sizeof(frames[0])));
		npc->start(250, 10000 + rand()%5000);
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
					// Find 'lab' tables.
	npc->find_nearby(tables, 1003, 20, 0);
	npc->find_nearby(tables, 1018, 20, 0);
	int cnt = tables.size();	// Look for book, chair.
	for (int i = 0; (!book || !chair) && i < cnt; i++)
		{
		static int chairs[2] = {873,292};
		Game_object *table = tables[i];
		Rectangle foot = table->get_footprint();
					// Book on table?
		if (!book && (book = table->find_closest(642, 4)) != 0)
			{
			Tile_coord p = book->get_tile();
			if (!foot.has_point(p.tx, p.ty))
				book = 0;
			}
		if (!chair)
			chair = table->find_closest(chairs, 2, 4);
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
	Game_window *gwin = Game_window::get_game_window();
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
		gwin->add_dirty(cauldron);
					// Set random frame (skip last frame).
		cauldron->set_frame(rand()%(cauldron->get_num_frames() - 1));
		gwin->add_dirty(cauldron);
		npc->add_dirty(gwin);
		npc->set_frame(
			npc->get_dir_framenum(dir, Actor::to_sit_frame));
		npc->add_dirty(gwin);
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
		gwin->add_dirty(book);	// Open book.
		int frnum = book->get_framenum();
		book->set_frame(frnum - frnum%3);
		gwin->add_dirty(book);
		break;
		}
	case stand_up:
		{
		if (book && npc->distance(book) < 4)
			{		// Close book.
			gwin->add_dirty(book);
			int frnum = book->get_framenum();
			book->set_frame(frnum - frnum%3 + 1);
			gwin->add_dirty(book);
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
		Shape_info& info = gwin->get_info(table);
		spot_on_table.tz += info.get_3d_height();
		npc->set_action(new Sequence_actor_action(pact,
			new Face_pos_actor_action(spot_on_table, 200)));
		state = use_potion;
		break;
		}
	case use_potion:
		{
		state = start;
		Exult_vector<Game_object *> potions;
		Game_object::find_nearby(potions, spot_on_table, 340, 0, 0);
		if (potions.size())	// Found a potion.  Remove it.
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
				int nframes = ShapeID(340, 0).get_num_frames();
				Game_object *p = gwin->create_ireg_object(
					gwin->get_info(340), 340,
					rand()%nframes, 0, 0, 0);
				p->move(t);
				}
			}
		char frames[2];
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
 *	Schedule change for 'shy':
 */

void Shy_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor *av = gwin->get_main_actor();
	Tile_coord avpos = av->get_tile(),
		   npcpos = npc->get_tile();
					// How far away is Avatar?
	int dist = npcpos.distance(avpos);
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
		npc->start(200);		// Start walking.
		}
	else					// Try again in a couple secs.
		npc->start(250, 500 + rand()%1000);
	}


/*
 *	Create a 'waiter' schedule.
 */

Waiter_schedule::Waiter_schedule
	(
	Actor *n
	) : Schedule(n), first(1), startpos(n->get_tile()), customer(0)
		
	{
	}

/*
 *	Get a new customer & walk to a prep. table.
 */

void Waiter_schedule::get_customer
	(
	)
{
	Game_window *gwin = Game_window::get_game_window();
	if (customers.empty())			// Got to search?
	{
		Actor_vector vec;		// Look within 32 tiles;
		npc->find_nearby_actors(vec, c_any_shapenum, 32);
		for (Actor_vector::const_iterator it = vec.begin();
							it != vec.end(); ++it)
		{		// Filter them.
			Actor *npc = (Actor *) *it;
			if (npc->get_schedule_type() == Schedule::eat_at_inn)
				customers.push(npc);
		}
	}

	if (!customers.empty())
		customer = customers.pop();
	
	if (prep_tables.size())	// Walk to a 'prep' table.
	{
		Game_object *table = prep_tables[rand()%prep_tables.size()];
		Tile_coord pos = Map_chunk::find_spot(
			table->get_tile(), 1, npc);
		if (pos.tx != -1 &&
		    npc->walk_path_to_tile(pos, gwin->get_std_delay(), 
							1000 + rand()%1000))
			return;
	}
	const int dist = 8;		// Bad luck?  Walk randomly.
	int newx = startpos.tx - dist + rand()%(2*dist);
	int newy = startpos.ty - dist + rand()%(2*dist);
	npc->walk_to_tile(newx, newy, startpos.tz, 2*gwin->get_std_delay(), 
								rand()%2000);
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
			prep_tables.append(table);
		else
			eating_tables.append(table);
		}
	}

/*
 *	Find serving spot for a customer.
 *
 *	Output:	1 if found, with spot set.
 */

int Waiter_schedule::find_serving_spot
	(
	Tile_coord& spot
	)
	{
	Game_object_vector plates;		// First look for a nearby plate.
	int cnt = npc->find_nearby(plates, 717, 1, 0);
	if (!cnt)
		cnt = npc->find_nearby(plates, 717, 2, 0);
	int floor = npc->get_lift()/5;	// Make sure it's on same floor.
	for (Game_object_vector::const_iterator it = plates.begin();
					it != plates.end(); ++it)
		{
		Game_object *plate = *it;
		if (plate->get_lift()/5 == floor)
			{
			spot = plate->get_tile();
			spot.tz++;	// Just above plate.
			return 1;
			}
		}
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord cpos = customer->get_tile();
	
	// Blame MSVC
	{
		// Go through tables.

		for (Game_object_vector::const_iterator it = eating_tables.begin();
						it != eating_tables.end(); ++it)
			{
			Game_object *table = *it;
			Rectangle foot = table->get_footprint();
			if (foot.distance(cpos.tx, cpos.ty) <= 2)
				{		// Found it.
				spot = cpos;	// Start here.
						// East/West of table?
				if (cpos.ty >= foot.y && cpos.ty < foot.y + foot.h)
					spot.tx = cpos.tx <= foot.x ? foot.x
								: foot.x + foot.w - 1;
				else		// North/south.
					spot.ty = cpos.ty <= foot.y ? foot.y
								: foot.y + foot.h - 1;
				if (foot.has_point(spot.tx, spot.ty))
					{	// Passes test.
					Shape_info& info = gwin->get_info(table);
					spot.tz = table->get_lift() +
							info.get_3d_height();
					return 1;
					}
				}
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
	if (rand() % 4 == 0)		// Check for lamps, etc.
		if (try_street_maintenance())
			return;		// We no longer exist.
	if (first)			// First time?
		{
		first = 0;		// Find tables.
		find_tables(971);
		find_tables(633);
		find_tables(847);
		find_tables(1003);
		find_tables(1018);
		find_tables(890);
		find_tables(964);
		find_tables(333);
		}
	int dist = customer ? npc->distance(customer) : 5000;
	if (dist > 32)			// Need a new customer?
		{
		get_customer();		// Find one, and walk to a prep. table.
		return;
		}
	Game_window *gwin = Game_window::get_game_window();
	if (dist < 3)			// Close enough to customer?
		{
		Game_object_vector foods;
		if (customer->find_nearby(foods, 377, 2, 0) > 0)
			{
			const char *msgs[]={"You look like you're doing fine.",
					"Everything okay?",
					"Ready for dessert?",
					""
					};
			int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
			npc->say(msgs[n]);
			}
		else			// Needs food.
			{
			Game_object *food = npc->get_readied(Actor::lhand);
			Tile_coord spot;
			if (food && food->get_shapenum() == 377 &&
			    find_serving_spot(spot))
				{
				const char *msgs[] = {"Enjoy!",
						"Specialty of the house!",
						""
						};
				gwin->add_dirty(npc);
				npc->set_frame(npc->get_dir_framenum(
					npc->get_direction(customer),
							Actor::standing));
				gwin->add_dirty(npc);
				npc->remove(food);
				food->set_invalid();
				food->move(spot);
				int n = rand()%(sizeof(msgs)/sizeof(msgs[0]));
				npc->say(msgs[n]);
				}
			}
		customer = 0;		// Done with this one.
		npc->start(250, rand()%3000);
		return;
		}
					// Walk to customer with food.
	if (!npc->get_readied(Actor::lhand))
		{			// Acquire some food.
		int nfoods = ShapeID(377, 0).get_num_frames();
		int frame = rand()%nfoods;
		Game_object *food = new Ireg_game_object(377, frame, 0, 0, 0);
		npc->add_readied(food, Actor::lhand);
		}
	Tile_coord dest = Map_chunk::find_spot(customer->get_tile(),
					3, npc);
	if (dest.tx != -1 && npc->walk_path_to_tile(dest,
					gwin->get_std_delay(), rand()%1000))
		return;				// Walking there.

	npc->start(200, 2000 + rand()%4000);	// Failed so try again later.
	}

/*
 *	Waiter schedule is done.
 */

void Waiter_schedule::ending
	(
	int new_type			// New schedule.
	)
	{
					// Remove what he/she is carrying.
	Game_object *obj = npc->get_readied(Actor::lhand);
	if (obj)
		obj->remove_this();
	obj = npc->get_readied(Actor::rhand);
	if (obj)
		obj->remove_this();
	}

/*
 *	Sew/weave schedule.
 */

Sew_schedule::Sew_schedule
	(
	Actor *n
	) : Schedule(n), state(get_wool), bale(0), spinwheel(0),
        chair(0), spindle(0), loom(0), cloth(0), work_table(0),
        wares_table(0), sew_clothes_cnt(0)
	{
	}

/*
 *	Sew/weave.
 */

void Sew_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord npcpos = npc->get_tile();
					// Often want to get within 1 tile.
	Actor_pathfinder_client cost(npc, 1);
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
		Shape_info& info = gwin->get_info(work_table);
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
		Game_object *shears = npc->get_readied(Actor::lhand);
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
			npc->add_readied(shears, Actor::lhand);
			}
		state = sew_clothes;
		sew_clothes_cnt = 0;
		break;
		}
	case sew_clothes:
		{
		int dir = npc->get_direction(cloth);
		char frames[5];
		int nframes = npc->get_attack_frames(dir, frames);
		npc->set_action(new Frames_actor_action(frames, nframes));
		sew_clothes_cnt++;
		if (sew_clothes_cnt > 1 && sew_clothes_cnt < 5)
			{
			gwin->add_dirty(cloth);
			int num_cloth_frames = ShapeID(851, 0).get_num_frames();
			cloth->set_frame(rand()%num_cloth_frames);
			gwin->add_dirty(cloth);
			}
		else if (sew_clothes_cnt == 5)
			{
			gwin->add_dirty(cloth);
			Tile_coord pos = cloth->get_tile();
			cloth->remove_this(1);
					// Top or pants.
			int shnum = rand()%2 ? 738 : 249;
			cloth->set_shape(shnum);
			int nframes = ShapeID(shnum, 0).get_num_frames();
			cloth->set_frame(rand()%nframes);
			cloth->move(pos);
			gwin->add_dirty(cloth);
			state = get_clothes;
			}
		break;
		}
	case get_clothes:
		{
		Game_object *shears = npc->get_readied(Actor::lhand);
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
		Shape_info& info = gwin->get_info(wares_table);
		Tile_coord cpos(foot.x + rand()%foot.w, foot.y + rand()%foot.h,
			wares_table->get_lift() + info.get_3d_height());
		if (pact)
			npc->set_action(new Sequence_actor_action(pact,
				new Pickup_actor_action(cloth, cpos, 250)));
		cloth = 0;			// Leave it be.
		break;
		}
	case done:				// Just put down clothing.
		{
		state = get_wool;
		Game_object_vector vec;		// Don't create too many.
		int cnt = npc->find_nearby(vec, 738, 4, 0);
		cnt += npc->find_nearby(vec, 249, 4, 0);
		if (cnt > 5)
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
	Game_window *gwin = Game_window::get_game_window();
					// Remove shears.
	Game_object *obj = npc->get_readied(Actor::lhand);
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
 *	Bake bread/pastries
 *
 * TODO: fix for SI tables... (specifically in Moonglow)
 */

Bake_schedule::Bake_schedule(Actor *n) : Schedule(n),
	state(to_flour), oven(0), worktable(0), displaytable(0),
	flourbag(0), dough(0), dough_in_oven(0), baked_count(0)
	{ }

void Bake_schedule::now_what()
{
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord npcpos = npc->get_tile();
	Actor_pathfinder_client cost(npc, 1);
	int delay = 100;

	switch (state) {
	case to_flour:
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
	case get_flour:
	{
		if (!flourbag) {
			// what are we doing here then? back to start
			state = to_flour;
			break;
		}

		int dir = npc->get_direction(flourbag);
		npc->add_dirty(gwin);
		npc->set_frame(npc->get_dir_framenum(dir,Actor::to_sit_frame));
		npc->add_dirty(gwin);

		if (flourbag->get_framenum() != 0) {
			flourbag->set_frame(0);
			gwin->add_dirty(flourbag);
		}

		delay = 750;
		state = to_table;
		break;
	}
	case to_table:
	{
		Game_object *table1 = npc->find_closest(1003);
		Game_object *table2 = npc->find_closest(1018);

		if (!table1)
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

					// Find where to put dough.
		Rectangle foot = worktable->get_footprint();
		Shape_info& info = gwin->get_info(worktable);
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
				dough = new Ireg_game_object(863, 16, 0, 0);
			else
				dough = new Ireg_game_object(658, 0, 0, 0);
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
	case make_dough:
	{
		if (!dough) {
			// better try again...
			delay = 2500;
			state = to_table;
			break;
		}

		int dir = npc->get_direction(dough);
		char fr[2];
		fr[0] = npc->get_dir_framenum(dir, 3);
		fr[1] = npc->get_dir_framenum(dir, 0);

		npc->set_action(new Sequence_actor_action(
				new Frames_actor_action(fr, 2, 500),
		((Game::get_game_type() == SERPENT_ISLE) ?
				new Frames_actor_action("\x11",1,250,dough) :
				new Frames_actor_action("\x01",1,250,dough)),
				new Frames_actor_action(fr, 2, 500),
		((Game::get_game_type() == SERPENT_ISLE) ?
				new Frames_actor_action("\x12",1,250,dough) :
				new Frames_actor_action("\x02",1,250,dough))
				));
		
		state = remove_from_oven;
		break;
	}
	case remove_from_oven:
	{
		if (!dough_in_oven) {
			// nothing in oven yet
			state = get_dough;
			break;
		}

		oven = npc->find_closest(831);
		if (!oven) {
			// this really shouldn't happen...
			dough_in_oven->remove_this();
			dough_in_oven = 0;

			delay = 2500;
			state = to_table;
			break;
		}

		gwin->add_dirty(dough_in_oven);
		dough_in_oven->set_shape(377);
		dough_in_oven->set_frame(rand()%7);
		gwin->add_dirty(dough_in_oven);

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
	case display_wares:
	{
		displaytable = npc->find_closest(633);
		if (!displaytable) {
			// uh-oh...
			dough_in_oven->remove_this();
			dough_in_oven = 0;

			delay = 2500;
			state = to_flour;
			break;
		}

		baked_count++;

		Tile_coord tpos = displaytable->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
					// Find where to put cloth.
		Rectangle foot = displaytable->get_footprint();
		Shape_info& info = gwin->get_info(displaytable);
		Tile_coord cpos(foot.x + rand()%foot.w, foot.y + rand()%foot.h,
			displaytable->get_lift() + info.get_3d_height());
		if (pact) {
			if (baked_count <= 5) {
				npc->set_action(new Sequence_actor_action(pact,
					new Pickup_actor_action(dough_in_oven,
							 cpos, 250)));
			} else {
				npc->set_action(pact);
				dough_in_oven->remove_this();
			}
			dough_in_oven = 0;
		} else {
			// just make it vanish
			dough_in_oven->remove_this();
			dough_in_oven = 0;
		}

		state = get_dough;
		break;
	}
	case get_dough:
	{
		if (!dough) {
			// try again
			delay = 2500;
			state = to_flour;
			break;
		}

		oven = npc->find_closest(831);
		if (!oven) {
			// wait a while
			delay = 2500;
			state = to_flour;
			break;
		}

		Tile_coord tpos = dough->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);
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
	case put_in_oven:
	{
		if (!dough) {
			// try again
			delay = 2500;
			state = to_flour;
			break;
		}

		oven = npc->find_closest(831);
		if (!oven) {
			// oops... retry
			dough->remove_this();
			dough = 0;

			delay = 2500;
			state = to_table;
			break;
		}

		Tile_coord tpos = oven->get_tile() + 
						Tile_coord(1, 1, 0);
		Actor_action *pact = Path_walking_actor_action::create_path(
					npcpos, tpos, cost);

		Rectangle foot = oven->get_footprint();
		Shape_info& info = gwin->get_info(oven);
		Tile_coord cpos(foot.x + 1, foot.y,
				oven->get_lift() + info.get_3d_height());

		if (pact) {
			npc->set_action(new Sequence_actor_action(
				pact,
				new Pickup_actor_action(dough, cpos, 250)));

			dough_in_oven = dough;
			dough = 0;
		} else {
			dough->remove_this();
			dough = 0;
		}

		state = to_flour;
		break;
	}
	}
	npc->start(250, delay);		// Back in queue.
}

void Bake_schedule::ending(int new_type)
{
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
 *	Blacksmith.
 *
 * Note: the original kept the tongs & hammer, and put them on a nearby table
 */

Forge_schedule::Forge_schedule(Actor *n) : Schedule(n), 
	state(put_sword_on_firepit), tongs(0), hammer(0), blank(0),
	firepit(0), anvil(0), trough(0), bellows(0)
	{ }

void Forge_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
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

		firepit = npc->find_closest(739);
		if (!firepit) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			return;
		}

		Tile_coord tpos = firepit->get_tile();
		Actor_action *pact = Path_walking_actor_action::create_path(
				npcpos, tpos, cost);

		Rectangle foot = firepit->get_footprint();
		Shape_info& info = gwin->get_info(firepit);
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
		a[2] = new Frames_actor_action("\x2b", 1, 0);
		a[3] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[4] = new Frames_actor_action("\x20", 1, 0);
		a[5] = new Frames_actor_action("\x01", 1, 0, firepit);
		a[6] = new Frames_actor_action("\x01", 1, 0, blank);
		a[7] = new Frames_actor_action("\x2b", 1, 0);
		a[8] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[9] = new Frames_actor_action("\x20", 1, 0);
		a[10] = new Frames_actor_action("\x02", 1, 0, blank);
		a[11] = new Frames_actor_action("\x2b", 1, 0);
		a[12] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[13] = new Frames_actor_action("\x20", 1, 0);
		a[14] = new Frames_actor_action("\x02", 1, 0, firepit);
		a[15] = new Frames_actor_action("\x03", 1, 0, blank);
		a[16] = new Frames_actor_action("\x2b", 1, 0);
		a[17] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[18] = new Frames_actor_action("\x20", 1, 0);
		a[19] = new Frames_actor_action("\x03", 1, 0, firepit);
		a[20] = new Frames_actor_action("\x04", 1, 0, blank);
		a[21] = new Frames_actor_action("\x2b", 1, 0);
		a[22] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[23] = new Frames_actor_action("\x20", 1, 0);
		a[24] = new Frames_actor_action("\x2b", 1, 0);
		a[25] =	new Object_animate_actor_action(bellows, 3, 1, 300);
		a[26] = new Frames_actor_action("\x20", 1, 0);
		a[27] = new Frames_actor_action("\x2b", 1, 0);
		a[28] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[29] = new Frames_actor_action("\x20", 1, 0);
		a[30] = new Frames_actor_action("\x2b", 1, 0);
		a[31] = new Object_animate_actor_action(bellows, 3, 1, 300);
		a[32] = new Frames_actor_action("\x20", 1, 0);
		a[33] = new Frames_actor_action("\x00", 1, 0, bellows);
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
		if (!tongs)
			tongs = new Ireg_game_object(994, 0, 0, 0);

		npc->add_dirty(gwin);
		npc->add_readied(tongs, Actor::rhand);
		npc->add_dirty(gwin);

		state = sword_on_anvil;
		break;
	}
	case sword_on_anvil:
	{
		anvil = npc->find_closest(991);
		firepit = npc->find_closest(739);
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
		Shape_info& info = gwin->get_info(anvil);
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

		if (!hammer)
			hammer = new Ireg_game_object(623, 0, 0, 0);

		npc->add_dirty(gwin);
		if (tongs) {
			tongs->remove_this();
			tongs = 0;
		}
		npc->add_readied(hammer, Actor::rhand);
		npc->add_dirty(gwin);

		state = use_hammer;
		break;
	}
	case use_hammer:
	{
		anvil = npc->find_closest(991);
		firepit = npc->find_closest(739);
		if (!anvil || !firepit || !blank) {
			// uh-oh... try again in a few seconds
			npc->start(250, 2500);
			state = put_sword_on_firepit;
			return;
		}

		char frames[12];
		int cnt = npc->get_attack_frames(0, frames);
		npc->set_action(new Frames_actor_action(frames, cnt));
		
		Actor_action **a = new Actor_action*[10];
		a[0] = new Frames_actor_action(frames, cnt);
		a[1] = new Frames_actor_action("\x03", 1, 0, blank);
		a[2] = new Frames_actor_action("\x02", 1, 0, firepit);
		a[3] = new Frames_actor_action(frames, cnt);
		a[4] = new Frames_actor_action("\x02", 1, 0, blank);
		a[5] = new Frames_actor_action("\x01", 1, 0, firepit);
		a[6] = new Frames_actor_action(frames, cnt);
		a[7] = new Frames_actor_action("\x01", 1, 0, blank);
		a[8] = new Frames_actor_action("\x00", 1, 0, firepit);
		a[9] = 0;
		npc->set_action(new Sequence_actor_action(a));

		state = walk_to_trough;
		break;
	}
	case walk_to_trough:
	{
		npc->add_dirty(gwin);
		if (hammer) {
			hammer->remove_this();
			hammer = 0;
		}
		npc->add_dirty(gwin);

		trough = npc->find_closest(719);
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

		int dir = npc->get_direction(trough);
		gwin->add_dirty(trough);
		trough->set_frame(3);
		gwin->add_dirty(trough);
		npc->add_dirty(gwin);
		npc->set_frame(
			npc->get_dir_framenum(dir, Actor::to_sit_frame));
		npc->add_dirty(gwin);

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
		if (!tongs)
			tongs = new Ireg_game_object(994, 0, 0, 0);

		npc->add_dirty(gwin);
		npc->add_readied(tongs, Actor::rhand);
		npc->add_dirty(gwin);

		state = use_trough;
		break;
	}
	case use_trough:
	{
		trough = npc->find_closest(719);
		anvil = npc->find_closest(991);
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
			char troughframe = trough->get_framenum() - 1;
			if (troughframe < 0) troughframe = 0;

			int dir = npc->get_direction(trough);
			char npcframe = npc->get_dir_framenum(dir, Actor::to_sit_frame);

			Actor_action **a = new Actor_action*[7];
			a[0] = pact;
			a[1] = new Pickup_actor_action(blank, 250);
			a[2] = pact2;
			a[3] = new Frames_actor_action(&npcframe, 1, 250);
			a[4] = new Frames_actor_action(&troughframe, 1, 0, trough);
			a[5] = new Frames_actor_action("\x00", 1, 0, blank);
			a[6] = 0;
			npc->set_action(new Sequence_actor_action(a));
		} else {
			// no path found, just pick up sword blank
			npc->set_action(new Sequence_actor_action(
			 	new Pickup_actor_action(blank, 250),
				new Frames_actor_action("\0", 1, 0, blank)));
		}	

		state = done;
		break;
	}
	case done:
	{
		npc->add_dirty(gwin);
		if (tongs) {
			tongs->remove_this();
			tongs = 0;
		}
		npc->add_dirty(gwin);
		

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
	Game_window *gwin = Game_window::get_game_window();
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

	if (firepit && firepit->get_framenum() != 0) {
		firepit->set_frame(0);
		gwin->add_dirty(firepit);
	}
	if (bellows && bellows->get_framenum() != 0) {
		bellows->set_frame(0);
		gwin->add_dirty(bellows);
	}
	if (blank && blank->get_framenum() != 0) {
		blank->set_frame(0);
		gwin->add_dirty(blank);
	}

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
	Tile_coord d,			// Destination.
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
	Game_window *gwin = Game_window::get_game_window();
	if (npc->get_tile().distance(dest) <= 3)
		{			// Close enough!
		npc->set_schedule_type(new_schedule);
		return;
		}
	if (legs >= 30 || retries >= 2)	// Trying too hard?
		{			// Going to jump there.
		npc->move(dest.tx, dest.ty, dest.tz);
		npc->set_schedule_type(new_schedule);
		return;
		}
					// Get screen rect. in tiles.
	Rectangle screen = gwin->get_win_tile_rect();
	screen.enlarge(4);		// Enlarge in all dirs.
					// Might do part of it first.
	Tile_coord from = npc->get_tile(),
		   to = dest;
					// Destination off the screen?
	if (!screen.has_point(to.tx, to.ty))
		{
		if (!screen.has_point(from.tx, from.ty))
			{		// Force teleport on next tick.
			retries = 100;
			npc->start(200, 100);
			return;
			}
					// Modify 'dest'. to walk off.
		walk_off_screen(screen, to);
		}
	else if (!screen.has_point(from.tx, from.ty))
					// Modify src. to walk from off-screen.
		walk_off_screen(screen, from);
	blocked = Tile_coord(-1, -1, -1);
	cout << "Finding path to schedule for " << npc->get_name() << endl;
					// Create path to dest., delaying
					//   0 to 2 seconds.
	if (!npc->walk_path_to_tile(from, to, gwin->get_std_delay(),
						first_delay + rand()%2000))
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
	Walk_to_schedule::now_what();	// Get there by any means.
	}

/*
 *	Get actual schedule (for Usecode intrinsic).
 */

int Walk_to_schedule::get_actual_type
	(
	Actor * /* npc */
	)
	{
	return new_schedule;
	}

/*
 *	Set a schedule.
 */

void Schedule_change::set
	(
	unsigned char *entry		// 4 bytes read from schedule.dat.
	)
	{
	time = entry[0]&7;
	type = entry[0]>>3;
	x = entry[1];
	y = entry[2];
	superchunk = entry[3];
	}

/*
 *	Get a schedule.
 */

void Schedule_change::get
	(
	unsigned char *entry		// 4 bytes to write to schedule.dat.
	)
	{
	entry[0] = time&7;
	entry[0] |= (type&31) << 3;
	entry[1] = x;
	entry[2] = y;
	entry[3] = superchunk;
	}

/*
 *	Set a schedule.
 */

void Schedule_change::set
	(
	int ax,
	int ay,
	unsigned char stype,
	unsigned char stime
	)
	{
	time = stime;
	type = stype;
	x = static_cast<unsigned char>(ax%c_tiles_per_schunk);
	y = static_cast<unsigned char>(ay%c_tiles_per_schunk);
	superchunk = static_cast<unsigned char>((ay/c_tiles_per_schunk)*12 + (ax/c_tiles_per_schunk));
	}

/*
 *	Get position.
 */

Tile_coord Schedule_change::get_pos() const
	{
	int cx = 16*(superchunk%12) + x/16,
	    cy = 16*(superchunk/12) + y/16,
	    tx = x%16,
	    ty = y%16;
	return Tile_coord(cx*c_tiles_per_chunk + tx, cy*c_tiles_per_chunk + ty, 0);
	}



