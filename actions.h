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

class Actor;
class Tile_coord;
class PathFinder;

/*
 *	This class controls the current actions of an actor:
 */
class Actor_action
	{
public:
					// Handle time event.
	virtual int handle_event(Actor *actor) = 0;
	};

/*
 *	Walk an actor towards a goal.
 */
class Walking_actor_action : public Actor_action
	{
public:
	virtual ~Walking_actor_action()
		{  }
					// Handle time event.
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
public:
	Path_walking_actor_action(PathFinder *p);
	~Path_walking_actor_action();
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
	Sequence_actor_action(Actor_action **act) : actions(act), index(0)
		{  }
	~Sequence_actor_action();
					// Handle time event.
	virtual int handle_event(Actor *actor);
	};

#endif	/* INCL_ACTIONS */

