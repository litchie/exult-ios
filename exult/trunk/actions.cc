/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actions.cc - Action controllers for actors.
 **
 **	Written: 4/20/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "gamewin.h"
#include "actions.h"
#include "actors.h"
#include "Zombie.h"
#include "Astar.h"
#include "paths.h"
#include "dir.h"
#include "ucmachine.h"
#include "frameseq.h"

using std::cout;
using std::endl;


/*
 *	Set to walk from one point to another the dumb way.
 *
 *	Output:	->this, or 0 if unsuccessful.
 */

Actor_action *Actor_action::walk_to_tile
	(
	Tile_coord src,
	Tile_coord dest,
	int move_flags
	)
	{
	Zombie *path = new Zombie();
					// Set up new path.
	if (path->NewPath(src, dest, 0))
		return (new Path_walking_actor_action(path));
	else
		{
		delete path;
		return (0);
		}
	}

/*
 *	Set up an action to get an actor to a location (via pathfinding), and
 *	then execute another action when he gets there.
 */

Actor_action *Actor_action::create_action_sequence
	(
	Actor *actor,			// Whom to activate.
	Tile_coord dest,		// Where to walk to.
	Actor_action *when_there,	// What to do when he gets there.
	bool from_off_screen,		// Have actor walk from off-screen.
	bool no_teleport		// Don't teleport if no path found.
	)
	{
	Actor_action *act = when_there;
	Tile_coord actloc = actor->get_abs_tile_coord();
	if (from_off_screen)
		actloc.tx = actloc.ty = -1;
	if (dest != actloc)		// Get to destination.
		{
		Actor_action *w = new Path_walking_actor_action(new Astar());
		Actor_action *w2 = w->walk_to_tile(actloc, dest, 
						actor->get_type_flags());
		if (w2 != w)
			delete w;
		if (!w2)		// Failed?  Teleport.
			{
			if (no_teleport)// Or, just do action here.
				return act;
			w2 = new Move_actor_action(dest);
			}
					// And teleport if blocked walking.
		Actor_action *tel = new Move_actor_action(dest);
					// Walk there, then do whatever.
		act = new Sequence_actor_action(w2, tel, act);
		}
	return act;
	}

/*
 *	Null action.
 */

int Null_action::handle_event
	(
	Actor *actor
	)
	{
	return 0;
	}

/*
 *	Create action to follow a path.
 */

Path_walking_actor_action::Path_walking_actor_action
	(
	PathFinder *p,			// Pathfinder, or 0 for Astar.
	int maxblk			// Max. retries when blocked.
	) : path(p), frame_index(0), from_offscreen(false), subseq(0),
	    blocked(0), max_blocked(maxblk)
	{
	if (!path)
		path = new Astar();
	Tile_coord src = path->get_src(), dest = path->get_dest();
	original_dir = (int) Get_direction4(
				src.ty - dest.ty, dest.tx - src.tx);
	}

/*
 *	Delete.
 */

Path_walking_actor_action::~Path_walking_actor_action
	(
	)
	{
	delete path;
	delete subseq;
	subseq = 0;			// (Debugging).
	original_dir = -1;
	}

/*
 *	Create action for walking to given destination using Astar.
 *	Note:  This is a static method.
 *
 *	Output:	Action if successful, else 0.
 */

Path_walking_actor_action *Path_walking_actor_action::create_path
	(
	Tile_coord src,			// Starting position.
	Tile_coord dest,		// Destination.
	Pathfinder_client& cost		// Cost for Astar.
	)
	{
	Astar *path = new Astar();
					// Get to within 1 tile.
	if (path->NewPath(src, dest, &cost))
		return new Path_walking_actor_action(path);
	else
		{
		delete path;
		return 0;
		}
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Path_walking_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (subseq)			// Going through a door?
		{
		int delay = subseq->handle_event(actor);
		if (delay)
			return delay;	// Still going.
		set_subseq(0);
					// He was stopped, so restore speed.
		actor->set_frame_time(speed);
		return speed;		// Come back in a moment.
		}
	Tile_coord tile;
	if (blocked)
		{
#if 0
std::cout << "Actor " << actor->get_name() << " blocked.  Retrying." << std::endl;
#endif
		if (actor->step(blocked_tile, blocked_frame))
			{		// Successful?
			blocked = 0;
			return speed;
			}
					// Wait up to 1.6 secs.
		return (blocked++ > max_blocked ? 0 
					: 100 + blocked*(std::rand()%500));
		}
	speed = actor->get_frame_time();// Get time between frames.
	if (!speed)
		return 0;		// Not moving.
	if (!path->GetNextStep(tile))
		return (0);
	Tile_coord cur = actor->get_abs_tile_coord();
	int newdir = (int) Get_direction4(cur.ty - tile.ty, tile.tx - cur.tx);
	Frames_sequence *frames = actor->get_frames(newdir);
					// Get frame (updates frame_index).
	int frame = frames->get_next(frame_index);
	if (from_offscreen)		// Teleport to 1st spot.
		{
		from_offscreen = false;
		actor->move(tile.tx, tile.ty, tile.tz);
		return speed;
		}
	else if (actor->step(tile, frame))	// Successful.
		return speed;
	Game_window *gwin = Game_window::get_game_window();
					// Blocked by a door?
	if (actor->get_abs_tile_coord().distance(tile) == 1 &&
					// But not a party member.
	    (actor != gwin->get_main_actor() && actor->get_party_id() < 0))
		{
		Game_object *door = Game_object::find_blocking(tile);
		if (door != 0 && door->is_closed_door())
					// Try to open it.
			{
			if (open_door(actor, door))
				return speed;
			}
		}
	if (!max_blocked ||		// No retries allowed?
	    actor->is_dormant())	// Or actor off-screen?
		return 0;
	blocked = 1;
	blocked_tile = tile;
	blocked_frame = frame;
	return (100 + std::rand()%500);	// Wait .1 to .6 seconds.
	}

/*
 *	Open door that's blocking the NPC, and set action to walk past and
 *	close it.
 *
 *	Output:	1 if successful.
 */

int Path_walking_actor_action::open_door
	(
	Actor *actor,
	Game_object *door
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Tile_coord cur = actor->get_abs_tile_coord();
					// Get door's footprint in tiles.
	Rectangle foot = door->get_footprint();
					// Open it.
	door->activate(gwin->get_usecode());
	Tile_coord past;		// Tile on other side of door.
	past.tz = cur.tz;
	int dir;			// Get dir to face door afterwards.
	if (foot.w > foot.h)		// Horizontal?
		{
		past.tx = foot.x + foot.w/2;
		if (cur.ty <= foot.y)	// N. of door?
			{
			past.ty = foot.y + foot.h;
			dir = 0;
			}
		else			// S. of door?
			{
			past.ty = foot.y - 1;
			dir = 4;
			}
		}
	else				// Vertical.
		{
		past.ty = foot.y + foot.h/2;
		if (cur.tx <= foot.x)	// W. of door?
			{
			past.tx = foot.x + foot.w;
			dir = 6;
			}
		else			// E. of door?
			{
			past.tx = foot.x - 1;
			dir = 2;
			}
		}
	Tile_coord tmp = Game_object::find_unblocked_tile(past, 0, 3);
	if (tmp.tx != -1)
		past = tmp;
	else
		past = Game_object::find_unblocked_tile(past, 1, 3);
	if (past.tx != -1)		// Succeeded.  Walk past and close it.
		{
		cout << "Path_walking_actor_action::open_door()" << endl;
		char frames[2];
		frames[0] = actor->get_dir_framenum(dir, Actor::standing);
		frames[1] = actor->get_dir_framenum(dir, 3);
		char standframe = frames[0];
		set_subseq(create_action_sequence(actor, past,
			new Sequence_actor_action(
				new Frames_actor_action(frames, 
							sizeof(frames)),
				new Activate_actor_action(door),
				new Frames_actor_action(&standframe, 1))));
		return 1;
		}
	return 0;
	}

/*
 *	Stopped moving.
 */

void Path_walking_actor_action::stop
	(
	Actor *actor
	)
	{
					// ++++For now, just use original dir.
	Frames_sequence *frames = actor->get_frames(original_dir);
	actor->set_frame(frames->get_resting());
	}

/*
 *	Set to walk from one point to another, using the same pathfinder.
 *
 *	Output:	->this, or 0 if unsuccessful.
 */

Actor_action *Path_walking_actor_action::walk_to_tile
	(
	Tile_coord src,			// tx=-1 or ty=-1 means don't care.
	Tile_coord dest,		// Same here.
	int move_flags
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	blocked = 0;			// Clear 'blocked' count.
	from_offscreen = false;
					// Set up new path.
					// Don't care about 1 coord.?
	if (dest.tx == -1 || dest.ty == -1)
		{
		if (dest.tx == dest.ty)	// Completely off-screen?
			{
			Offscreen_pathfinder_client cost(gwin, move_flags);
			if (!path->NewPath(src, dest, &cost))
				return (0);
			}
		else
			{
			Onecoord_pathfinder_client cost(move_flags);
			if (!path->NewPath(src, dest, &cost))
				return (0);
			}
		}
					// How about from source?
	else if (src.tx == -1 || src.ty == -1)
		{			// Figure path in opposite dir.
		if (src.tx == src.ty)	// Both -1?
			{
			Offscreen_pathfinder_client cost(gwin, move_flags);
			if (!path->NewPath(dest, src, &cost))
				return (0);
			from_offscreen = true;
			}
		else
			{
			Onecoord_pathfinder_client cost(move_flags);
			if (!path->NewPath(dest, src, &cost))
				return (0);
			}
					// Set to go backwards.
		if (!path->set_backwards())
			return (0);
		}
	else
		{
		Actor_pathfinder_client cost(move_flags);
		if (!path->NewPath(src, dest, &cost))
			return (0);
		}
					// Reset direction (but not index).
	original_dir = (int) Get_direction4(
				src.ty - dest.ty, dest.tx - src.tx);
	return (this);
	}

/*
 *	Return current destination.
 *
 *	Output:	0 if none.
 */

int Path_walking_actor_action::get_dest
	(
	Tile_coord& dest		// Returned here.
	)
	{
	dest = path->get_dest();
	return (1);
	}

/*
 *	Following an Astar path?
 */

int Path_walking_actor_action::following_smart_path
	(
	)
	{
	return path != 0 && path->following_smart_path();
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Move_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (dest.tx < 0 || actor->get_abs_tile_coord() == dest)
		return (0);		// Done.
	actor->move(dest);		// Zip right there.
	Game_window *gwin = Game_window::get_game_window();
	if (actor == gwin->get_main_actor())
					// Teleported Avatar?
		gwin->center_view(dest);
	dest.tx = -1;			// Set to stop.
	return (100);			// Wait 1/10 sec.
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Activate_actor_action::handle_event
	(
	Actor *actor
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	obj->activate(gwin->get_usecode());
	return 0;			// That's all.
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Usecode_actor_action::handle_event
	(
	Actor *actor
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->get_usecode()->call_usecode(fun, item, 
			(Usecode_machine::Usecode_events) eventid);
	gwin->set_all_dirty();		// Clean up screen.
	gwin->set_mode(Game_window::normal);
	return 0;			// That's all.
	}

/*
 *	Create sequence of frames.
 */

Frames_actor_action::Frames_actor_action
	(
	char *f,			// Frames.  -1 means don't change.
	int c,				// Count.
	int spd,			// Frame delay in 1/1000 secs.
	Game_object *o
	) : cnt(c), index(0), speed(spd), obj(o)
	{
	frames = new char[cnt];
	std::memcpy(frames, f, cnt);
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Frames_actor_action::handle_event
	(
	Actor *actor
	)
	{
	Game_object *o = obj;
	if (index == cnt)
		return (0);		// Done.
	int frnum = frames[index++];	// Get frame.
	if (frnum >= 0)
		{
		Game_window *gwin = Game_window::get_game_window();
		if (o) {
			gwin->add_dirty(o);
			o->set_frame(frnum);
			gwin->add_dirty(o);
		} else {
			actor->add_dirty(gwin);	// Get weapon to redraw too.
			actor->set_frame(frnum);
			actor->add_dirty(gwin, 1);
		}
		}
	return (speed);
	}

/*
 *	Create a sequence with up to 4 actions.
 */

Sequence_actor_action::Sequence_actor_action
	(
	Actor_action *a0,		// (These will be deleted when done.)
	Actor_action *a1,
	Actor_action *a2,
	Actor_action *a3
	) : index(0)
	{
	actions = new Actor_action *[5];// Create list.
	actions[0] = a0;
	actions[1] = a1;
	actions[2] = a2;
	actions[3] = a3;
	actions[4] = 0;			// 0-delimit.
	}

/*
 *	Delete.
 */

Sequence_actor_action::~Sequence_actor_action
	(
	)
	{
	for (int i = 0; actions[i]; i++)
		delete actions[i];
	delete [] actions;
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Sequence_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (!actions[index])		// Done?
		return (0);
	int old_sched_type = actor->get_schedule_type();
	Actor_action *old_action = actor->get_action();
					// Do current action.
	int delay = actions[index]->handle_event(actor);
	if (actor->get_action() != old_action ||
	    actor->get_schedule_type() != old_sched_type)
		return 0;		// We've been deleted!
	if (!delay)
		{
		index++;		// That one's done now.
		delay = 100;		// 1/10 second.
		}
	return (delay);
	}

/*
 *	Create object animator.
 */
Object_animate_actor_action::Object_animate_actor_action
	(
	Game_object *o,
	int cy,				// # of cycles.
	int spd				// Time between frames.
	) : obj(o), cycles(cy), speed(spd)
	{
	Game_window *gwin = Game_window::get_game_window();
	nframes = gwin->get_shape_num_frames(obj->get_shapenum());
	}

Object_animate_actor_action::Object_animate_actor_action
	(
	Game_object *o,
	int nfr,
	int cy,
	int spd
	) : obj(o), nframes(nfr), cycles(cy), speed(spd) 
	{ }
	

/*
 *	Handle tick of the clock.
 */

int Object_animate_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (!cycles) return 0;
	int frnum = (obj->get_framenum() + 1) % nframes;
	if (!frnum)			// New cycle?
		--cycles;
	Game_window *gwin = Game_window::get_game_window();
	gwin->add_dirty(obj);		// Paint over old.
	obj->set_frame(frnum);
	gwin->add_dirty(obj);
	return (cycles ? speed : 0);
	}

/*
 *	Pick up/put down an object.
 */
Pickup_actor_action::Pickup_actor_action(Game_object *o, int spd)
	: obj(o), pickup(1), speed(spd), cnt(0), 
	  objpos(obj->get_abs_tile_coord()), dir(0)
	{
	}
					// To put down an object:
Pickup_actor_action::Pickup_actor_action(Game_object *o, Tile_coord opos, 
								int spd)
	: obj(o), pickup(0), speed(spd), cnt(0), objpos(opos), dir(0)
	{
	}

/*
 *	Pick up an item (or put it down).
 */

int Pickup_actor_action::handle_event
	(
	Actor *actor
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int frnum = -1;
	switch (cnt)
		{
	case 0:				// Face object.
		dir = actor->get_direction(objpos);
		frnum = actor->get_dir_framenum(dir, Actor::standing);
		cnt++;
		break;
	case 1:				// Bend down.
		frnum = actor->get_dir_framenum(dir, Actor::to_sit_frame);
		cnt++;
		if (pickup)
			{
			gwin->add_dirty(obj);
			obj->remove_this(1);
			actor->add(obj, 1);
			}
		else
			{
			obj->remove_this(1);
			obj->move(objpos);
			gwin->add_dirty(obj);
			}
		break;
	default:
		return 0;		// Done.
		}
	actor->add_dirty(gwin);		// Get weapon to redraw too.
	actor->set_frame(frnum);
	actor->add_dirty(gwin, 1);
	return speed;
	}

/*
 *	Action to turn towards a position or an object.
 */

Face_pos_actor_action::Face_pos_actor_action(Tile_coord p, int spd)
	: speed(spd), pos(p)
	{
	}
Face_pos_actor_action::Face_pos_actor_action(Game_object *o, int spd)
	: speed(spd),
	  pos(o->get_abs_tile_coord())
	{
	}

/*
 *	Just turn to face a tile.
 */

int Face_pos_actor_action::handle_event
	(
	Actor *actor
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int dir = actor->get_direction(pos);
	int frnum = actor->get_dir_framenum(dir, Actor::standing);
	if (actor->get_framenum() == frnum)
		return 0;		// There.
	actor->add_dirty(gwin);		// Get weapon to redraw too.
	actor->set_frame(frnum);
	actor->add_dirty(gwin, 1);
	return speed;
	}

