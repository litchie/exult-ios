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

/*
 *	This class provides A* cost methods.
 */
class Actor_pathfinder_client : public Pathfinder_client
	{
public:
					// Figure cost for a single step.
	virtual int get_step_cost(Tile_coord from, Tile_coord& to);
					// Estimate cost between two points.
	virtual int estimate_cost(Tile_coord& from, Tile_coord& to);
					// Is tile at the goal?
	virtual int at_goal(Tile_coord& tile, Tile_coord& goal);
	};

/*
 *	Figure cost going from one tile to an adjacent tile (for pathfinding).
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

int Actor_pathfinder_client::get_step_cost
	(
	Tile_coord from,
	Tile_coord& to			// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int cx = to.tx/tiles_per_chunk, cy = to.ty/tiles_per_chunk;
	Chunk_object_list *olist = gwin->get_objects(cx, cy);
	int tx = to.tx%tiles_per_chunk;	// Get tile within chunk.
	int ty = to.ty%tiles_per_chunk;
	olist->setup_cache();		// Make sure cache is valid.
	int new_lift;			// Might climb/descend.
					// For now, assume height=3.
	if (olist->is_blocked(3, to.tz, tx, ty, new_lift))
		{			//+++++++Check for door.
					//+++++++Need method to get shape.
		return -1;
		}
	int cost = 1;
	if (new_lift != to.tz)
		{
		cost++;
		to.tz = new_lift;
		}
					// Maybe check types of ground?
	return (cost);
	}

/*
 *	Estimate cost from one point to another.
 */

int Actor_pathfinder_client::estimate_cost
	(
	Tile_coord& from,
	Tile_coord& to
	)
	{
	return from.distance(to);
	}

/*
 *	Is tile at goal?
 */

int Actor_pathfinder_client::at_goal
	(
	Tile_coord& tile,
	Tile_coord& goal
	)
	{
	return (tile.tx == goal.tx && tile.ty == goal.ty &&
		(goal.tz == -1 || tile.tz == goal.tz));
	}

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
	PathFinder *p			// Already set to path.
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
	Tile_coord cur = actor->get_abs_tile_coord();
	int newdir = (int) Get_direction4(cur.ty - t.ty, t.tx - cur.tx);
	Frames_sequence *frames = actor->get_frames(newdir);
					// Get frame (updates frame_index).
	int frame = frames->get_next(frame_index);
	return actor->step(t, frame);
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
	Tile_coord src,
	Tile_coord dest
	)
	{
	Actor_pathfinder_client cost;
					// Set up new path.
	if (!path->NewPath(src, dest, &cost))
		return (0);
					// Reset direction (but not index).
	original_dir = (int) Get_direction4(
				src.ty - dest.ty, dest.tx - src.tx);
	return (this);
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



