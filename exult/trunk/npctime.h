/*
 *	Npctime.h - Timed-even handlers for NPC's.
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef NPCTIME_H
#define	NPCTIME_H	1

#include "tqueue.h"

class Game_window;
class Actor;
class Npc_hunger_timer;
class Npc_poison_timer;
class Npc_sleep_timer;
class Npc_invisibility_timer;
class Npc_protection_timer;
class Npc_might_timer;

/*
 *	List of references to timers for an NPC.
 */
class Npc_timer_list
	{
	Actor *npc;
	Npc_hunger_timer *hunger;
	Npc_poison_timer *poison;
	Npc_sleep_timer *sleep;
	Npc_invisibility_timer *invisibility;
	Npc_protection_timer *protection;
	Npc_might_timer *might;
public:
	friend class Npc_hunger_timer;
	friend class Npc_poison_timer;
	friend class Npc_sleep_timer;
	friend class Npc_invisibility_timer;
	friend class Npc_protection_timer;
	friend class Npc_might_timer;
	Npc_timer_list(Actor *n) : npc(n), hunger(0), poison(0), sleep(0),
				invisibility(0), protection(0), might(0)
		{  }
	~Npc_timer_list();
	void start_hunger();
	void start_poison();
	void start_sleep();
	void start_invisibility();
	void start_protection();
	void start_might();
	};

#endif	/* INCL_NPCTIME */
