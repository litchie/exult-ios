/*
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

#ifndef _UCSCHED_H
#define _UCSCHED_H

class Game_object;
class Usecode_value;
class Usecode_internal;

#include "tqueue.h"
#include "databuf.h"

/*
 *  A class for executing usecode at a scheduled time:
 */
class Usecode_script : public Time_sensitive {
	static int count;       // Total # of these around.
	static Usecode_script *first;// ->chain of all of them.
	Usecode_script *next, *prev;    // Next/prev. in global chain.
	Game_object *obj;       // From objval.
	Usecode_value *code;        // Array of code to execute.
	int cnt;            // Length of arrval.
	int i;              // Current index.
	int frame_index;        // For taking steps.
	bool no_halt;           // 1 to ignore halt().
	bool must_finish;       // 1 to finish before deleting.
	bool killed_barks;      // 1 to prevent barks from showing.
	int delay;          // Used for restoring.
	// For restore:
	Usecode_script(Game_object *item, Usecode_value *cd, int findex,
	               int nhalt, int del);
public:
	Usecode_script(Game_object *o, Usecode_value *cd = 0);
	virtual ~Usecode_script();
	void start(long delay = 1); // Start after 'delay' msecs.
	long get_delay() const {
		return delay;
	}
	void halt();            // Stop executing.
	bool is_no_halt() const {   // Is the 'no_halt' flag set?
		return no_halt;
	}
	int is_activated() const {  // Started already?
		return i > 0;
	}
	void add(int v1);       // Append new instructions:
	void add(int v1, int v2);
	void add(int v1, const char *str);
	void add(int *vals, int cnt);
	Usecode_script &operator<<(int v) {
		add(v);
		return *this;
	}
	inline void activate_egg(Usecode_internal *usecode,  Game_object *e);

	static int get_count() {
		return count;
	}
	// Find for given item.
	static Usecode_script *find(Game_object *srch,
	                            Usecode_script *last_found = 0);
	static Usecode_script *find_active(Game_object *srch,
	                                   Usecode_script *last_found = 0);
	static void terminate(Game_object *obj);
	static void clear();        // Delete all.
	// Remove all whose objs. are too far.
	static void purge(Tile_coord const &pos, int dist);
	virtual void handle_event(unsigned long curtime, long udata);
	int exec(Usecode_internal *usecode, bool finish);
	// Move object in given direction.
	void step(Usecode_internal *usecode, int dir, int dz);
	// Save/restore.
	int save(DataSource *out) const;
	static Usecode_script *restore(Game_object *item, DataSource *in);
	void print(std::ostream &out) const;    // Print values.
	void kill_barks() {
		killed_barks = true;
	}
};


#endif
