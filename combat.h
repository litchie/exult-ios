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

/*
 *	Combat schedule:
 */
class Combat_schedule : public Schedule
	{
protected:
	static unsigned long battle_time;// Time when battle started.
	enum Phase			// We'll be a finite-state-machine.
		{
		initial = 0,		// Just constructed.
		approach = 1,		// Approaching a foe to attack.
		retreat = 2,		// Avoiding a foe.
		flee = 3,		// Run away!
		strike = 4,		// In the process of striking.
		fire = 5,		// In process of firing range weapon.
		parry = 6,		// In the process of parrying a blow.
		stunned = 7		// Just been hit.
		} state;
	Schedule_types prev_schedule;	// Before going into combat.
	Actor_queue opponents;		// Possible opponents.
	Game_object *opponent;		// Current opponent.
	int weapon_shape;		// Weapon's shape in shapes.vga.
	int max_reach;			// Max. weapon reach in tiles.
	int ammo_shape;			// If required, else 0.
	unsigned char ammo_consumed;	// Does ammo get used up.
	unsigned char yelled;		// Yell when first opponent targeted.
	unsigned char started_battle;	// 1st opponent targeted.
	unsigned char fleed;		// Set 1st time fleeing.
	int failures;			// # failures to find opponent.
	virtual void find_opponents();
	Actor *find_foe(int mode);	// Find a new opponent.
	Actor *find_foe();
	void approach_foe();		// Approach foe.
	void start_strike();		// Start to hit.
	void set_weapon_info();		// Set 'max_reach' of weapon.
public:
	Combat_schedule(Actor *n, Schedule_types prev_sched) 
		: Schedule(n), state(initial), prev_schedule(prev_sched),
			opponent(0), weapon_shape(0),
			max_reach(1), ammo_shape(0), 
			ammo_consumed(0), yelled(0), 
			started_battle(0), fleed(0), failures(0)
		{ set_weapon_info(); }
	virtual void now_what();	// Npc calls this when it's done
	virtual void im_dormant();	// Npc calls this when it goes dormant.
					// Set opponent in combat.
	virtual void set_opponent(Game_object *obj);
	virtual Game_object *get_opponent();	// Get opponent.
	};

/*
 *	Dueling is like combat, but nobody gets hurt.
 */

class Duel_schedule : public Combat_schedule
	{
	Tile_coord start;		// Starting position.
	int attacks;			// Count strikes.
	virtual void find_opponents();
public:
	Duel_schedule(Actor *n);
	virtual void now_what();
	};

#endif
