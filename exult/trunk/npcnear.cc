/**
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
	timeval curtime,		// Current time.
	Npc_actor *npc,
	int additional_secs		// More secs. to wait.
	)
	{
					// Wait between 5 & 15 secs.
	int msecs = (rand() % 10000) + 5000;
	timeval newtime = Add_usecs(curtime, msecs*1000);
	newtime.tv_sec += additional_secs;
	gwin->get_tqueue()->add(newtime, this, (long) npc);
	}

/*
 *	Run proximity usecode function for the NPC now.
 */

void Npc_proximity_handler::handle_event
	(
	timeval curtime,
	long udata
	)
	{
	Npc_actor *npc = (Npc_actor *) udata;
					// See if still on visible screen.
	Rectangle vchunks(gwin->get_chunkx(), gwin->get_chunky(),
		(gwin->get_width() + (chunksize - 1))/chunksize,
		(gwin->get_height() + (chunksize - 1))/chunksize);
					// No longer visible?
	if (!vchunks.has_point(npc->get_cx(), npc->get_cy()))
		{
		npc->clear_nearby();
		return;
		}
	gwin->get_usecode()->call_usecode(npc->get_usecode(), npc,
					Usecode_machine::npc_proximity);
	add(curtime, npc, 5);		// Add back for next time.
	}
