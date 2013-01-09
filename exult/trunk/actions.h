/*
 *	actions.h - Action controllers for actors.
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

#ifndef ACTIONS_H
#define ACTIONS_H	1

#include "tiles.h"

class Actor;
class Game_object;
class Tile_coord;
class PathFinder;
class Pathfinder_client;
class Path_walking_actor_action;
class If_else_path_actor_action;

/*
 *	This class controls the current actions of an actor:
 */
class Actor_action
	{
	static long seqcnt;		// Sequence # to check for deletion.
protected:
	bool get_party;			// At each step (of the Avatar), have
					//   the party follow.
	long seq;			// 'unique' sequence #.
public:
	Actor_action() : get_party(false)
		{ seq = ++seqcnt; }
	virtual ~Actor_action() { }
	void set_get_party(bool tf = true)
		{ get_party = true; }
	int handle_event_safely(Actor *actor, bool& deleted);
					// Handle time event.
	virtual int handle_event(Actor *actor) = 0;
	virtual void stop(Actor *actor)	// Stop moving.
		{  }
					// Set simple path to destination.
	virtual Actor_action *walk_to_tile(Actor *npc, Tile_coord const& src, 
			Tile_coord const& dest, int dist = 0, bool ignnpc = false);
					// Set action to walk to dest, then
					//   exec. another action when there.
	static Actor_action *create_action_sequence(Actor *actor, 
			Tile_coord const& dest, Actor_action *when_there, 
					bool from_off_screen = false, bool ignnpc = false);
					// Get destination, or ret. 0.
	virtual int get_dest(Tile_coord& dest) const
		{ return 0; }
					// Check for Astar.
	virtual int following_smart_path() const
		{ return 0; }
	virtual If_else_path_actor_action *as_usecode_path()
		{ return 0; }
	virtual int get_speed() const
		{ return 0; }
	virtual Actor_action *kill()
		{
		delete this;
		return 0;
		}
	};

/*
 *	A null action just returns 0 the first time.
 */
class Null_action : public Actor_action
	{
public:
	Null_action() {  }
	virtual int handle_event(Actor *actor);
	};

/*
 *	Follow a path.
 */
class Path_walking_actor_action : public Actor_action
	{
protected:
	bool reached_end;		// Reached end of path.
	PathFinder *path;		// Allocated pathfinder.
	bool deleted;			// True if the action has been killed.
private:
	int original_dir;		// From src. to dest. (0-7).
	int speed;			// Time between frames.
	bool from_offscreen;		// Walking from offscreen.
	Actor_action *subseq;		// For opening doors.
	unsigned char blocked;		// Blocked-tile retries.
	unsigned char max_blocked;	// Try this many times.
	unsigned char blocked_frame;	// Frame for blocked tile.
	unsigned char persistence;
	Tile_coord blocked_tile;	// Tile to retry.
	void set_subseq(Actor_action *sub)
		{
		delete subseq;
		subseq = sub;
		}
public:
	Path_walking_actor_action(PathFinder *p = 0, int maxblk = 3, int pers = 0);
	virtual ~Path_walking_actor_action();
	static Path_walking_actor_action *create_path(Tile_coord const& src,
			Tile_coord const& dest, Pathfinder_client& cost);
					// Handle time event.
	virtual int handle_event(Actor *actor);
	int open_door(Actor *actor, Game_object *door);
	virtual void stop(Actor *actor);// Stop moving.
					// Set simple path to destination.
	virtual Actor_action *walk_to_tile(Actor *npc, Tile_coord const& src, 
				Tile_coord const& dest, int dist = 0, bool persistant = false);
					// Get destination, or ret. 0.
	virtual int get_dest(Tile_coord& dest) const;
					// Check for Astar.
	virtual int following_smart_path() const;
	virtual int get_speed() const
		{ return speed; }
	virtual Actor_action *kill()
		{
		deleted = true;
		return this;
		}
	};

/*
 *	Follow a path to approach a given object, and stop half-way if it
 *	moved.
 */
class Approach_actor_action : public Path_walking_actor_action
	{
	Game_object *dest_obj;		// Destination object.
	int goal_dist;			// Stop if within this distance.
	Tile_coord orig_dest_pos;	// Dest_obj's pos. when we start.
	int cur_step;			// Count steps.
	int check_step;			// Check at this step.
	bool for_projectile;		// Check for proj. path.
public:
	Approach_actor_action(PathFinder *p, Game_object *d, 
			int gdist = -1, bool for_proj = false);
	static Approach_actor_action *create_path(Tile_coord const& src,
		Game_object *dest, int gdist, Pathfinder_client& cost);
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

/*
 *	Follow a path and execute one action if successful, another if
 *	failed.
 */
class If_else_path_actor_action : public Path_walking_actor_action
	{
	bool succeeded, failed, done;
	Actor_action *success, *failure;
public:
	If_else_path_actor_action(Actor *actor, Tile_coord const& dest, 
		Actor_action *s, Actor_action *f = 0);
	~If_else_path_actor_action();
	void set_failure(Actor_action *f);
	bool done_and_failed() const		// Happens if no path found in ctor.
		{ return done && failed; }
	bool is_done() const
		{ return done; }
					// Handle time event.
	virtual int handle_event(Actor *actor);
	virtual If_else_path_actor_action *as_usecode_path()
		{ return this; }
	};

/*
 *	Just move (i.e. teleport) to a desired location.
 */
class Move_actor_action : public Actor_action
	{
	Tile_coord dest;		// Where to go.
public:
	Move_actor_action(Tile_coord const& d) : dest(d)
		{  }
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

/*
 *	Activate an object.
 */
class Activate_actor_action : public Actor_action
	{
	Game_object *obj;
public:
	Activate_actor_action(Game_object *o) : obj(o)
		{  }
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

/*
 *	Go through a series of frames.
 */
class Frames_actor_action : public Actor_action
	{
	signed char *frames;		// List to go through (a -1 means to
					//   leave frame alone.)
	int cnt;			// Size of list.
	int index;			// Index for next.
	int speed;			// Frame delay in 1/1000 secs.
	Game_object *obj;		// Object to animate
public:
	Frames_actor_action(signed char *f, int c, int spd = 200, Game_object *o = 0);
	Frames_actor_action(char f, int spd = 200, Game_object *o = 0);
	virtual ~Frames_actor_action()
		{ delete [] frames; }
					// Handle time event.
	virtual int handle_event(Actor *actor);
	int get_index() const
		{ return index; }
	virtual int get_speed() const
		{ return speed; }
	};

/*
 *	Call a usecode function.
 */
class Usecode_actor_action : public Actor_action
	{
	int fun;			// Fun. #.
	Game_object *item;		// Call it on this item.	
	int eventid;
public:
	Usecode_actor_action(int f, Game_object *i, int ev)
		: fun(f), item(i), eventid(ev)
		{  }	
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

/*
 *	Do a sequence of actions.
 */
class Sequence_actor_action : public Actor_action
	{
	Actor_action **actions;		// List of actions, ending with null.
	int index;			// Index into list.
	int speed;			// Frame delay in 1/1000 secs. between
					//   actions.
public:
					// Create with allocated list.
	Sequence_actor_action(Actor_action **act, int spd = 100) 
					: actions(act), index(0), speed(spd)
		{  }
					// Create with up to 4.
	Sequence_actor_action(Actor_action *a0, Actor_action *a1,
				Actor_action *a2 = 0, Actor_action *a3 = 0);
	virtual int get_speed() const
		{ return speed; }
	void set_speed(int spd)
		{ speed = spd; }
	virtual ~Sequence_actor_action();
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

//
//	The below could perhaps go into a highact.h file.
//

/*
 *	Rotate through an object's frames.
 */
class Object_animate_actor_action : public Actor_action
	{
	Game_object *obj;
	int nframes;			// # of frames.
	int cycles;			// # of cycles to do.
	int speed;			// Time between frames.
public:
	Object_animate_actor_action(Game_object *o, int cy, int spd);
	Object_animate_actor_action(Game_object *o, int nframes, int cy, int spd);
					// Handle time event.
	virtual int handle_event(Actor *actor);
	virtual int get_speed() const
		{ return speed; }
	};

/*
 *	Action to pick up an item or put it down.
 */

class Pickup_actor_action : public Actor_action
	{
	Game_object *obj;		// What to pick up/put down.
	int pickup;			// 1 to pick up, 0 to put down.
	int speed;			// Time between frames.
	int cnt;			// 0, 1, 2.
	Tile_coord objpos;		// Where to put it.
	int dir;			// Direction to face.
	bool temp;			// True to make object temporary on drop.
public:
					// To pick up an object:
	Pickup_actor_action(Game_object *o, int spd);
					// To put down an object:
	Pickup_actor_action(Game_object *o, Tile_coord const& opos, int spd, bool t = false);
	virtual int handle_event(Actor *actor);
	virtual int get_speed() const
		{ return speed; }
	};
	

/*
 *	Action to turn towards an object or spot.
 */

class Face_pos_actor_action : public Actor_action
	{
	int speed;			// Time between frames.
	Tile_coord pos;			// Where to put it.
public:
	Face_pos_actor_action(Tile_coord const& p, int spd);
					// To pick up an object:
	Face_pos_actor_action(Game_object *o, int spd);
	virtual int handle_event(Actor *actor);
	virtual int get_speed() const
		{ return speed; }
	};
	

#endif	/* INCL_ACTIONS */

