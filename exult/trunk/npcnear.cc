/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npcnear.cc - At random times, run proximity usecode funs. on nearby
 **		NPC's.
 **
 **	Written: 2/17/00 - JSF
 **/

#include <stdlib.h>
#include "npcnear.h"
#include "gamewin.h"
#include "usecode.h"

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
					// Wait between 5 & 20 secs.
	int msecs = (rand() % 15000) + 5000;
	unsigned long newtime = curtime + msecs;
	newtime += 1000*additional_secs;
	gwin->get_tqueue()->add(newtime, this, (long) npc);
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
					// See if still on visible screen.
	Rectangle tiles = gwin->get_win_tile_rect();
	int tx, ty, tz;
	npc->get_abs_tile(tx, ty, tz);
	if (!tiles.has_point(tx, ty))	// No longer visible?
		{
		npc->clear_nearby();
		return;
		}
	if (!(curtime < wait_until))
		gwin->get_usecode()->call_usecode(npc->get_usecode(), npc,
					Usecode_machine::npc_proximity);
	add(curtime, npc, 3);		// Add back for next time.
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

