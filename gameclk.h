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
	Game_clock(Time_queue *tq) : tqueue(tq), hour(12), minute(0), day(0)
		{  }
	int get_hour()
		{ return hour; }
	int get_minute()
		{ return minute; }
	int get_day()
		{ return day; }
	virtual void handle_event(timeval curtime, long udata);
	};

#endif	/* INCL_GAMECLK */
