/**
 **	Items.h - Names of items.
 **
 **	Written: 12/3/98 - JSF
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

#ifndef INCL_ITEMS
#define INCL_ITEMS 1

#ifndef ALPHA_LINUX_CXX
#  include	<iosfwd>
#endif

extern char **item_names;		// The game items' names.
extern int num_item_names;

void Setup_item_names (std::ifstream& items);

/*
 *	Some offsets in text.flx:
 */
const int first_move_aside = 0x400;	// For guards when blocked.
const int last_move_aside = 0x402;
const int first_ouch = 0x429;
const int last_ouch = 0x42c;
const int first_need_help = 0x430;
const int last_need_help = 0x433;
const int first_will_help = 0x434;
const int last_will_help = 0x436;
const int first_to_battle = 0x439;
const int last_to_battle = 0x43b;
const int first_flee = 0x448;
const int last_flee = 0x44e;
const int first_lamp_on = 0x463;
const int last_lamp_on = 0x466;
const int lamp_off = 0x467;
const int first_call_police = 0x469;
const int last_call_police = 0x46d;
const int first_call_guards = 0x46c;
const int last_call_guards = 0x46d;
const int first_theft = 0x46e;		// Warnings.
const int last_theft = 0x470;
const int first_close_shutters = 0x471;
const int last_close_shutters = 0x473;
const int first_open_shutters = 0x474;
const int last_open_shutters = 0x476;
const int first_hunger = 0x477;		// A little hungry.  (3 of each).
const int first_needfood = 0x47a;	// Must have food.
const int first_starving = 0x47b;	// Starving.
const int first_awakened = 0x495;
const int last_awakened = 0x49a;

#endif





