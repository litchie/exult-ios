/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Combat.h - Combat scheduling.
 **
 **	Written: 6/20/2000 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INCL_COMBAT
#define INCL_COMBAT	1

#include "schedule.h"
#include "lists.h"

class Actor;
class Chunk_object_list;

/*
 *	Combat schedule:
 */
class Combat_schedule : public Schedule
	{
	enum Phase			// We'll be a finite-state-machine.
		{
		approach = 0,		// Approaching a foe to attack.
		retreat = 1,		// Avoiding a foe.
		flee = 2,		// Run away!
		strike = 3,		// In the process of striking.
		parry = 4,		// In the process of parrying a blow.
		stunned = 5		// Just been hit.
		} state;
	Slist opponents;		// Possible opponents.
	Game_object *opponent;		// Current opponent.
	int max_reach;			// Max. weapon reach in tiles.
	unsigned char yelled;		// Yell when first opponent targeted.
	int failures;			// # failures to find opponent.
#if 0	/* ++++Going away. */
					// Find monsters, opponents.
	void find_monsters(Chunk_object_list *chunk);
#endif
	void find_opponents();
	Actor *find_foe(int mode);	// Find a new opponent.
	Actor *find_foe();
	void approach_foe();		// Approach foe.
	void start_strike();		// Start to hit.
public:
	Combat_schedule(Actor *n) : Schedule(n), state(approach), 
			opponent(0),
			max_reach(1), yelled(0), failures(0)
		{  }
	virtual void now_what();	// Npc calls this when it's done
					// Set opponent in combat.
	virtual void set_opponent(Game_object *obj);
	};

#endif
