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

/*
 *	Base class for keeping track of things like poison, protection, hunger.
 */
class Npc_timer : public Time_sensitive
	{
protected:
	Actor *npc;			// Character affected.
	unsigned long get_minute();	// Get game minutes.
public:
	Npc_timer(Actor *n);
	virtual ~Npc_timer();
	};

/*
 *	Handle starvation.
 */
class Npc_hunger_timer : public Npc_timer
	{
	unsigned long last_time;	// Last game minute when penalized.
public:
	Npc_hunger_timer(Actor *n) : Npc_timer(n)
		{ last_time = get_minute(); }
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Handle poison.
 */
class Npc_poison_timer : public Npc_timer
	{
	unsigned long end_time;		// Time when it wears off.
	unsigned long last_time;	// Last game minute when penalized.
public:
	Npc_poison_timer(Actor *n);
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

#endif	/* INCL_NPCTIME */
