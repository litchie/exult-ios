/**
 **	Monstinf.cc - Information (about NPC's, really) from 'monster.dat'.
 **
 **	Written: 8/13/01 - JSF
 **/

/*
Copyright (C) 2000-2013 The Exult Team

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

#include "utils.h"
#include "exult_constants.h"
#include "monstinf.h"

using std::istream;

std::vector<Equip_record> Monster_info::equip;
Monster_info Monster_info::default_info;

/*
 *	Read in monster info. from 'monsters.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

bool Monster_info::read
	(
	std::istream& in,	// Input stream.
	int version,		// Data file version.
	Exult_Game game		// Loading BG file.
	)
	{
	uint8 buf[Monster_info::entry_size-2];		// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	if (buf[Monster_info::entry_size-3] == 0xff)	// means delete entry.
		{
		set_invalid(true);
		return true;
		}
	m_sleep_safe = (*ptr&1);
	m_charm_safe = (*ptr >> 1) & 1;
	strength = (*ptr++ >> 2) & 63;	// Byte 2.

	m_curse_safe = (*ptr&1);
	m_paralysis_safe = (*ptr >> 1) & 1;
	dexterity = (*ptr++ >> 2) & 63;	// Byte 3.

	m_poison_safe = (*ptr&1);	// verified
	m_int_b1 = (*ptr >> 1) & 1;	// What does this do?
	intelligence = (*ptr++ >> 2) & 63;	// Byte 4.

	alignment = *ptr & 3;		// Byte 5.
	combat = (*ptr++ >> 2) & 63;

	m_splits = (*ptr & 1);	// Byte 6 (slimes).
	m_cant_die = (*ptr & 2) != 0;
	m_power_safe = (*ptr & 4) != 0;
	m_death_safe = (*ptr & 8) != 0;
	armor = (*ptr++ >> 4) & 15;

	ptr++;				// Byte 7: Unknown.
	reach = *ptr & 15;		// Byte 8 - weapon reach.
	weapon = (*ptr++ >> 4) & 15;
	flags = *ptr++;			// Byte 9.
	vulnerable = *ptr++;	// Byte 10.
	immune = *ptr++;		// Byte 11.
	m_cant_yell = (*ptr & (1<<5)) != 0;		// Byte 12.
	m_cant_bleed = (*ptr & (1<<6)) != 0;
	ptr++;
	m_attackmode = (*ptr & 7)-1;
	if (m_attackmode < 0)		// Fixes invalid data saved by older
		m_attackmode = 2;		// versions of ES.
	m_byte13 = (*ptr++)&~7;	// Byte 13: partly unknown.
	equip_offset = *ptr++;		// Byte 14.
	m_can_teleport = (*ptr & 1) != 0;	// Exult's extra flags: byte 15.
	m_can_summon = (*ptr & 2) != 0;
	m_can_be_invisible = (*ptr & 4) != 0;
	ptr++;
	ptr++;		// Byte 16: Unknown (0).
	int sfx_delta = game == BLACK_GATE ? -1 : 0;
	sfx = ((signed char)*ptr++) + sfx_delta;	// Byte 17.
	return true;
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
		default_info.combat = 10;
		default_info.alignment = 0;		// Neutral.
		default_info.armor = 
		default_info.weapon = 0;
		default_info.reach = 3;
		default_info.flags = (1<<(int) walk);
		default_info.equip_offset = 0;
		default_info.immune =
		default_info.vulnerable = 
		default_info.m_byte13;
		default_info.m_can_be_invisible = 
		default_info.m_can_summon = 
		default_info.m_can_teleport = 
		default_info.m_cant_bleed = 
		default_info.m_cant_die = 
		default_info.m_cant_yell = 
		default_info.m_poison_safe = 
		default_info.m_charm_safe =
		default_info.m_sleep_safe =
		default_info.m_paralysis_safe =
		default_info.m_curse_safe =
		default_info.m_power_safe =
		default_info.m_death_safe =
		default_info.m_int_b1 =
		default_info.m_splits = false;
		default_info.m_attackmode = 2;
		}
	return &default_info;
	}

/*
 *	Set all the stats.
 */

void Monster_info::set_stats
	(
	int str, int dex, int intel, int cmb, int armour,
	int wpn, int rch
	)
	{
	if (strength != str || dexterity != dex || intelligence != intel
			|| combat != cmb || armor != armour || weapon != wpn
			|| reach != rch)
		set_modified(true);
	strength = str;
	dexterity = dex;
	intelligence = intel;
	combat = cmb;
	armor = armour;
	weapon = wpn;
	reach = rch;
	}

/*
 *	How much experience the monster is worth. Ignores stats,
 *	immunities and vulnerabilities (check Actor::reduce_health
 *	for why).
 */

int Monster_info::get_base_xp_value()
	{
		// This formula is exact.
	int expval = armor + weapon;
	expval += m_sleep_safe ? 1 : 0;
	expval += m_charm_safe ? 1 : 0;
	expval += m_curse_safe ? 1 : 0;
	expval += m_paralysis_safe ? 1 : 0;
	expval += m_poison_safe ? 1 : 0;
		// Don't know what this does, but it *does* add to XP.
	expval += m_int_b1 ? 1 : 0;
		// This prevents death from Death Bolt.
	expval += m_death_safe ? 1 : 0;
	expval += m_power_safe ? 8 : 0;
	expval += (flags & (1 << fly)) ? 1 : 0;
	expval += (flags & (1 << swim)) ? 1 : 0;
	expval += (flags & (1 << ethereal)) ? 2 : 0;
	expval += (flags & (1 << 5)) ? 2 : 0;	// No idea what this does.
	expval += (flags & (1 << see_invisible)) ? 2 : 0;
	expval += (flags & (1 << start_invisible)) ? 8 : 0;
	expval += m_splits ? 2 : 0;
	expval += reach > 5 ? 2 : (reach > 3 ? 1 : 0);
	return expval;
	}

