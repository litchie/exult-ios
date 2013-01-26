/**
 ** Items.h - Names of items.
 **
 ** Written: 12/3/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman
Copyright (C) 2000? - 2013 The Exult Team

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
#  include  <iosfwd>
#endif

extern char **item_names;       // The game items' names.
extern int num_item_names;
extern char **text_msgs;        // Msgs. (0x400 - ).
extern int num_text_msgs;
extern char **misc_names;       // Frames, etc (0x500 - 0x5ff/0x685 (BG/SI)).
extern int num_misc_names;

void Setup_text(bool si, bool expansion);
void Free_text();
void Write_text_file();
void Set_item_name(int num, const char *name);

/*
 *  Message #'s.  These are (offset-0x400) in text.flx and exultmsg.txt:
 */
// Schedule barks.
const int first_move_aside = 0x00, last_move_aside = 0x02;  // For guards when blocked.
const int first_preach = 0x03, last_preach = 0x07;
const int first_preach2 = 0x08, last_preach2 = 0x0b;
const int first_amen = 0x0c, last_amen = 0x0f;
const int first_thief = 0x10, last_thief = 0x13;
const int first_talk = 0x14, last_talk = 0x16;
const int first_arrest = 0x17, last_arrest = 0x1a;
const int first_waiter_ask = 0x1b, last_waiter_ask = 0x1f;
const int first_more_food = 0x20, last_more_food = 0x24;
const int first_munch = 0x25, last_munch = 0x28;
const int first_farmer = 0x3f, last_farmer = 0x41;
const int first_miner = 0x42, last_miner = 0x44;
const int first_miner_gold = 0x45, last_miner_gold = 0x47;
const int first_farmer2 = 0x60, last_farmer2 = 0x62;
const int first_lamp_on = 0x63, last_lamp_on = 0x66;
const int lamp_off = 0x67;
const int new_candle = 0x68;
const int first_close_shutters = 0x71, last_close_shutters = 0x73;
const int first_open_shutters = 0x74, last_open_shutters = 0x76;
// Combat-related barks.
const int first_ouch = 0x29, last_ouch = 0x2c;
const int first_need_help = 0x30, last_need_help = 0x33;
const int first_will_help = 0x34, last_will_help = 0x36;
const int first_to_battle = 0x39, last_to_battle = 0x3b;
const int flee_screaming = 0x37;
const int first_flee = 0x48, last_flee = 0x52;
const int first_taunt = 0x53, last_taunt = 0x59;
const int goblin_ouch = 0xcc;
const int first_goblin_taunt = 0xcd, last_goblin_taunt = 0xce;
// Goblin barks 0xcf, 0xd0 and 0xd1 probably come here, but how???
const int goblin_need_help = 0xd2;
const int goblin_will_help = 0xd3;
const int goblin_flee_screaming = 0xd4;
const int first_goblin_flee = 0xd5, last_goblin_flee = 0xd9;
const int goblin_to_battle = 0xda;
// Thieving avatar warnings.
const int first_theft = 0x6e, last_theft = 0x70;
const int goblin_theft = 0xcb;
// Invisible thieving avatar warnings.
const int first_invis_theft = 0x85, last_invis_theft = 0x87;
const int goblin_invis_theft = 0xc8;
// Call guards.
const int first_call_guards_theft = 0x3c, last_call_guards_theft = 0x3e;
const int first_call_police = 0x69, last_call_police = 0x6d;
const int first_call_guards = 0x6c, last_call_guards = 0x6d;
const int first_goblin_call_police = 0xc6, last_goblin_call_police = 0xc7;
const int first_goblin_call_guards = 0xc6, last_goblin_call_guards = 0xc7;
const int goblin_call_guards_theft = 0xc9;
// Awoke sleeper.
const int heard_something = 0x95;
const int first_awakened = 0x95, last_awakened = 0x9a;
const int goblin_heard_something = 0xca;
const int goblin_awakened = 0xca;
// Magebane (SI only).
const int first_magebane_struck = 0x9b, last_magebane_struck = 0x9d;
// Hunger.
const int first_hunger = 0x77, last_hunger = 0x79;      // A little hungry.
const int first_needfood = 0x7a, last_needfood = 0x7c;  // Must have food.
const int first_starving = 0x7d, last_starving = 0x7f;  // Starving.
// Cold.
const int first_chilly = 0x9e, last_chilly = 0xa0;  // 0 - 9 temp
const int first_cold = 0xa1, last_cold = 0xa3;  // 10 - 19 temp
const int first_colder = 0xa4, last_colder = 0xa6;  // 20 - 29 temp
const int first_frostbite = 0xa7, last_frostbite = 0xa9;    // 30 - 39 temp
const int first_warming_in_cold_2 = 0xaa, last_warming_in_cold_2 = 0xb0; // < 30 temp
const int first_warming_in_cold = 0xb1, last_warming_in_cold = 0xb5; // 30+ temp
const int first_frostbite_2 = 0xb6, last_frostbite_2 = 0xb8;    // 40 - 49 temp
const int first_frostbite_3 = 0xb9, last_frostbite_3 = 0xbb;    // 50 - 59 temp
const int first_frozen = 0xbc, last_frozen = 0xbd;  // 60+ temp
const int first_warming_up_2 = 0xbe, last_warming_up_2 = 0xc1; // < 30 temp (warm area)
const int first_warming_up = 0xc2, last_warming_up = 0xc5;  // 30+ temp (warm area)

//	Messages in exultmsg.txt ( - 0x400):
const int first_chair_thief = 0x100, last_chair_thief = 0x104;
const int first_waiter_banter = 0x105, last_waiter_banter = 0x107;
const int first_waiter_serve = 0x108, last_waiter_serve = 0x109;
const int first_bed_occupied = 0x10a, num_bed_occupied = 3;
const int first_catchup = 0x10d, last_catchup = 0x10f;
//const int with_help_from = 0x110, exult_team = 0x111, driven_by_exult = 0x112;
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
const int my_leige = 0x13E, yo_homes = 0x13F;
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





