/**
 **	Npcnear.cc - At random times, run proximity usecode funs. on nearby
 **		NPC's.
 **
 **	Written: 2/17/00 - JSF
 **/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#endif
#include "npcnear.h"
#include "gamewin.h"
#include "actors.h"
#include "ucmachine.h"
#include "schedule.h"
#include "items.h"
#include "game.h"

#include "SDL_timer.h"

using std::rand;

/*
 *	Add an npc to the time queue.
 */

void Npc_proximity_handler::add
	(
	unsigned long curtime,		// Current time (msecs).
	Npc_actor *npc,
	int additional_secs		// More secs. to wait.
	)
	{
	int msecs;			// Hostile?  Wait 0-4 secs.
	if (npc->get_alignment() >= Npc_actor::hostile)
		msecs = rand() % 4000;
	else				// Wait between 2 & 6 secs.
		msecs = (rand() % 4000) + 2000;
	unsigned long newtime = curtime + msecs;
	newtime += 1000*additional_secs;
	gwin->get_tqueue()->add(newtime, this, (long) npc);
	}

/*
 *	Remove entry for an npc.
 */

void Npc_proximity_handler::remove
	(
	Npc_actor *npc
	)
	{
	npc->clear_nearby();
	gwin->get_tqueue()->remove(this, (long) npc); 
	}

/*
 *	Run proximity usecode function for the NPC now.
 */

void Npc_proximity_handler::handle_event
	(
	unsigned long curtime,
	long udata
	)
	{
	Npc_actor *npc = (Npc_actor *) udata;
	int extra_delay = 5;		// For next time.
					// See if still on visible screen.
	Rectangle tiles = gwin->get_win_tile_rect().enlarge(10);
	int tx, ty, tz;
	npc->get_abs_tile(tx, ty, tz);
	if (!tiles.has_point(tx, ty) ||	// No longer visible?
	    npc->is_dead())		// Or no longer living?
		{
		npc->clear_nearby();
		return;
		}
					// Sleep schedule?
	if (npc->get_schedule() &&
	    npc->get_schedule_type() == (int) Schedule::sleep &&
					// But not under a sleep spell?
	    !npc->get_flag(Obj_flags::asleep) &&
	    gwin->is_main_actor_inside() &&
	    npc->distance(gwin->get_main_actor()) < 6 && rand()%3 != 0)
		{
					// Trick:  Stand, but stay in
					//   sleep_schedule.
		npc->get_schedule()->ending((int) Schedule::stand);
		npc->say(first_awakened, last_awakened);
		}
					// Hostile monster?  ATTACK!
	else if (npc->get_alignment() == Npc_actor::hostile &&
		npc->is_monster() &&
		npc->get_schedule_type() != (int) Schedule::combat &&
					// jsf-Trying to fix mage in
					// Test of Courage:
		npc->get_schedule_type() != (int) Schedule::wait)
		{
		npc->set_schedule_type(Schedule::combat);
		}
			
	else if (!(curtime < wait_until) && 
					// Do it 50% of the time OR if
					//   a rabbit (SI start).
		 (rand()%2 == 1 || npc->get_shapenum() == 811)  &&
					// And not for party members.
			npc->get_party_id() < 0 &&
					// And not for patrollers/monsters
					//  in SI. !!Guessing.
		 (Game::get_game_type() != SERPENT_ISLE ||
			(npc->get_schedule_type() != (int) Schedule::patrol &&
			 !npc->is_monster())))
				
		{
		int ucfun = npc->get_usecode();
		ucfun = ucfun == -1 ? npc->get_shapenum() : ucfun;
		gwin->get_usecode()->call_usecode(ucfun, npc,
					Usecode_machine::npc_proximity);
		extra_delay += 3;
		curtime = SDL_GetTicks();// Time may have passed.
		}
	add(curtime, npc, extra_delay);	// Add back for next time.
	}

/*
 *	Set a time to wait for before running any usecodes.  This is to
 *	skip all the text events that would happen at the end of a conver-
 *	sation.
 */

void Npc_proximity_handler::wait
	(
	int secs			// # of seconds.
	)
	{
	wait_until = SDL_GetTicks() + 1000*secs;
	}

/*
 *	Fill a list with all the nearby NPC's.
 */

void Npc_proximity_handler::get_all
	(
	Actor_queue& list			// They're appended to this.
	)
	{
	Time_queue_iterator next(gwin->get_tqueue(), this);
	Time_sensitive *obj;
	long data;			// NPC is the data.
	while (next(obj, data))
		list.push((Npc_actor *) data);
	}
