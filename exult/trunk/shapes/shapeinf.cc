/**
 **	Shapeinf.cc: Info. about shapes read from various 'static' data files.
 **
 **	Written: 4/29/99 - JSF
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "shapeinf.h"
#include "monstinf.h"

#include "utils.h"

/*
 *	Read in a weapon-info entry from 'weapons.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Weapon_info::read
	(
	std::istream& in,			// Read from here.
	bool bg
	)
	{
	uint8 buf[21];			// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	ammo = Read2(ptr);		// This is ammo family, or a neg. #.
					// Shape to strike with, or projectile
					//   shape if shoot/throw.
	projectile = Read2(ptr);
#if 0
		extern char **item_names;
		cout << dec << "Weapon " << item_names[shapenum]
			<< '(' << shapenum << ')' << endl;
		cout << "ammoshape = " << ammoshape << ", projectile = " 
				<< projectile
				<< endl;
#endif
					// +++++Wonder what strike < 0 means.
	if (projectile == shapenum || projectile < 0)
		projectile = 0;		// Means no projectile thrown.
	damage = *ptr++;
	unsigned char flags0 = *ptr++;
	m_explodes = (flags0>>1)&1;
	damage_type = (flags0>>4)&15;
	range = *ptr++;
	uses = (range>>1)&3;		// Throwable, etc.:
	range = range>>3;
	unsigned char flags1 = *ptr++;
	m_returns = (flags1&1);
	unsigned char unk1 = *ptr++;
	powers = *ptr++;
	*ptr++;				// Skip (0).
	usecode = Read2(ptr);
					// BG:  Subtract 1 from each sfx.
	int sfx_delta = bg ? -1 : 0;
	sfx = Read2(ptr) + sfx_delta;
	hitsfx = Read2(ptr) + sfx_delta;
					// Last 2 bytes unknown/unused.
#if 0
		cout << "Damage = " << damage << ", flags0 = " << hex
			<< " 0x" << setfill('0') << 
			setw(2) << flags0 << ", use = " << use <<
			", range = " <<
			hex << "0x" << range << hex << ", unk1 = " << setw(2)
			<< "0x" << unk1 << endl;
		cout << "Special flags = " << "0x" << special <<
			", usecode = 0x" << usecode << endl;
		cout << dec << "Sfx = " << usesfx << ", hitsfx = " <<
				hitsfx << endl;
		cout << "Unknown at end:  ";
		for (int i = 0; i < sizeof(unk2); i++)
			cout << setw(2) << setfill('0') <<
						(short) unk2[i] << ' ';
		cout << dec << endl << endl;
#endif
	return shapenum;
	}

/*
 *	Read in an ammo-info entry from 'ammo.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Ammo_info::read
	(
	std::istream& in		// Read from here.
	)
	{
	uint8 buf[13];			// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	family_shape = Read2(ptr);
	unsigned short type2 = Read2(ptr);	// ???
	damage = *ptr++;
	ptr += 2;			// 2 unknown.
	damage_type = (*ptr++>>4)&15;
	powers = *ptr++;
					// Last 2 unknown.
	return shapenum;
	}

/*
 *	Read in an armor-info entry from 'armor.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Armor_info::read
	(
	std::istream& in		// Read from here.
	)
	{
	uint8 buf[10];			// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	prot = *ptr++;			// Protection value.
	ptr++;				// Unknown.
	immune = *ptr++;		// Immunity flags.
					// Last 5 are unknown/unused.
	return shapenum;
	}

/*
 *	Clean up.
 */

Shape_info::~Shape_info()
	{
	delete weapon;
	delete ammo;
	if(weapon_offsets)
		delete [] weapon_offsets;
	delete monstinf;
	}
