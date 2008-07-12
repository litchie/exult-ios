/**
 **	weaponinf.cc - Information from 'weapons.dat'.
 **
 **	Written: 06/01/2008 - Marzo
 **/

/*
Copyright (C) 2008 The Exult Team

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

#include <iomanip>	/* Debugging */
#include "utils.h"
#include "weaponinf.h"
using std::istream;
using std::cout;
using std::endl;

using namespace std;

Weapon_info Weapon_info::default_info;

/*
 *	Default shape info if not found.
 */
const Weapon_info *Weapon_info::get_default()
	{
	if (!default_info.damage)
		{
		default_info.missile_speed = 1;
		default_info.damage = 1;
		default_info.ammo = -1;
		default_info.projectile = -3;
		default_info.range = 4;
		default_info.uses = 2;
		default_info.actor_frames = 3;
		default_info.powers = 0;
		default_info.damage_type = 0;
		default_info.m_autohit =
		default_info.m_lucky =
		default_info.m_explodes =
		default_info.m_no_blocking =
		default_info.m_delete_depleted =
		default_info.m_returns =
		default_info.m_need_target = false;
		default_info.rotation_speed = 0;
		default_info.usecode = 0;
		default_info.sfx =
		default_info.hitsfx = -1;
		}
	return &default_info;
	}

int Weapon_info::get_base_strength()
	{
		// ++++The strength values are utter guesses.
	if (m_explodes && uses == melee)
		return -50;		// Avoid hand-held explosives at all costs.
	else if (usecode == 0x689)
		// Causes death in BG. In SI, weapons set with this function
		// also get picked very often even though the function does
		// not exits.
		return 5000;
	else if (m_explodes)	// These are safer, and seem to be preferred.
		return 3000;
	else
		{
		int strength;
		if (powers & Weapon_data::no_damage)
			strength = 0;	// Start from zero.
		else
			strength = damage;
			// These don't kill, but disable.
		strength += (powers & Weapon_data::sleep) ? 25 : 0;
		strength += (powers & Weapon_data::paralyze) ? 25 : 0;
			// Charm is slightly worse than the above two.
		strength += (powers & Weapon_data::charm) ? 20 : 0;
		strength += (powers & Weapon_data::poison) ? 10 : 0;
		strength += (powers & Weapon_data::curse) ? 5 : 0;
		strength += m_lucky ? 5 : 0;
		strength += damage_type != Weapon_data::normal_damage ? 10 : 0;
		if (m_autohit)
			strength *= 2;
			// Magebane power is too situation-specific.
			// Maybe give bonus for lightning damage, as it ignores armor?
		return strength;
		}
	}

/*
 *	Read in a weapon-info entry from 'weapons.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Weapon_info::read
	(
	std::istream& in,	// Input stream.
	int index,			// Line number (in some cases, this is the shapenum)
	int version,		// Data file version.
	bool bg				// Loading BG file.
	)
	{
	uint8 buf[21];			// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	if (buf[20] == 0xff)	// means delete entry.
		{
		set_invalid(true);
		return shapenum;
		}
	ammo = Read2(ptr);		// This is ammo family, or a neg. #.
					// Shape to strike with, or projectile
					//   shape if shoot/throw.
	projectile = Read2(ptr);	// What a projectile fired will look like.
#if 0
		extern char **item_names;
		cout << dec << "Weapon " //  << item_names[shapenum]
			<< '(' << shapenum << ')' << endl;
#endif
	damage = *ptr++;
	unsigned char flags0 = *ptr++;
	m_lucky = flags0&1;
	m_explodes = (flags0>>1)&1;
	m_no_blocking = (flags0>>2)&1;
	m_delete_depleted = (flags0>>3)&1;
	damage_type = (flags0>>4)&15;
	range = *ptr++;
	m_autohit = range&1;
	uses = (range>>1)&3;		// Throwable, etc.:
	range = range>>3;
	unsigned char flags1 = *ptr++;
	m_returns = (flags1&1);
	m_need_target = (flags1>>1)&1;
	missile_speed = (flags1>>2)&3;
	rotation_speed = (flags1>>4)&15;
#if 0
	// Testing if 'throwable'.  Looks like ammo==-3 => throwable UNLESS
	//   uses == 0.
	extern char **item_names;
	if ((ammo == -3) != (uses == 1 || uses == 2))
		cout << "Shape #" << shapenum << "(" << item_names[shapenum]
			<< ") has ammo = " << ammo << " and uses = "
			<< (int) uses << endl;
#endif
	unsigned char flags2 = *ptr++;
	actor_frames = (flags2&15);
	int speed =  (flags2>>5)&7;
	if (missile_speed)
		missile_speed = 4;
	else
		missile_speed = !speed ? 3 : (speed < 3 ? 2 : 1); 
	powers = *ptr++;
	*ptr++;				// Skip (0).
	usecode = Read2(ptr);
					// BG:  Subtract 1 from each sfx.
	int sfx_delta = bg ? -1 : 0;
	sfx = Read2(ptr) + sfx_delta;
	hitsfx = Read2(ptr) + sfx_delta;
	/*	Don't (seems to be unused).
	if (hitsfx == 123 && !bg)	// SerpentIsle:  Does not sound right.
		hitsfx = 61;		// Sounds more like a weapon.
	*/
	return shapenum;
	}

/*
 *	How much experience the weapon is worth in the hands of a
 *	defeated enemy. Ignores weapon range and ranged type (see
 *	Actor::reduce_health for why).
 */

int Weapon_info::get_base_xp_value()
	{
		// This formula is exact.
	int expval = damage;
	expval += 2*bitcount(powers);
	expval += m_explodes ? 1 : 0;
	expval += m_autohit ? 10 : 0;
	expval += m_no_blocking ? 1 : 0;
	}
