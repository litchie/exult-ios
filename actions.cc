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

#include "actions.h"
#include "actors.h"
#include "PathFinder.h"

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action, else delay for next frame.
 */

int Walking_actor_action::handle_event
	(
	Actor *actor
	)
	{
	return actor->walk();
	}

/*
 *	Create action to follow a path.
 */
Path_walking_actor_action::Path_walking_actor_action
	(
	PathFinder *p			// Controls pathfinding.
	) : path(p), frame_index(0)
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
	Tile_coord t;
	if (!path->GetNextStep(t))
		return (0);
					// ++++For now, just use original dir.
	Frames_sequence *frames = actor->get_frames(original_dir);
					// Get frame (updates frame_index).
	int frame = frames->get_next(frame_index);
	return actor->step(t, frame);
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



