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
	if (hour < 5)
		gwin->set_palette(PALETTE_NIGHT);
	else if (hour < 6)
		gwin->set_palette(PALETTE_DAWN);
	else if (hour < 19)
		gwin->set_palette(PALETTE_DAY);
	else if (hour < 21)
		gwin->set_palette(PALETTE_DUSK);
	else
		gwin->set_palette(PALETTE_NIGHT);
	}

/*
 *	Set palette.  Used for restoring a game.
 */

void Game_clock::set_palette
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Set_palette(gwin, hour);	// Update palette to new time.
	}

/*
 *	Increment clock.
 */

void Game_clock::increment
	(
	int num_minutes			// # of minutes to increment.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int old_3hour = hour/3;		// Remember current 3-hour period.
	num_minutes += 6;		// Round to nearest 12 minutes.
	num_minutes -= num_minutes%12;
	long new_min = minute + num_minutes;
	hour += new_min/60;		// Update hour.
	minute = new_min%60;
	day += hour/24;			// Update day.
	hour %= 24;
	Set_palette(gwin, hour);	// Update palette to new time.
	int new_3hour = hour/3;		// New 3-hour period.
	if (new_3hour != old_3hour)	// In a new period?
					// Update NPC schedules.
		gwin->schedule_npcs(new_3hour);
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
	static int first_day = 1;
	Game_window *gwin = (Game_window *) udata;
	if ((minute += 12) >= 60)	// 1 real minute = 12 game minutes.
		{
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
#if 1
	else if (first_day &&		// Set 6am schedules after start.
		 gwin->get_usecode()->get_global_flag(
					Usecode_machine::found_stable_key))
		{
		first_day = 0;
		gwin->schedule_npcs(hour/3);
		}
#endif
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
