/**
 **	Actors.h - Game actors.
 **
 **	Written: 11/3/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef INCL_ACTORS
#define INCL_ACTORS	1

#include "objs.h"

class Image_window;

/*
 *	An actor:
 */
class Actor : public Sprite
	{
	Actor *next, *prev;		// Next, prev. in vicinity.
	char *name;			// Its name.
	int usecode;			// # of usecode function.
	int npc_num;			// # in Game_window::npcs list, or -1.
	short properties[12];		// Properties set/used in 'usecode'.
public:
	void set_default_frames();	// Set usual frame sequence.
	Actor(char *nm, int shapenum, int num = -1, int uc = -1);
	~Actor()
		{ delete name; }
	int get_npc_num()		// Get its ID.
		{ return npc_num; }
	int get_face_shapenum()		// Get "portrait" shape #.
		{ return npc_num; }	// It's the NPC's #.
	void add_after_this(Actor *a)	// Add another actor after this.
		{
					// Remove from current chain.
		a->next->prev = a->prev;
		a->prev->next = a->next;
		a->next = next;		// Add to this one.
		a->prev = this;
		next->prev = a;
		next = a;
		}
	Actor *get_next()
		{ return next; }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	virtual char *get_name();
	virtual void set_property(int prop, int val)
		{
		if (prop >= 0 && prop < 12)
			properties[prop] = (short) val;
		}
	virtual int get_property(int prop)
		{ return (prop >= 0 && prop < 12) ? properties[prop] : 0; }
	};

/*
 *	A non-player-character that one can converse with:
 */
class Npc_actor : public Actor
	{
public:
	Npc_actor(char *nm, int shapenum, int fshape = -1, int uc = -1);
	~Npc_actor();
	};

/*
 *	Here's an actor that's just hanging around an area.
 */
class Area_actor : public Npc_actor
	{
	timeval next_change;		// When to change motion.
public:
	Area_actor(char *nm, int shapenum, int fshape = -1) : Npc_actor(nm, shapenum, fshape)
		{
		next_change.tv_sec = next_change.tv_usec = 0;
		}
					// Figure next frame location.
	virtual int next_frame(timeval& time,
		int& new_cx, int& new_cy, int& new_sx, int& new_sy,
		int& new_frame);
	};

#endif
