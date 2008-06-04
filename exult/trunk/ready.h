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
 *  GNU General Public License for more details.
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
	one_handed_weapon = 0x01,
	neck_armor = 0x04,
	torso_armor = 0x05,
	ring = 0x06,
	usecode_container_bg = 0x07,	// Porting from SI
	ammunition = 0x08,
	head_armor = 0x09,		// I.e., helm.
	leg_armor = 0x0a,		// I.e., leggings.
	foot_armor = 0x0b,		// I.e., boots.
	triple_crossbow_bolts = 0x0f,
	tongs = 0x12,
	two_handed_weapon = 0x14,
	gloves = 0x15
	};

// Serpent Isle Ready types
enum Ready_type_SI {
	other_si = 0x00,			// Anything else.
	one_handed_si = 0x01,
	cloak_si = 0x02,
	amulet_si = 0x03,
	helm_si = 0x04,
	gloves_si = 0x05,
	usecode_container_si = 0x06,
	ring_si = 0x08,
	earrings_si = 0x09,
	ammo_si = 0x0a,
	belt_si = 0x0b,
	armour_si = 0x0c,
	boots_si = 0x0d,
	leggings_si = 0x0e,
	backpack_si = 0x0f,			// Diaper as well?????
	two_handed_si = 0x14
	};
	
#endif	/* INCL_READY	*/

