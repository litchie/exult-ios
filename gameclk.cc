/**
 **	Gameclk.cc - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#include <iostream.h>			/* Debugging. */
#include "gameclk.h"

/*
 *	Animation.
 */

void Game_clock::activate
	(
	timeval curtime,		// Current time of day.
	long udata			// Ignored.
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
		}
	cout << "Clock updated to " << hour << ':' << minute << '\n';
	curtime.tv_sec += 60;		// Do it again in 60 seconds.
	tqueue->add(curtime, this, 0L);
	}
