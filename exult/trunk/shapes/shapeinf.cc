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

Ammo_table *Ammo_info::table = 0;

#include "utils.h"

#ifndef DONT_HAVE_HASH_MAP
#if __GNUG__ > 2
#  include <ext/hash_map>
using std::hash_map;
#else
#  include <hash_map>
#endif
#  ifdef MACOS
	using Metrowerks::hash_map;
#  endif
#else
#  include <map>
#endif

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
 *	Read in a amm-info entry from 'amms.dat'.
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
	shapenum = Read2(ptr);		// Bytes 0-1.
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
 *	For looking up ammo entries:
 */

class Ammo_table
	{
#ifndef DONT_HAVE_HASH_MAP
	hash_map<int, Ammo_info> my_map;
#else
	std::map<int, Ammo_info> my_map;
#endif
public:

#ifndef DONT_HAVE_HASH_MAP
	Ammo_table() : my_map(53) {  }
#else
	Ammo_table() : my_map() {  }
#endif
	void insert(int shnum, Ammo_info& ent)
		{ my_map[shnum] = ent; }
	Ammo_info *find(int shnum)	// Look up given shape.
		{
#ifndef DONT_HAVE_HASH_MAP
		hash_map<int, Ammo_info>::iterator it = my_map.find(shnum);
#else
		std::map<int, Ammo_info>::iterator it = my_map.find(shnum);
#endif
		return it == my_map.end() ? 0 : &((*it).second);
		}
	};

/*
 *	Create ammo table.
 */

void Ammo_info::create
	(
	)
	{
	table = new Ammo_table;
	}

/*
 *	Insert an entry.
 */

void Ammo_info::insert(int shnum, Ammo_info& ent)
	{ table->insert(shnum, ent); }


/*
 *	Look up ammo's entry.
 */

Ammo_info *Ammo_info::find
	(
	int shnum			// Shape #.
	)
	{
	return Ammo_info::table->find(shnum);
	}


Shape_info::~Shape_info()
	{
	delete weapon;
	if(weapon_offsets)
		delete [] weapon_offsets;
	delete monstinf;
	}
