/*
 *	shapevga.cc - Handle the 'shapes.vga' file and associated info.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iomanip>			/* For debugging only. */
#include "shapevga.h"
#include "monstinf.h"
#include "utils.h"

using std::ifstream;
using std::ios;

// For convienience
#define patch_exists(p) (have_patch_path && U7exists(p))

/*
 *	Read in data files about shapes.
 *
 *	Output:	0 if error.
 */

void Shapes_vga_file::read_info
	(
	)
	{
	int i, cnt;
	bool have_patch_path = is_system_path_defined("<PATCH>");

	// ShapeDims

	// Starts at 0x96'th shape.
	ifstream shpdims;
	if (patch_exists(PATCH_SHPDIMS)) U7open(shpdims, PATCH_SHPDIMS);
	else U7open(shpdims, SHPDIMS);
	for (i = 0x96; i < num_shapes && !shpdims.eof(); i++)
		{
		shpdims.get((char&) info[i].shpdims[0]);
		shpdims.get((char&) info[i].shpdims[1]);
		}

	// WGTVOL
	ifstream wgtvol;
	if (patch_exists(PATCH_WGTVOL)) U7open(wgtvol, PATCH_WGTVOL);
	else U7open(wgtvol, WGTVOL);
	for (i = 0; i < num_shapes && !wgtvol.eof(); i++)
		{
		wgtvol.get((char&) info[i].weight);
		wgtvol.get((char&) info[i].volume);
		}

	// TFA
	ifstream tfa;
	if (patch_exists(PATCH_TFA)) U7open(tfa, PATCH_TFA);
	else U7open(tfa, TFA);
	for (i = 0; i < num_shapes && !tfa.eof(); i++)
		{
		tfa.read((char*)&info[i].tfa[0], 3);
		info[i].set_tfa_data();
		}

	ifstream ready;
	if (patch_exists(PATCH_READY)) U7open(ready, PATCH_READY);
	else U7open(ready, READY);
	cnt = Read1(ready);		// Get # entries.
	for (i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(ready);
		unsigned char type = Read1(ready);
		info[shapenum].ready_type = type;
		ready.seekg(6, ios::cur);// Skip 9 bytes.
		}
	ready.close();
	ifstream armor;
	if (patch_exists(PATCH_ARMOR)) U7open(armor, PATCH_ARMOR);
	else U7open(armor, ARMOR);
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
	if (patch_exists(PATCH_WEAPONS)) U7open(weapon, PATCH_WEAPONS);
	else U7open(weapon, WEAPONS);
	cnt = Read1(weapon);
	for (i = 0; i < cnt; i++)
		{
		Weapon_info *winf = new Weapon_info();
		unsigned short shapenum = winf->read(weapon);
		info[shapenum].weapon = winf;
		}
	weapon.close();	
	ifstream ammo;
	if (patch_exists(PATCH_AMMO)) U7open(ammo, PATCH_AMMO);
	else U7open(ammo, AMMO);
	cnt = Read1(ammo);
	Ammo_info::create();		// Create table.
	for (i = 0; i < cnt; i++)
		{
		Ammo_info ent;
		unsigned short shapenum = ent.read(ammo);
		Ammo_info::insert(shapenum, ent);
		}
	ammo.close();

	// Load data about drawing the weapon in an actor's hand
	ifstream wihh;
	unsigned short offsets[1024];
	if (patch_exists(PATCH_WIHH)) U7open(wihh, PATCH_WIHH);
	else U7open(wihh, WIHH);
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
	wihh.close();
	ifstream mfile;			// Now get monster info.
	U7open(mfile, MONSTERS);
	int num_monsters = Read1(mfile);
	for (i = 0; i < num_monsters; i++)
		{
		Monster_info *minf = new Monster_info();
		int shnum = minf->read(mfile);
		info[shnum].monstinf = minf;
		}
	mfile.close();
	U7open(mfile, EQUIP);		// Get 'equip.dat'.
	int num_recs = Read1(mfile);
	Equip_record *equip = new Equip_record[num_recs];
	for (i = 0; i < num_recs; i++)
		{
		Equip_record& rec = equip[i];
					// 10 elements/record.
		for (int elem = 0; elem < 10; elem++)
			{
			int shnum = Read2(mfile);
			unsigned prob = Read1(mfile);
			unsigned quant = Read1(mfile);
			Read2(mfile);
			rec.set(elem, shnum, prob, quant);
			}
		}
					// Monster_info owns this.
	Monster_info::set_equip(equip, num_recs);
	mfile.close();
	ifstream(occ);			// Read flags from occlude.dat.
	U7open(occ, OCCLUDE);
	unsigned char occbits[128];	// 1024 bit flags.
	occ.read((char *)occbits, sizeof(occbits));
	for (i = 0; i < sizeof(occbits); i++)
		{
		unsigned char bits = occbits[i];
		int shnum = i*8;	// Check each bit.
		for (int b = 0; bits; b++, bits = bits>>1)
			if (bits&1)
				{
				info[shnum + b].occludes_flag = true;
//				cout << "Occludes: " << shnum + b << endl;
				}
		}
	}


Shapes_vga_file::~Shapes_vga_file()
	{
	}


void Shapes_vga_file::init()
{
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_SHAPES))
		load(SHAPES_VGA, PATCH_SHAPES);
	else
		load(SHAPES_VGA);

	info.set_size(num_shapes);
}
