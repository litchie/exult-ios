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

class Npc_actor;

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
	Npc_actor *opponent;		// Current opponent.
public:
	Combat_schedule(Actor *n) : Schedule(n), state(approach), opponent(0)
		{  }
	virtual void now_what();	// Npc calls this when it's done
	};

#endif
