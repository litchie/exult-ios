/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Gameclk.cc - Keep track of time.
 **
 **	Written: 2/16/00 - JSF
 **/

#include <iostream.h>			/* Debugging. */
#include "gameclk.h"
#include "gamewin.h"
#include "actors.h"
#include "usecode.h"

/*
 *	Palette #'s in 'palettes.flx'.  Just need them here for now.
	++++++Need to verify #'s.
 */
const int PALETTE_DAY = 0;
const int PALETTE_DUSK = 1;
const int PALETTE_NIGHT = 2;
const int PALETTE_DAWN = 1;		// Think this is it.

/*
 *	Set palette.
 */

void Game_clock::set_time_palette
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int new_palette;
	if (in_dungeon || hour < 5)
		new_palette = PALETTE_NIGHT;
	else if (hour < 6)
		new_palette = PALETTE_DAWN;
	else if (hour < 19)
		new_palette = PALETTE_DAY;
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
 *	Decrement food level and check hunger of the party members.
 */

void Game_clock::check_hunger
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Usecode_machine *uc = gwin->get_usecode();
	int cnt = uc->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		Npc_actor *npc = (Npc_actor *) gwin->get_npc(
						uc->get_party_member(i));
		npc->use_food();
		}
					// Player needs to eat too.
	gwin->get_main_actor()->use_food();
	}

/*
 *	Update palette according to whether we're in a dungeon.
 */

void Game_clock::set_in_dungeon
	(
	int tf				// 1 if yes, else 0.
 	)
	{
	in_dungeon = tf;
	set_time_palette();
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
	num_minutes += time_factor/2;	// Round to nearest 15 minutes.
	num_minutes -= num_minutes%time_factor;
	long new_min = minute + num_minutes;
	hour += new_min/60;		// Update hour.
	minute = new_min%60;
	day += hour/24;			// Update day.
	hour %= 24;
					// Update palette to new time.
	set_time_palette();
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
	static int first_day = 1, first_hour_passed = 0;
	Game_window *gwin = (Game_window *) udata;
	if ((minute += time_factor) >= 60)// 1 real minute = 15 game minutes.
		{
		minute -= 60;
		first_hour_passed = 1;
		if (++hour >= 24)
			{
			hour -= 24;
			day++;
			}
		set_time_palette();
		if (hour%3 == 0)	// New 3-hour period?
			{
			check_hunger();	// Use food, and print complaints.
					// Update NPC schedules.
			gwin->schedule_npcs(hour/3);
			}
		}
	else if (first_day &&		// Set 6am schedules after start.
		 (first_hour_passed || gwin->get_usecode()->get_global_flag(
					Usecode_machine::found_stable_key)))
		{
		first_day = 0;
		gwin->schedule_npcs(hour/3);
		}
	cout << "Clock updated to " << hour << ':' << minute << endl;
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
	set_time_palette();
	check_hunger();
	gwin->schedule_npcs(hour/3);
	cout << "The hour is now " << hour << endl;
	}

