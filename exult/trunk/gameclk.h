/**
 **	Gameclk.h - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#ifndef INCL_GAMECLK
#define INCL_GAMECLK 1

#include "tqueue.h"

/*
 *	Keep track of time, and of the palette for a given time.
 */
class Game_clock : public Time_sensitive
	{
	Time_queue *tqueue;		// The time queue.
	short hour, minute;		// Time (0-23, 0-59).
	int day;			// Keep track of days played.
	int light_source_level;		// Light source level.
	int in_dungeon;			// 1 if in a dungeon.
	void set_time_palette();
	void set_light_source_level(int lev);
public:
	Game_clock(Time_queue *tq) : tqueue(tq), hour(6), minute(0), day(0),
			light_source_level(0), in_dungeon(0)
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
	void set_palette();		// Set palette for current hour.
					// Set light source.  MUST be fast,
					//   since it's called during paint().
	void set_light_source(int lev = 1)
		{
		if (lev != light_source_level)
			set_light_source_level(lev);
		}
	void set_in_dungeon(int tf);
	void increment(int num_minutes);// Increment clock.
	virtual void handle_event(unsigned long curtime, long udata);
	void fake_next_period();	// For debugging.
	};

#endif	/* INCL_GAMECLK */
