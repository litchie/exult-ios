/**
 **	Gameclk.cc - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#include <iostream.h>			/* Debugging. */
#include "gameclk.h"
#include "gamewin.h"

/*
 *	Animation.
 */

void Game_clock::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->game window.
	)
	{
	if ((minute += 12) >= 60)	// 1 real minute = 12 game minutes.
		{
		minute -= 60;
		if (++hour >= 24)
			{
			hour -= 24;
			day++;
			}
		if (hour%3 == 0)	// New 3-hour period?
			{		// Update NPC schedules.
			Game_window *gwin = (Game_window *) udata;
			gwin->schedule_npcs(hour/3);
			}
		}
	cout << "Clock updated to " << hour << ':' << minute << '\n';
	curtime += 60*1000;		// Do it again in 60 seconds.
	tqueue->add(curtime, this, udata);
	}
