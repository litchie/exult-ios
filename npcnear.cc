/*
 *	npcnear.cc - At random times, run proximity usecode functions on nearby NPC's.
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
#include "cheat.h"

#include "SDL_timer.h"

#ifndef UNDER_CE
using std::rand;
#endif

bool Bg_dont_wake(Game_window *gwin, Actor *npc);

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
	int msecs;			// Hostile?  Wait 0-2 secs.
	if (npc->get_alignment() >= Npc_actor::hostile)
		msecs = rand() % 2000;
	else				// Wait between 2 & 6 secs.
		msecs = (rand() % 4000) + 2000;
	unsigned long newtime = curtime + msecs;
	newtime += 1000*additional_secs;
	gwin->get_tqueue()->add(newtime, this, reinterpret_cast<long>(npc));
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
	gwin->get_tqueue()->remove(this, reinterpret_cast<long>(npc)); 
	}

/*
 *	Is this a Black Gate (Skara Brae) ghost, or Penumbra?
 */

bool Bg_dont_wake
	(
	Game_window *gwin,
	Actor *npc
	)
	{
	int num;
	return (Game::get_game_type() == BLACK_GATE &&
		(npc->get_info().has_translucency() ||
					// Horace or Penumbra?
		 (num = npc->Actor::get_npc_num()) == 141 || num == 150));
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
	Tile_coord t = npc->get_tile();
	if (!tiles.has_point(t.tx, t.ty) ||	// No longer visible?
	    npc->is_dead())		// Or no longer living?
		{
		npc->clear_nearby();
		return;
		}
	Schedule::Schedule_types sched = (Schedule::Schedule_types)
						npc->get_schedule_type();
					// Sleep schedule?
	if (npc->get_schedule() &&
	    sched == Schedule::sleep &&
					// But not under a sleep spell?
	    !npc->get_flag(Obj_flags::asleep) &&
	    gwin->is_main_actor_inside() &&
	    !Bg_dont_wake(gwin, npc) &&
	    npc->distance(gwin->get_main_actor()) < 6 && rand()%3 != 0)
		{
					// Trick:  Stand, but stay in
					//   sleep_schedule.
		npc->get_schedule()->ending(Schedule::stand);
		npc->say(first_awakened, last_awakened);
					// In 10 seconds, go back to sleep.
		npc->start(0, 10000);
		extra_delay = 11;	// And don't run Usecode while up.
		}
			
	else if (!(curtime < wait_until) && !cheat.in_map_editor() && 
					// Do it 50% of the time OR if
					//   a rabbit (SI start).
		 (rand()%2 == 1 || npc->get_shapenum() == 811)  &&
					// And not for party members.
			!npc->is_in_party() &&
					// And not if walking to sched. spot.
		 sched != Schedule::walk_to_schedule &&
					// And not for patrollers/monsters
					//  in SI. !!Guessing.
		 (Game::get_game_type() != SERPENT_ISLE ||
			(sched != Schedule::patrol &&
			 sched != Schedule::wait &&
			 sched != Schedule::horiz_pace &&
			 sched != Schedule::vert_pace &&
			 !npc->is_monster())))
				
		{
		int ucfun = npc->get_usecode();
		gwin->get_usecode()->call_usecode(ucfun, npc,
					Usecode_machine::npc_proximity);
		extra_delay += 3;
		curtime = Game::get_ticks();// Time may have passed.
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
	wait_until = Game::get_ticks() + 1000*secs;
	}

/*
 *	Fill a list with all the nearby NPC's.
 */

void Npc_proximity_handler::get_all
	(
	Actor_queue& alist			// They're appended to this.
	)
	{
	Time_queue_iterator next(gwin->get_tqueue(), this);
	Time_sensitive *obj;
	long data;			// NPC is the data.
	while (next(obj, data))
		alist.push((Npc_actor *) data);
	}
