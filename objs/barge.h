/**
 **	Barge.h - Ships, carts, flying-carpets.
 **
 **	Written: 7/13/2000 - JSF
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

#ifndef INCL_BARGE
#define INCL_BARGE	1

#include "contain.h"

/*
 *	A 'barge', such as a ship or horse-and-cart.  The elements of a barge
 *	are stored in the outside world, so rendering and obstacle detection
 *	don't have to be reimplemented.
 */
class Barge_object : public Container_game_object, public Time_sensitive
	{
	Game_object_vector objects;	// All objects in/on barge.
	int perm_count;			// Counts permanent parts of barge,
					//   which proceed those placed on it.
	unsigned char xtiles, ytiles;	// Tiles covered (when vertical).
	unsigned char dir;		// Direction: 0=N, 1=E, 2=S, 3=W.
	bool complete;			// Flag:  all members have been read.
	bool gathered;			// Items on barge have been gathered.
	bool ice_raft;			// For Serpent Isle.
	bool first_step;		// So first motion can just be 1 tile.
	signed char boat;		// 1 if a boat, 0 if not; -1=untested.
	int frame_time;			// Time between frames in msecs.  0 if
					//   not moving.
	PathFinder *path;		// For traveling.
	Tile_coord center;		// Center of barge.
	Game_object *get_object(int i)
		{ return objects[i]; }
	void swap_dims();
	void set_center();
	int okay_to_rotate(Tile_coord pos);
	void add_dirty();
					// Finish up move/rotate operation.
	void finish_move(Tile_coord *positions, int newmap = -1);
public:
	Barge_object(int shapenum, int framenum,
		unsigned int shapex, unsigned int shapey, unsigned int lft,
			int xt, int yt, int d)
		: Container_game_object(shapenum, framenum,
							shapex, shapey, lft),
			perm_count(0),
			xtiles(xt), ytiles(yt), dir(d),
			complete(false), gathered(false), ice_raft(false),
			first_step(true), boat(-1), frame_time(0), path(0)
		{  }
	Rectangle get_tile_footprint();
	bool is_moving()
		{ return frame_time > 0; }
	int get_xtiles()		// Dims. in tiles.
		{ return xtiles; }
	int get_ytiles()
		{ return ytiles; }
	Tile_coord get_center()
		{ return center; }
	virtual ~Barge_object();
	void set_to_gather()		// Require 'gather' on next move.
		{ gathered = false; }
	void gather();			// Gather up objects on barge.
	void face_direction(int ndir);	// Face dir. (0-7).
					// Start rolling/sailing.
	void travel_to_tile(Tile_coord dest, int speed);
	void turn_right();		// Turn 90 degrees right.
	void turn_left();
	void turn_around();
	void stop()			// Stop moving.
		{ frame_time = 0; first_step = true; }
	void done();			// No longer being operated.
	int okay_to_land();		// See if clear to land.
	virtual Barge_object *as_barge() { return this; }
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
							bool combine = false);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Render.
	virtual void paint();
					// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame = -1);
					// Write out to IREG file.
	virtual void write_ireg(DataSource* out);
				// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();
	virtual void elements_read();	// Called when all member items read.
	};

#endif
