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

#include "gamewin.h"
#include "actions.h"
#include "actors.h"
#include "Zombie.h"
#include "Astar.h"
#include "paths.h"
#include "dir.h"

/*
 *	Set to walk from one point to another the dumb way.
 *
 *	Output:	->this, or 0 if unsuccessful.
 */

Actor_action *Actor_action::walk_to_tile
	(
	Tile_coord src,
	Tile_coord dest
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
 *	Create action to follow a path.
 */

Path_walking_actor_action::Path_walking_actor_action
	(
	PathFinder *p,			// Already set to path.
	int maxblk			// Max. retries when blocked.
	) : path(p), frame_index(0), blocked(0), max_blocked(maxblk)
	{
	Tile_coord src = p->get_src(), dest = p->get_dest();
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
	Tile_coord tile;
	if (blocked)
		{
#if DEBUG
cout << "Actor " << actor->get_name() << " blocked.  Retrying." << endl;
#endif
		if (actor->step(blocked_tile, blocked_frame))
			{		// Successful?
			blocked = 0;
			return speed;
			}
					// Wait up to 1.6 secs.
		return (blocked++ > max_blocked ? 0 
					: 100 + blocked*(rand()%500));
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
	if (actor->step(tile, frame))	// Successful.
		return speed;
	if (!max_blocked ||		// No retries allowed?
	    actor->is_dormant())	// Or actor off-screen?
		return 0;
	blocked = 1;
	blocked_tile = tile;
	blocked_frame = frame;
	return (100 + rand()%500);	// Wait .1 to .6 seconds.
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
	Tile_coord dest			// Same here.
	)
	{
	blocked = 0;			// Clear 'blocked' count.
					// Set up new path.
					// Don't care about 1 coord.?
	if (dest.tx == -1 || dest.ty == -1)
		{
		Onecoord_pathfinder_client cost;
		if (!path->NewPath(src, dest, &cost))
			return (0);
		}
					// How about from source?
	else if (src.tx == -1 || src.ty == -1)
		{			// Figure path in opposite dir.
		Onecoord_pathfinder_client cost;
		if (!path->NewPath(dest, src, &cost))
			return (0);
					// Set to go backwards.
		if (!path->set_backwards())
			return (0);
		}
	else
		{
		Actor_pathfinder_client cost;
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
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Move_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (dest.tx < 0)
		return (0);		// Done.
	actor->move(dest);		// Zip right there.
	dest.tx = -1;			// Set to stop.
	return (100);			// Wait 1/10 sec.
	}

/*
 *	Create sequence of frames.
 */

Frames_actor_action::Frames_actor_action
	(
	char *f,			// Frames.  -1 means don't change.
	int c,				// Count.
	int spd				// Frame delay in 1/1000 secs.
	) : cnt(c), index(0), speed(spd)
	{
	frames = new char[cnt];
	memcpy(frames, f, cnt);
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
	if (index == cnt)
		return (0);		// Done.
	int frnum = frames[index++];	// Get frame.
	if (frnum >= 0)
		{
		Game_window *gwin = Game_window::get_game_window();
		gwin->add_dirty(actor);
		actor->set_frame(frnum);
		gwin->add_dirty(actor);
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
	delete actions;
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
					// Do current action.
	int delay = actions[index]->handle_event(actor);
	if (!delay)
		{
		index++;		// That one's done now.
		delay = 100;		// 1/10 second.
		}
	return (delay);
	}



