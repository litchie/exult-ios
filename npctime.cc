/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npctime.cc - Timed-even handlers for NPC's.
 **
 **	Written: 8/4/00 - JSF
 **/

#include "npctime.h"
#include "gamewin.h"
#include "actors.h"
#include "items.h"

/*
 *	Get game minute from start.
 */

unsigned long Npc_timer::get_minute
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	return 60*gwin->get_total_hours() + gwin->get_minute();
	}

/*
 *	Start timer.
 */

Npc_timer::Npc_timer
	(
	Actor *n
	) : npc(n)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Start in 10 secs.
	gwin->get_tqueue()->add(SDL_GetTicks() + 10000, this, 0L);
	}

/*
 *	Done.
 */

Npc_timer::~Npc_timer
	(
	)
	{
	// ++++++Need this: npc->remove_timer(this);
	}

/*
 *	Time to penalize for hunger.
 */

void Npc_hunger_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// No longer a party member?
	if ((npc != gwin->get_main_actor() && npc->get_party_id() < 0) ||
					//   or no longer hungry?
	    npc->get_property((int) Actor::food_level) >= 0)
		{
		delete this;
		return;
		}
	unsigned long minute = get_minute();
					// Once/hour.
	if (minute >= last_time + 60)
		{
		int health = npc->get_property((int) Actor::health);
		health -= rand()%3;
		npc->set_property((int) Actor::health, health);
		if (rand()%4)
			npc->say(first_starving, first_starving + 2);
		last_time = minute;
		}
	gwin->get_tqueue()->add(curtime + 30000, this, 0L);
	}

/*
 *	Initialize poison timer.
 */

Npc_poison_timer::Npc_poison_timer
	(
	Actor *n
	) : Npc_timer(n)
	{
	last_time = get_minute();	// Lasts 1-2 hours.
	end_time = last_time + 60 + rand()%60;
	}

/*
 *	Time to penalize for poison, or see if it's worn off.
 */

void Npc_poison_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	unsigned long minute = get_minute();
	if (minute >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Actor::poisoned) == 0)
		{
		npc->clear_flag(Actor::poisoned);
		delete this;
		return;
		}
					// Once per 20 minutes.
	if (minute >= last_time + 20)
		{
		int health = npc->get_property((int) Actor::health);
		health -= rand()%3;
		npc->set_property((int) Actor::health, health);
		if (rand()%4)
			npc->say(first_ouch, last_ouch);
		last_time = minute;
		}
					// Check again in 20 secs.
	gwin->get_tqueue()->add(curtime + 20000, this, 0L);
	}

