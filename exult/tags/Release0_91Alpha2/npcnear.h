/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npcnear.h - At random times, run proximity usecode funs. on nearby
 **		NPC's.
 **
 **	Written: 2/17/00 - JSF
 **/

#ifndef INCL_NPCNEAR
#define	INCL_NPCNEAR	1

#include "lists.h"
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
	unsigned long wait_until;	// Skip running usecodes until past.
public:
	Npc_proximity_handler(Game_window *gw) : gwin(gw)
		{
		wait_until = 0;
		}
					// Add npc to queue.
	void add(unsigned long curtime, Npc_actor *npc,
					int additional_secs = 0);
					// Run usecode function.
	void handle_event(unsigned long curtime, long udata);
					// Wait before running more funs.
	void wait(int secs);
	void get_all(Actor_queue& list);	// Fill list with nearby NPC's.
	};

#endif	/* INCL_NPCNEAR */
