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
extern char **text_msgs;		// Msgs. (0x400 - ).
extern int num_text_msgs;
extern char **misc_names;		// Frames, etc (0x500 - ).
extern int num_misc_names;

void Setup_text();
void Free_text();
void Write_text_file();
void Set_item_name(int num, const char *name);

/*
 *	Message #'s.  These are (offset-0x400) in text.flx and exultmsg.txt:
 */
const int first_move_aside = 0x00;	// For guards when blocked.
const int last_move_aside = 0x02;
const int first_preach = 0x03, last_preach = 0x07;
const int first_preach2 = 0x08, last_preach2 = 0x0b;
const int first_amen = 0x0c, last_amen = 0x0f;
const int first_thief = 0x10, last_thief = 0x13;
const int first_waiter_ask = 0x1b, last_waiter_ask = 0x1f;
const int first_more_food = 0x20, last_more_food = 0x24;
const int first_munch = 0x25, last_munch = 0x28;
const int first_ouch = 0x29;
const int last_ouch = 0x2c;
const int first_need_help = 0x30;
const int last_need_help = 0x33;
const int first_will_help = 0x34;
const int last_will_help = 0x36;
const int first_to_battle = 0x39;
const int last_to_battle = 0x3b;
const int first_farmer = 0x3f, last_farmer = 0x41;
const int first_miner = 0x42, last_miner = 0x44;
const int first_flee = 0x48;
const int last_flee = 0x4e;
const int first_lamp_on = 0x63;
const int last_lamp_on = 0x66;
const int lamp_off = 0x67;
const int first_call_police = 0x69;
const int last_call_police = 0x6d;
const int first_call_guards = 0x6c;
const int last_call_guards = 0x6d;
const int first_theft = 0x6e;		// Warnings.
const int last_theft = 0x70;
const int first_close_shutters = 0x71;
const int last_close_shutters = 0x73;
const int first_open_shutters = 0x74;
const int last_open_shutters = 0x76;
const int first_hunger = 0x77;		// A little hungry.  (3 of each).
const int first_needfood = 0x7a;	// Must have food.
const int first_starving = 0x7b;	// Starving.
const int heard_something = 0x95;
const int first_awakened = 0x95;
const int last_awakened = 0x9a;
const int magebane_struck = 0x9b;	// (SI only).

//	Messages in exultmsg.txt ( - 0x400):
const int first_chair_thief = 0x100, last_chair_thief = 0x104;
const int first_waiter_banter = 0x105, last_waiter_banter = 0x107;
const int first_waiter_serve = 0x108, last_waiter_serve = 0x109;
const int first_bed_occupied = 0x10a, num_bed_occupied = 3;
const int first_catchup = 0x10d, last_catchup = 0x10f;
const int with_help_from = 0x110, exult_team = 0x111, driven_by_exult = 0x112;
const int end_of_ultima7 = 0x113, end_of_britannia = 0x114;
const int you_cannot_do_that = 0x115, damn_avatar = 0x116;
const int blackgate_destroyed = 0x117, guardian_has_stopped = 0x118;
const int txt_screen0 = 0x119; //to 0x11E
const int txt_screen1 = 0x11F; //to 0x128
const int txt_screen2 = 0x129; //to 0x12E
const int txt_screen3 = 0x12F; //to 0x134
const int txt_screen4 = 0x135; //to 0x138
const int lord_castle = 0x139, dick_castle = 0x13A;
const int bg_fellow = 0x13B; //to 0x13D
const int my_leige = 0x13E, yo_homes = 0x53F;
const int all_we0 = 0x140; //to 0x541
const int and_a0 = 0x142; //to 0x543
const int indeed = 0x144; //to 0x545
const int iree = 0x146;
const int stand_back = 0x147;
const int jump_back = 0x148;
const int batlin = 0x149;
const int you_shall = 0x14B;
const int there_i = 0x14D;
const int batlin2 = 0x14F;
const int you_must = 0x151;
const int soon_i = 0x153;
const int tis_my = 0x155;

//	Misc. text (frames, etc.) start at 0x500 in text.flx.
const int misc_name0 = 0x500;

#endif





