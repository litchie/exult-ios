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

#ifndef FLAGS_H
#define FLAGS_H


namespace  Obj_flags
{
	enum {		// Bit #'s of flags:
		invisible = 0,
		asleep = 1,
		charmed = 2,
		cursed = 3,
		dead = 4,
		in_party = 6,		// Guess, appears to be correct
		paralyzed = 7,
		poisoned = 8,
		protection = 9,
		on_moving_barge = 10,	// ??Guessing.
		okay_to_take = 11,	// Okay to take??
		might = 12,		// Double strength, dext, intel.
		cant_die = 14,		// Test flag in Monster_info.
		dancing = 15,		// ??Not sure.
		dont_render = 16,	// Completely invisible.
		si_on_moving_barge = 17,// SI's version of 10?
		is_temporary = 18,	// Is temporary
		okay_to_land = 21,	// Used for flying-carpet.
		in_dungeon = 23,	// Pretty sure.  If set, you won't
					//   be accused of stealing food.
		confused = 25,		// ??Guessing.
		in_motion = 26,		// ??Guessing (cart, boat)??
		met = 28,			// Has the npc been met
		si_tournament = 29,	// SI-Call usecode (eventid=7)
		si_zombie = 30,		// Used for sick Neyobi.
		// Flags > 31
		polymorph = 32,		// SI.  Pretty sure about this.
		tattooed = 33,			// Guess (SI).
		read = 34,			// Guess (SI).
		petra = 35,			// Guess
		freeze = 37		// SI.  Pretty sure.
	};
}

#endif
