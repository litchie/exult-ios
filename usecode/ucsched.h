/*
Copyright (C) 2000 The Exult team

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

#ifndef _UCSCHED_H
#define _UCSCHED_H

class Usecode_internal;

#include "useval.h"
#include "tqueue.h"
#include "egg.h"
#include "actors.h"

/*
 *	A class for executing usecode at a scheduled time:
 */
class Scheduled_usecode : public Time_sensitive
	{
	static int count;		// Total # of these around.
	static Scheduled_usecode *first;// ->chain of all of them.
	Scheduled_usecode *next, *prev;	// Next/prev. in global chain.
	Usecode_value objval;		// The 'itemref' object.
	Game_object *obj;		// From objval.
	Tile_coord objpos;		// Abs. tile coord.
	Usecode_value arrval;		// Array of code to execute.
	int cnt;			// Length of arrval.
	int i;				// Current index.
	int frame_index;		// For taking steps.
	int no_halt;			// 1 to ignore halt().
	int delay;			// Used for restoring.
					// For restore:
	Scheduled_usecode(Game_object *item, Usecode_value& aval, int findex,
						int nhalt, int del);
public:
	Scheduled_usecode(Usecode_internal *usecode,
				Usecode_value& oval, Usecode_value& aval);
	virtual ~Scheduled_usecode()
		{
		count--;
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = next;
		else
			first = next;
		}
	void halt()			// Stop executing.
		{
		if (!no_halt)
			i = cnt;
		}
	int is_activated()		// Started already?
		{ return i > 0; }
	inline void activate_egg(Usecode_internal *usecode, 
				 Game_object *e, int type);

	static int get_count()
		{ return count; }
					// Find for given item.
	static Scheduled_usecode *find(Game_object *srch);
	static void clear();		// Delete all.
					// Activate itemref eggs.
	void activate_eggs(Usecode_internal *usecode);
	virtual void handle_event(unsigned long curtime, long udata);
					// Move object in given direction.
	void step(Usecode_internal *usecode, int dir);
					// Save/restore.
	int save(unsigned char *buf, int buflen);
	static Scheduled_usecode *restore(Game_object *item,
					unsigned char *buf, int buflen);
	};


#endif
