/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actions.h - Action controllers for actors.
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

#ifndef INCL_ACTIONS
#define INCL_ACTIONS	1

#include "tiles.h"

class Actor;
class Tile_coord;
class PathFinder;

/*
 *	This class controls the current actions of an actor:
 */
class Actor_action
	{
public:
	virtual ~Actor_action() { }
					// Handle time event.
	virtual int handle_event(Actor *actor) = 0;
	virtual void stop(Actor *actor)	// Stop moving.
		{  }
					// Set simple path to destination.
	virtual Actor_action *walk_to_tile(Tile_coord src, Tile_coord dest, int move_flags);
					// Get destination, or ret. 0.
	virtual int get_dest(Tile_coord& dest)
		{ return 0; }
					// Check for Astar.
	virtual int following_smart_path()
		{ return 0; }
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
	PathFinder *path;		// Allocated pathfinder.
	int original_dir;		// From src. to dest. (0-7).
	int frame_index;		// Index within frame sequence.
	int speed;			// Time between frames.
	unsigned char blocked;		// Blocked-tile retries.
	unsigned char max_blocked;	// Try this many times.
	unsigned char blocked_frame;	// Frame for blocked tile.
	Tile_coord blocked_tile;	// Tile to retry.
public:
	Path_walking_actor_action(PathFinder *p, int maxblk = 3);
	virtual ~Path_walking_actor_action();
					// Handle time event.
	virtual int handle_event(Actor *actor);
	virtual void stop(Actor *actor);// Stop moving.
					// Set simple path to destination.
	virtual Actor_action *walk_to_tile(Tile_coord src, Tile_coord dest, int move_flags);
					// Get destination, or ret. 0.
	virtual int get_dest(Tile_coord& dest);
					// Check for Astar.
	virtual int following_smart_path();
	};

/*
 *	Just move (i.e. teleport) to a desired location.
 */
class Move_actor_action : public Actor_action
	{
	Tile_coord dest;		// Where to go.
public:
	Move_actor_action(Tile_coord d) : dest(d)
		{  }
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

#if 0	/* +++Maybe not needed. */
/*
 *	Approach an enemy during combat.
 */
class Combat_path_actor_action : public Actor_action
	{
	PathFinder *path;		// Allocated pathfinder.
	int frame_index;		// Index within frame sequence.
public:
	Combat_path_walking_actor_action(PathFinder *p);
	virtual ~Combat_path_walking_actor_action();
					// Handle time event.
	virtual int handle_event(Actor *actor);
					// Get destination, or ret. 0.
	virtual int get_dest(Tile_coord& dest);
	};
#endif

/*
 *	Go through a series of frames.
 */
class Frames_actor_action : public Actor_action
	{
	char *frames;			// List to go through (a -1 means to
					//   leave frame alone.)
	int cnt;			// Size of list.
	int index;			// Index for next.
	int speed;			// Frame delay in 1/1000 secs.
public:
	Frames_actor_action(char *f, int c, int spd = 200);
	virtual ~Frames_actor_action()
		{ delete [] frames; }
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
public:
					// Create with allocated list.
	Sequence_actor_action(Actor_action **act) : actions(act), index(0)
		{  }
					// Create with up to 4.
	Sequence_actor_action(Actor_action *a0, Actor_action *a1,
				Actor_action *a2 = 0, Actor_action *a3 = 0);
	virtual ~Sequence_actor_action();
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

#endif	/* INCL_ACTIONS */

