/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Shapevga.cc - Handle the 'shapes.vga' file and associated info.
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

#include <iomanip>			/* For debugging only. */
#include "shapevga.h"
#include "utils.h"

using std::ifstream;
using std::ios;

/*
 *	Read in data files about shapes.
 *
 *	Output:	0 if error.
 */

void Shapes_vga_file::read_info
	(
	)
	{
	ifstream shpdims;
	U7open(shpdims, SHPDIMS);
					// Starts at 0x96'th shape.
	int i; // Blame MSVC
	for (i = 0x96; i < num_shapes; i++)
		{
		shpdims.get((char&) info[i].shpdims[0]);
		shpdims.get((char&) info[i].shpdims[1]);
		}
	ifstream wgtvol;
	U7open(wgtvol, WGTVOL);
	for (i = 0; i < num_shapes; i++)
		{
		wgtvol.get((char&) info[i].weight);
		wgtvol.get((char&) info[i].volume);
		}
	ifstream tfa;
	U7open(tfa, TFA);
	for (i = 0; i < num_shapes; i++)
		{
		tfa.read((char*)&info[i].tfa[0], 3);
		info[i].set_tfa_data();
		}
	ifstream ready;
	U7open(ready, READY);
	int cnt = Read1(ready);		// Get # entries.
	for (i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(ready);
		unsigned char type = Read1(ready);
		info[shapenum].ready_type = type;
		ready.seekg(6, ios::cur);// Skip 9 bytes.
		}
	ready.close();
	ifstream armor;
	U7open(armor, ARMOR);
	cnt = Read1(armor);
	for (i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(armor);
		unsigned char points = Read1(armor);
		info[shapenum].armor = points;
		armor.seekg(7, ios::cur);// Skip 7 bytes.
		}
	armor.close();
	ifstream weapon;
	U7open(weapon, WEAPONS);
	cnt = Read1(weapon);
	for (i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(weapon);
					// This is ammo family, or a neg. #.
		short ammoshape = Read2(weapon);
					// Shape to strike with, or projectile
					//   shape if shoot/throw.
		short strikeshape = Read2(weapon);
#if 0
		extern char **item_names;
		cout << dec << "Weapon " << item_names[shapenum]
			<< '(' << shapenum << ')' << endl;
		cout << "ammoshape = " << ammoshape << ", strikeshape = " 
				<< strikeshape
				<< endl;
#endif
					// +++++Wonder what strike < 0 means.
		if (strikeshape == shapenum || strikeshape < 0)
			strikeshape = 0;// Means no projectile thrown.
		int damage = Read1(weapon);
		unsigned short flags0 = Read1(weapon);
		unsigned short range = Read1(weapon);	// High nibble.
		unsigned short unk1 = Read2(weapon);
		unsigned short special = Read1(weapon);
		Read1(weapon);		// Skip (0).
		short usecode = Read2(weapon);
		short usesfx = Read2(weapon) - 1;
		short hitsfx = Read2(weapon) - 1;
		unsigned char unk2[2];
		weapon.read((char *)unk2, sizeof(unk2));
#if 0
		cout << "Damage = " << damage << ", flags0 = " << hex
			<< " 0x" << setfill('0') << 
			setw(2) << flags0 << ", range?? = " <<
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
		info[shapenum].weapon = new Weapon_info(damage, 
			(range>>4)&0xf, range&0xf, special,
			ammoshape, strikeshape, usecode, usesfx, hitsfx);
		}
	weapon.close();	
	ifstream ammo;
	U7open(ammo, AMMO);
	cnt = Read1(ammo);
	Ammo_info::create();		// Create table.
	for (i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(ammo);
		unsigned short family = Read2(ammo);
		unsigned short type2 = Read2(ammo);	// ???
		unsigned char damage = Read1(ammo);
		ammo.seekg(6, ios::cur);	// Skip unknown.
		Ammo_info ent(shapenum, family, damage);
		Ammo_info::insert(shapenum, ent);
		}
	ammo.close();

	// Load data about drawing the weapon in an actor's hand
	ifstream wihh;
	unsigned short offsets[1024];
	U7open(wihh, WIHH);
	for (i = 0; i < 1024; i++)
		offsets[i] = Read2(wihh);
	for (i = 0; i < 1024; i++)
		// A zero offset means there is no record
		if(offsets[i] == 0)
			info[i].weapon_offsets = 0;
		else
			{
			wihh.seekg(offsets[i]);
			// There are two bytes per frame: 64 total
			info[i].weapon_offsets = new unsigned char[64];
			for(int j = 0; j < 32; j++)
				{
				unsigned char x = Read1(wihh);
				unsigned char y = Read1(wihh);
				// Set x/y to 255 if weapon is not to be drawn
				// In the file x/y are either 64 or 255:
				// I am assuming that they mean the same
				if(x > 63 || y > 63)
					x = y = 255;
				info[i].weapon_offsets[j * 2] = x;
				info[i].weapon_offsets[j * 2 + 1] = y;
				}
			}
	}


Shapes_vga_file::~Shapes_vga_file()
	{
	}

