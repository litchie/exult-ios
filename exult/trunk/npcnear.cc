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
	Npc_actor *npc
	)
	{
					// Wait between 1 & 4 secs.
	int msecs = (rand() % 3000) + 1000;
	gwin->get_tqueue()->add(
			Add_usecs(curtime, msecs*1000), this, (long) npc);
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
	if (!vchunks.has_point(npc->get_cx(), npc->get_cy()))
		return;
	gwin->get_usecode()->call_usecode(npc->get_usecode(), npc,
					Usecode_machine::npc_proximity);
	add(curtime, npc);		// Add back for next time.
	}
