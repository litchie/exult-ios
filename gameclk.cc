/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Gameclk.cc - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#include <iostream.h>			/* Debugging. */
#include "gameclk.h"
#include "gamewin.h"

/*
 *	Palette #'s in 'palettes.flx'.  Just need them here for now.
	++++++Need to verify #'s.
 */
const int PALETTE_DAY = 0;
const int PALETTE_DUSK = 1;
const int PALETTE_NIGHT = 2;
const int PALETTE_DAWN = 3;

/*
 *	Set palette.
 */

inline void Set_palette
	(
	Game_window *gwin,
	int hour
	)
	{
	if (hour == 5)
		gwin->set_palette(PALETTE_DAWN);
	if (hour == 6)
		gwin->set_palette(PALETTE_DAY);
	if (hour == 19)
		gwin->set_palette(PALETTE_DUSK);
	if (hour == 21)
		gwin->set_palette(PALETTE_NIGHT);
	}

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
		Game_window *gwin = (Game_window *) udata;
		minute -= 60;
		if (++hour >= 24)
			{
			hour -= 24;
			day++;
			}
		Set_palette(gwin, hour);
		if (hour%3 == 0)	// New 3-hour period?
					// Update NPC schedules.
			gwin->schedule_npcs(hour/3);
		}
	cout << "Clock updated to " << hour << ':' << minute << '\n';
	curtime += 60*1000;		// Do it again in 60 seconds.
	tqueue->add(curtime, this, udata);
	}

/*
 *	Fake an update to the next 3-hour period.
 */

void Game_clock::fake_next_period
	(
	)
	{
	minute = 0;
	hour = ((hour/3 + 1)*3)%24;
	Game_window *gwin = Game_window::get_game_window();
	Set_palette(gwin, hour);
	gwin->schedule_npcs(hour/3);
	cout << "The hour is now " << hour << '\n';
	}
