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
 *  GNU General Public License for more details.
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

static inline bool is_light_palette(int pal)
	{
	return (pal == PALETTE_SINGLE_LIGHT || pal == PALETTE_MANY_LIGHTS);
	}

static inline bool is_dark_palette(int pal)
	{
	return (pal == PALETTE_DUSK || pal == PALETTE_NIGHT);
	}

static inline bool is_weather_palette(int pal)
	{
	return (pal == PALETTE_OVERCAST || pal == PALETTE_FOG);
	}

static inline bool is_day_palette(int pal)
	{
	return (pal == PALETTE_DAWN || pal == PALETTE_DAY);
	}

static inline int get_time_palette(int hour, bool dungeon)
	{
	if (dungeon || hour < 6)
		return PALETTE_NIGHT;
	else if (hour == 6)
		return PALETTE_DAWN;
	else if (hour == 7)
		return PALETTE_DAY;
	else if (hour < 20)
		return PALETTE_DAY;
	else if (hour == 20)
		return PALETTE_DUSK;
	else
		return PALETTE_NIGHT;
	}

static inline int get_final_palette
	(
	int pal,
	bool cloudy,
	bool foggy,
	int light,
	bool special
	)
	{
	if (light && is_dark_palette(pal))
		{
		int light_palette = (light > 1) + PALETTE_SINGLE_LIGHT;
						// Gump mode, or light spell?
		if (special)
			light_palette = PALETTE_MANY_LIGHTS;

		return light_palette;
		}
	else if (is_day_palette(pal))
		{
		if (foggy)
			return PALETTE_FOG;
		else if (cloudy)
			return PALETTE_OVERCAST;
		}
	return pal;
	}

/*
 *	Set palette.
 */

void Game_clock::set_time_palette
	(
	)
	{
	Game_window *gwin = Game_window::get_instance();
	Actor *main_actor = gwin->get_main_actor();
	if (main_actor && main_actor->get_flag(Obj_flags::invisible))
	{
		gwin->get_pal()->set(PALETTE_INVISIBLE);
		return;
		}

	if (cheat.in_infravision()) {
		gwin->get_pal()->set(PALETTE_DAY);
		return;
	}

	dungeon = gwin->is_in_dungeon();

	int new_palette = get_time_palette(hour+1, dungeon),
		old_palette = get_time_palette(hour, dungeon);
	bool cloudy = overcast > 0;
	bool foggy = fog > 0;
	bool weather_change = (cloudy != was_overcast) || (foggy != was_foggy);
	bool light_change = (light_source_level != old_light_level);
	bool need_new_transition = (weather_change || light_change);

	new_palette = get_final_palette(new_palette, cloudy, foggy,
				light_source_level, gwin->is_special_light());
	old_palette = get_final_palette(old_palette, was_overcast, was_foggy,
				old_light_level, gwin->is_special_light());

	was_overcast = cloudy;
	was_foggy = foggy;
	old_light_level = light_source_level;

	if (weather_change)
		{	// TODO: Maybe implement smoother transition from
			// weather to/from dawn/sunrise/sundown/dusk.
			// Right now, it works like the original.
		if (transition)
			delete transition;
		transition = new Palette_transition(old_palette, new_palette,
							hour, minute, 1, 4, hour, minute);
		return;
		}
	else if (light_change)
		{
		if (transition)
			{
			delete transition;
			transition = 0;
			}
		gwin->get_pal()->set(new_palette);
		return;
		}
	if (transition)
		{
		if (transition->set_step(hour, minute))
			return;
		delete transition;
		transition = 0;
		}

	if (old_palette != new_palette)	// Do we have a transition?
		{
		transition = new Palette_transition(old_palette, new_palette,
							hour, minute, 4, 15, hour, 0);
		return;
		}

	gwin->get_pal()->set(new_palette);
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
 *	Cloud cover.
 */

void Game_clock::set_overcast
	(
	bool onoff
	)
{
	overcast += (onoff ? 1 : -1);
	set_time_palette();		// Update palette.
}

/*
 *	Fog.
 */

void Game_clock::set_fog
	(
	bool onoff
	)
{
	fog += (onoff ? 1 : -1);
	if (hour < 6 || hour > 20)
		fog = 0;	// Disable fog at night???
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
	int old_hour;
	long new_min;
	
	old_hour = hour;		// Remember current 3-hour period.
	num_minutes += 7;		// Round to nearest 15 minutes.
	num_minutes -= num_minutes%15;
	new_min = minute + num_minutes;
	hour += new_min/60;		// Update hour.
	minute = new_min%60;
	day += hour/24;			// Update day.
	hour %= 24;
	
	// Update palette to new time.
	reset();
	set_time_palette();
	// Check to see if we need to update the NPC schedules.
	if (hour != old_hour)		// Update NPC schedules.
		gwin->schedule_npcs(hour);
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
	if (!gwin->is_time_stopped() && !cheat.in_map_editor())
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
		// Testing.
		//set_time_palette();
		gwin->mend_npcs();	// Restore HP's each hour.
		check_hunger();		// Use food, and print complaints.
		gwin->schedule_npcs(hour);
	}

	if (transition && !transition->set_step(hour, minute))
		{
		delete transition;
		transition = 0;
		set_time_palette();
		}
	else if (hour != hour_old)
		set_time_palette();

	if ((hour != hour_old) || (minute/15 != min_old/15))
		COUT("Clock updated to " << hour << ':' << minute);
	curtime += gwin->get_std_delay()*ticks_per_minute;
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
	reset();
	set_time_palette();
	check_hunger();
	gwin->schedule_npcs(hour);
	gwin->mend_npcs();		// Just do it once, cheater.
	COUT("The hour is now " << hour);
}

