/**
 **	Monstinf.cc - Information (about NPC's, really) from 'monster.dat'.
 **
 **	Written: 8/13/01 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "utils.h"
#include "monstinf.h"

// #include "items.h"

using std::ios;
using std::cout;
using std::endl;
using std::istream;

Equip_record *Monster_info::equip = 0;
int Monster_info::equip_cnt = 0;
Monster_info Monster_info::default_info;

/*
 *	Read in monster info. from 'monsters.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Monster_info::read
	(
	istream& in			// Read from here.
	)
	{
	uint8 buf[25];		// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	strength = (*ptr++ >> 2) & 63;	// Byte 2.
	dexterity = (*ptr++ >> 2) & 63;	// Byte 3.
	m_poison_safe = (*ptr&1) != 0;	// This looks reasonable, as it
					//   includes automaton, slug, spider.
//	if (poison_safe)
//		cout << "Shape " << item_names[shapenum] << " is poison_safe"<< endl;
	intelligence = (*ptr++ >> 2) & 63;	// Byte 4.
	alignment = *ptr & 3;		// Byte 5.
	combat = (*ptr++ >> 2) & 63;
	m_splits = (*ptr & 1) != 0;	// Byte 6 (slimes).
	m_cant_die = (*ptr & 2) != 0;
	armor = (*ptr++ >> 4) & 15;
	ptr++;				// Unknown.
	reach = *ptr & 15;		// Byte 8 - weapon reach.
	weapon = (*ptr++ >> 4) & 15;
	flags = *ptr++;			// Byte 9.
	vulnerable = *ptr++;
	immune = *ptr++;
	m_cant_yell = (*ptr & (1<<5)) != 0;
	m_cant_bleed = (*ptr & (1<<6)) != 0;
	ptr++;
	ptr++;				// Unknown.
	equip_offset = *ptr++;		// Byte 13.
	return shapenum;
	}

/*
 *	Get a default block for generic NPC's.
 */

const Monster_info *Monster_info::get_default
	(
	)
	{
	if (!default_info.strength)	// First time?
		{
		default_info.strength = 
		default_info.dexterity = 
		default_info.intelligence = 
		default_info.combat = 4;
		default_info.alignment = 0;		// Neutral.
		default_info.m_splits = default_info.m_cant_die = false;
		default_info.armor = 
		default_info.weapon = default_info.reach = 0;
		default_info.flags = (1<<(int) walk);
		default_info.equip_offset = 0;
		}
	return &default_info;
	}

/*
 *	Set all the stats.
 */

void Monster_info::set_stats
	(
	int str, int dex, int intel, int cmb, int arm,
	int wpn, int rch
	)
	{
	strength = str;
	dexterity = dex;
	intelligence = intel;
	combat = cmb;
	armor = arm;
	weapon = wpn;
	reach = rch;
	}

