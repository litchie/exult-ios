/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npctime.h - Timed-even handlers for NPC's.
 **
 **	Written: 8/4/00 - JSF
 **/

#ifndef INCL_NPCTIME
#define	INCL_NPCTIME	1

#include "tqueue.h"

class Game_window;
class Actor;
class Npc_hunger_timer;
class Npc_poison_timer;
class Npc_sleep_timer;

/*
 *	List of references to timers for an NPC.
 */
class Npc_timer_list
	{
	Actor *npc;
	Npc_hunger_timer *hunger;
	Npc_poison_timer *poison;
	Npc_sleep_timer *sleep;
public:
	friend class Npc_hunger_timer;
	friend class Npc_poison_timer;
	friend class Npc_sleep_timer;
	Npc_timer_list(Actor *n) : npc(n), hunger(0), poison(0), sleep(0)
		{  }
	~Npc_timer_list();
	void start_hunger();
	void start_poison();
	void start_sleep();
	};

#endif	/* INCL_NPCTIME */
