/**
 **	Gameclk.h - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#ifndef INCL_GAMECLK
#define INCL_GAMECLK 1

#include "tqueue.h"

/*
 *	Keep track of time.
 */
class Game_clock : public Time_sensitive
	{
	Time_queue *tqueue;		// The time queue.
	short hour, minute;		// Time (0-23, 0-59).
	int day;			// Keep track of days played.
public:
	Game_clock(Time_queue *tq) : tqueue(tq), hour(6), minute(0), day(0)
		{ }
	int get_hour()
		{ return hour; }
	int get_minute()
		{ return minute; }
	int get_day()
		{ return day; }
	void increment(int num_minutes);// Increment clock.
	virtual void handle_event(unsigned long curtime, long udata);
	void fake_next_period();	// For debugging.
	};

#endif	/* INCL_GAMECLK */
