/**
 **	Npcnear.h - At random times, run proximity usecode funs. on nearby
 **		NPC's.
 **
 **	Written: 2/17/00 - JSF
 **/

#ifndef INCL_NPCNEAR
#define	INCL_NPCNEAR	1

#include "tqueue.h"

class Game_window;
class Npc_actor;

/*
 *	This class keeps track of NPC's nearby, and randomly runs the Usecode
 *	proximity functions for them.
 */
class Npc_proximity_handler : public Time_sensitive
	{
	Game_window *gwin;
public:
	Npc_proximity_handler(Game_window *gw) : gwin(gw)
		{  }
					// Add npc to queue.
	void add(timeval curtime, Npc_actor *npc,
					int additional_secs = 0);
					// Run usecode function.
	void handle_event(timeval curtime, long udata);
	};

#endif	/* INCL_NPCNEAR */
