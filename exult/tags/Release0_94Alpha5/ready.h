/*
 *	ready.h - Information from the 'ready.dat' file.
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

#ifndef INCL_READY
#define INCL_READY	1

/*
 *	Types from 'ready.dat' describing how a shape may be worn:
 */
enum Ready_type {
	other = 0x00,			// Anything else.
	spell = 0x01,			// 1-handed spell.
	one_handed_weapon = 0x08,
	neck_armor = 0x20,
	torso_armor = 0x28,
	ring = 0x30,
	ammunition = 0x40,
	head_armor = 0x48,		// I.e., helm.
	leg_armor = 0x50,		// I.e., leggings.
	foot_armor = 0x58,		// I.e., boots.
	// triple_crossbow_bolts? = 0x78,
	tongs = 0x90,
	two_handed_weapon = 0xa0,
	other_spell = 0xa1,		// #676,
	gloves = 0xa8
	};

// Serpent Isle Ready types
enum Ready_type_SI {
	other_si = 0x00,			// Anything else.
	spell_si = 0x01,			// 1-handed spell.
	one_handed_si = 0x08,
	cloak_si = 0x10,
	amulet_si = 0x18,
	helm_si = 0x20,
	gloves_si = 0x28,
	usecode_container_si = 0x30,
	ring_si = 0x40,
	earrings_si = 0x48,
	ammo_si = 0x50,
	belt_si = 0x58,
	armour_si = 0x60,
	boots_si = 0x68,
	leggings_si = 0x70,
	backpack_si = 0x78,			// Diaper as well?????
	two_handed_si = 0xa0,
	other_spell_si = 0xa1		// #676,
	};
	
#endif	/* INCL_READY	*/

