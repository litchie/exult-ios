/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Ready.h - Information from the 'ready.dat' file.
 **
 **	Written: 5/1/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#ifndef INCL_READY
#define INCL_READY	1

class Ready_table;

/*
 *	This class provides a lookup table to see where item's may be held.
 */
class Ready_info
	{
	Ready_table *table;
public:
	Ready_info(char *fname);	// Create from file.
	~Ready_info();
	enum Ready_type {
		other = 0x00,		// Anything else.
		spell = 0x01,		// 1-handed spell.
		one_handed_weapon = 0x08,
		neck_armor = 0x20,
		torso_armor = 0x28,
		ring = 0x30,
		ammunition = 0x40,
		head_armor = 0x48,	// I.e., helm.
		leg_armor = 0x50,	// I.e., leggings.
		foot_armor = 0x58,	// I.e., boots.
		// triple_crossbow_bolts? = 0x78,
		tongs = 0x90,
		two_handed_weapon = 0xa0,
		other_spell = 0xa1,	// #676,
		gloves = 0xa8
		};
	enum Ready_type get_type(int shapenum);
	};

#endif	/* INCL_READY	*/

