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

#include "../alpha_kludges.h"

#include <iomanip.h>			/* For debugging only. */
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
	for (int i = 0x96; i < num_shapes; i++)
		{
		shpdims.get((char&) info[i].shpdims[0]);
		shpdims.get((char&) info[i].shpdims[1]);
		}
	ifstream wgtvol;
	U7open(wgtvol, WGTVOL);
	for (int i = 0; i < num_shapes; i++)
		{
		wgtvol.get((char&) info[i].weight);
		wgtvol.get((char&) info[i].volume);
		}
	ifstream tfa;
	U7open(tfa, TFA);
	for (int i = 0; i < num_shapes; i++)
		{
		tfa.read((char*)&info[i].tfa[0], 3);
		info[i].set_tfa_data();
		}
	ifstream ready;
	U7open(ready, READY);
	int cnt = Read1(ready);		// Get # entries.
	for (int i = 0; i < cnt; i++)
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
	for (int i = 0; i < cnt; i++)
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
	for (int i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(weapon);
		short unk0 = Read2(weapon);
		short ammoshape = Read2(weapon);
#if 0
		extern char **item_names;
		cout << dec << "Weapon " << item_names[shapenum] << endl;
		cout << "ammo = " << ammoshape << ", bytes1-2 = " << unk0
				<< endl;
#endif
					// +++++Wonder what ammo < 0 means.
		if (ammoshape == shapenum || ammoshape < 0)
			ammoshape = 0;
		int damage = Read1(weapon);
		unsigned short flags0 = Read1(weapon);
		unsigned short range = Read1(weapon);	// High nibble.
		unsigned short unk1 = Read2(weapon);
		unsigned short special = Read1(weapon);
		Read1(weapon);		// Skip (0).
		short usecode = Read2(weapon);
#if 0
		cout << "Damage = " << damage << ", flags0 = " << hex
			<< " 0x" << setw(2) << flags0 << ", range?? = " <<
			hex << "0x" << range << hex << ", unk1 = " << setw(2)
			<< "0x" << unk1 << endl;
		cout << "Special flags = " << "0x" << special <<
			", usecode = 0x" << usecode << endl;
		cout << endl;
#endif
		weapon.seekg(6, ios::cur);	// Skip unknown.
		info[shapenum].weapon = new Weapon_info(damage, special,
							ammoshape, usecode);
		}
	weapon.close();	
					// Since we don't know the flag yet...
	info[278].set_ammo_consumed();	// Musket.
	info[474].set_ammo_consumed();	// Sling.
	info[563].set_ammo_consumed();	// Blowgun.
	info[597].set_ammo_consumed();	// Bow.
	info[598].set_ammo_consumed();	// Crossbow.
	info[606].set_ammo_consumed();	// Magic bow.	
	info[647].set_ammo_consumed();	// Triple crossbow.
	ifstream ammo;
	U7open(ammo, AMMO);
	cnt = Read1(ammo);
	Ammo_info::create();		// Create table.
	for (int i = 0; i < cnt; i++)
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
	for (int i = 0; i < 1024; i++)
		offsets[i] = Read2(wihh);
	for (int i = 0; i < 1024; i++)
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

