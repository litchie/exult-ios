/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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

#include "objs.h"

/*
 *	A 'barge', such as a ship or horse-and-cart.  The elements of a barge
 *	are stored in the outside world, so rendering and obstacle detection
 *	don't have to be reimplemented.
 */
class Barge_object : public Container_game_object, public Time_sensitive
	{
	Vector objects;			// All objects in/on barge.
	int perm_count;			// Counts permanent parts of barge,
					//   which proceed those placed on it.
	unsigned char xtiles, ytiles;	// Tiles covered (when vert?).
	unsigned char horizontal;	// Flag:  1 if horizontal.
	unsigned char complete;		// Flag:  all members have been read.
	int frame_time;			// Time between frames in msecs.  0 if
					//   not moving.
	PathFinder *path;		// For traveling.
	Game_object *get_object(int i)
		{ return (Game_object *) objects.get(i); }
public:
	Barge_object(unsigned char l, unsigned char h, 
		unsigned int shapex, unsigned int shapey, unsigned int lft,
			int xt, int yt, int horiz)
		: Container_game_object(l, h, shapex, shapey, lft),
			perm_count(0),
			xtiles(xt), ytiles(yt), horizontal(horiz),
			complete(0), frame_time(0), path(0)
		{  }
#if 0
	Barge_object() : Ireg_game_object()
		{  }
#endif
	virtual ~Barge_object();
	void gather();			// Gather up objects on barge.
					// For Time_sensitive:
	virtual void handle_event(unsigned long curtime, long udata);
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Render.
	virtual void paint(Game_window *gwin);
	int step(Tile_coord t);		// Step onto an (adjacent) tile.
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	virtual void elements_read();	// Called when all member items read.
	};

#endif
