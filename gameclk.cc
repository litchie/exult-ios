/*
 *	gameclk.cc - Keep track of time.
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>			/* Debugging. */
#include "gameclk.h"
#include "gamewin.h"
#include "actors.h"
#include "cheat.h"
#include "game.h"

/*
 *	Set palette.
 */

void Game_clock::set_time_palette
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Actor *main_actor = gwin->get_main_actor();
	int new_palette;
	if (main_actor && main_actor->get_flag(Obj_flags::invisible))
	{
		gwin->set_palette(PALETTE_INVISIBLE);
		return;
		}

	if (cheat.in_infravision()) {
		gwin->set_palette(PALETTE_DAY);
		return;
	}

	if (gwin->is_in_dungeon() || hour < 5)
		new_palette = PALETTE_NIGHT;
	else if (hour < 6)
		new_palette = PALETTE_DAWN;
	else if (hour < 19)
		new_palette = storm <= 0 ? PALETTE_DAY : PALETTE_DUSK;
	else if (hour < 21)
		new_palette = PALETTE_DUSK;
	else
		new_palette = PALETTE_NIGHT;
	if (light_source_level)
		{
		if (new_palette == PALETTE_NIGHT)
			new_palette = light_source_level == 1 ? PALETTE_DUSK
							: PALETTE_DAWN;
		else if (new_palette == PALETTE_DUSK)
			new_palette = PALETTE_DAWN;
		}
					// Gump mode, or light spell?
	if (gwin->is_special_light() && new_palette == PALETTE_NIGHT)
		new_palette = PALETTE_DAWN;
	gwin->set_palette(new_palette);
	}

/*
 *	Set palette.  Used for restoring a game.
 */

void Game_clock::set_palette
	(
	)
{
	// Update palette to new time.
	set_time_palette();
}

/*
 *	Set the palette for a changed light source level.
 */

void Game_clock::set_light_source_level
	(
	int lev
	)
{
	light_source_level = lev;
	set_time_palette();
}

/*
 *	Storm ending/starting.
 */

void Game_clock::set_storm
	(
	bool onoff
	)
{
	storm += (onoff ? 1 : -1);
	set_time_palette();		// Update palette.
}

/*
 *	Decrement food level and check hunger of the party members.
 */

void Game_clock::check_hunger
	(
	)
{
	Game_window *gwin = Game_window::get_instance();
	Actor *party[9];		// Get party + Avatar.
	int cnt = gwin->get_party(party, 1);
	for (int i = 0; i < cnt; i++)
		party[i]->use_food();
}

static void Check_freezing
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
					// Avatar's flag applies to party.
	bool freeze = gwin->get_main_actor()->get_flag(Obj_flags::freeze)!=false;
	Actor *party[9];		// Get party + Avatar.
	int cnt = gwin->get_party(party, 1);
	for (int i = 0; i < cnt; i++)
		party[i]->check_temperature(freeze);
	}

/*
 *	Increment clock.
 *
 *	FOR NOW, skipping call to mend_npcs().  Not sure...
 */

void Game_clock::increment
	(
	int num_minutes			// # of minutes to increment.
	)
{
	Game_window *gwin = Game_window::get_instance();
	int new_3hour, old_3hour, delta_3hour;
	long new_min;
	
	old_3hour = hour/3;		// Remember current 3-hour period.
	num_minutes += time_factor/2;	// Round to nearest 15 minutes.
	num_minutes -= num_minutes%time_factor;
	new_min = minute + num_minutes;
	hour += new_min/60;		// Update hour.
	minute = new_min%60;
	day += hour/24;			// Update day.
	hour %= 24;
	
	// Update palette to new time.
	set_time_palette();
	new_3hour = hour/3;		// New 3-hour period.
	delta_3hour = new_3hour - old_3hour;
	if (delta_3hour != 0)		// In a new period?
	{			// Update NPC schedules.
		if (Game::get_game_type() == SERPENT_ISLE)
			delta_3hour = 8;
		gwin->schedule_npcs(new_3hour, (delta_3hour +7)%8);
	}
}

/*
 *	Advance clock.
 */

void Game_clock::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->game window.
	)
{
	Game_window *gwin = (Game_window *) udata;

	int min_old = minute;
	int hour_old = hour;
				// Time stopped?  Don't advance.
	if (!gwin->is_time_stopped())
		{
		minute += time_rate;
		if (Game::get_game_type() == SERPENT_ISLE)
			Check_freezing();
		}

	while (minute >= 60)	// advance to the correct hour (and day)
	{
		minute -= 60;
		if (++hour >= 24)
		{
			hour -= 24;
			day++;
		}
		set_time_palette();
		gwin->mend_npcs();	// Restore HP's each hour.
		if (hour%3 == 0)	// New 3-hour period?
		{
			check_hunger();	// Use food, and print complaints.
					// Update NPC schedules.
			gwin->schedule_npcs(hour/3);
		}
	}
	if ((hour != hour_old) || (minute/15 != min_old/15))
		COUT("Clock updated to " << hour << ':' << minute);
	curtime += 60*1000/time_factor;		// 15 changes per minute
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
	hour = ((hour/3 + 1)*3);
	day += hour/24;			// Update day.
	hour %= 24;
	Game_window *gwin = Game_window::get_instance();
	set_time_palette();
	check_hunger();
	gwin->schedule_npcs(hour/3);
	gwin->mend_npcs();		// Just do it once, cheater.
	COUT("The hour is now " << hour);
}

