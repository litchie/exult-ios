/*
 *	gameclk.h - Keep track of time.
 *
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef GAMECLK_H
#define GAMECLK_H 1

#include "tqueue.h"
#include "palette.h"

/*
 *	The number of times that the game clock ticks in one game minute.
 */
const int ticks_per_minute = 25;

/*
 *	Keep track of time, and of the palette for a given time.
 */
class Game_clock : public Time_sensitive
	{
	Time_queue *tqueue;		// The time queue.
	short hour, minute;		// Time (0-23, 0-59).
	int day;			// Keep track of days played.
	int light_source_level;		// Last set light source level.
	int old_light_level;		// Last set light source level.
	unsigned int dungeon;		// Last set 'in_dungeon' value.
	int overcast;			// >0 if day is overcast (e.g., from a storm).
	bool was_overcast;
	int fog;			// >0 if there is fog.
	bool was_foggy;
	Palette_transition *transition;	// For smooth palette transitions.
	unsigned short time_rate;
	void set_time_palette();
	void set_light_source_level(int lev);
	void check_hunger();
public:
	Game_clock(Time_queue *tq) : tqueue(tq), hour(6), minute(0), day(0),
			light_source_level(0), dungeon(255), time_rate(1),
			overcast(0), fog(0), transition(0),
			was_overcast(false), was_foggy(false)
		{ }
	int get_hour()
		{ return hour; }
	void set_hour(int h)
		{ hour = h; }
	int get_minute()
		{ return minute; }
	void set_minute(int m)
		{ minute = m; }
	int get_day()
		{ return day; }
	void set_day(int d)
		{ day = d; }
	unsigned long get_total_hours()	// Get total # hours.
		{ return day*24 + hour; }
	unsigned long get_total_minutes()
		{ return get_total_hours()*60 + minute; }
	void set_palette();		// Set palette for current hour.
					// Set light source.  MUST be fast,
					//   since it's called during paint().
	void set_light_source(int lev, int dun)
		{
		if (lev != light_source_level || dun != dungeon)
			set_light_source_level(lev);
		}
	void reset()
		{
		overcast = fog = 0;
		was_overcast = was_foggy = false;
		if (transition)
			delete transition;
		transition = 0;
		}
	void set_overcast(bool onoff);	// Start/end cloud cover.
	void set_fog(bool onoff);	// Start/end cloud cover.
	void increment(int num_minutes);// Increment clock.
	virtual void handle_event(unsigned long curtime, long udata);
	void fake_next_period();	// For debugging.
	int get_time_rate() { return time_rate; }
	void set_time_rate(int i) { time_rate=i>0?i:1; }
	};

#endif	/* INCL_GAMECLK */
