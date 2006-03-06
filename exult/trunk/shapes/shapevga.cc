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
 *  GNU General Public License for more details.
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
#include "exceptions.h"

using std::ifstream;
using std::ios;

// For convienience
#define patch_exists(p) (have_patch_path && U7exists(p))
#define patch_name(p) (patch_exists(p) ? (p) : 0)

/*
 *	Open, but don't quit if editing.  We first try the patch name if it's
 *	given.
 */

static bool U7open2
	(
	ifstream& in,			// Stream to open.
	const char *pname,		// Patch name, or null.
	const char *fname,		// File name.
	bool editing
	)
	{
	if (pname)
		{
		U7open(in, pname);
		return true;
		}
	try
		{
		U7open(in, fname);
		}
	catch(const file_exception & f)
		{
#if 0
		if (editing)
			return false;
		throw f;
#else	/* This is preferable. */
		return false;
#endif
		}
	return true;
	}

/*
 *	Reload info.
 */

void Shapes_vga_file::reload_info
	(
	Exult_Game game,		// Which game.
	int min_info_size
	)
	{
	info_read = false;
	info.resize(0);
	info.resize(min_info_size > num_shapes ? min_info_size : num_shapes);
	read_info(game);
	}	

/*
 *	Read in data files about shapes.
 *
 *	Output:	0 if error.
 */

void Shapes_vga_file::read_info
	(
	Exult_Game game,		// Which game.
	bool editing			// True to allow files to not exist.
	)
	{
	if (info_read)
		return;
	info_read = true;
	int i, cnt;
	bool have_patch_path = is_system_path_defined("<PATCH>");

	// ShapeDims

	// Starts at 0x96'th shape.
	ifstream shpdims;
	if (U7open2(shpdims, patch_name(PATCH_SHPDIMS), SHPDIMS, editing))
		for (i = c_first_obj_shape; 
					i < num_shapes && !shpdims.eof(); i++)
			{
			shpdims.get((char&) info[i].shpdims[0]);
			shpdims.get((char&) info[i].shpdims[1]);
			}

	// WGTVOL
	ifstream wgtvol;
	if (U7open2(wgtvol, patch_name(PATCH_WGTVOL), WGTVOL, editing))
		for (i = 0; i < num_shapes && !wgtvol.eof(); i++)
			{
			wgtvol.get((char&) info[i].weight);
			wgtvol.get((char&) info[i].volume);
			}

	// TFA
	ifstream tfa;
	if (U7open2(tfa, patch_name(PATCH_TFA), TFA, editing))
		for (i = 0; i < num_shapes && !tfa.eof(); i++)
			{
			tfa.read((char*)&info[i].tfa[0], 3);
			info[i].set_tfa_data();
			}

	ifstream ready;
	if (U7open2(ready, patch_name(PATCH_READY), READY, editing))
		{
		cnt = Read1(ready);		// Get # entries.
		for (i = 0; i < cnt; i++)
			{
			unsigned short shapenum = Read2(ready);
			unsigned char type = Read1(ready);
			info[shapenum].ready_type = type;
			ready.seekg(6, ios::cur);// Skip 9 bytes.
			}
		ready.close();
		}
	ifstream armor;
	if (U7open2(armor, patch_name(PATCH_ARMOR), ARMOR, editing))
		{
		cnt = Read1(armor);
		for (i = 0; i < cnt; i++)
			{
			Armor_info *ainf = new Armor_info();
			unsigned short shapenum = ainf->read(armor);
			info[shapenum].armor = ainf;
			}
		armor.close();
		}
	ifstream weapon;
	if (U7open2(weapon, patch_name(PATCH_WEAPONS), WEAPONS, editing))
		{
		cnt = Read1(weapon);
		for (i = 0; i < cnt; i++)
			{
			Weapon_info *winf = new Weapon_info();
			unsigned short shapenum = winf->read(weapon, 
					game == BLACK_GATE);
			info[shapenum].weapon = winf;
			}
		weapon.close();	
		}
	ifstream ammo;
	if (U7open2(ammo, patch_name(PATCH_AMMO), AMMO, editing))
		{
		cnt = Read1(ammo);
		for (i = 0; i < cnt; i++)
			{
			Ammo_info *ainf = new Ammo_info();
			unsigned short shapenum = ainf->read(ammo);
			info[shapenum].ammo = ainf;
			}
		ammo.close();
		}
	ifstream container;
	if (U7open2(container, patch_name(PATCH_CONTAINER), CONTAINER,editing))
		{
		int vers = Read1(container);
		cnt = Read1(container);
		for (i = 0; i < cnt; i++)
			{
			uint8 buf[4];
			uint8 *ptr = &buf[0];
			container.read((char *)&buf[0], sizeof(buf));
			int shapenum = Read2(ptr);
			info[shapenum].container_gump = Read2(ptr);
			}
		container.close();
		}
	// Load data about drawing the weapon in an actor's hand
	ifstream wihh;
	unsigned short offsets[2048];
	if (U7open2(wihh, patch_name(PATCH_WIHH), WIHH, editing))
		cnt = num_shapes;// <= 1024 ? num_shapes : 1024;
	else
		cnt = 0;
	for (i = 0; i < cnt; i++)
		offsets[i] = Read2(wihh);
	for (i = 0; i < cnt; i++)
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
	if (cnt)
		wihh.close();
	ifstream mfile;			// Now get monster info.
	if (U7open2(mfile, patch_name(PATCH_MONSTERS), MONSTERS, editing))
		{
		int num_monsters = Read1(mfile);
		for (i = 0; i < num_monsters; i++)
			{
			Monster_info *minf = new Monster_info();
			int shnum = minf->read(mfile);
			info[shnum].monstinf = minf;
			}
		mfile.close();
		}
					// Get 'equip.dat'.
	if (U7open2(mfile, patch_name(PATCH_EQUIP), EQUIP, editing))
		{
		int num_recs = Read1(mfile);
		Monster_info::reserve_equip(num_recs);
		for (i = 0; i < num_recs; i++)
			{
			Equip_record equip;
					// 10 elements/record.
			for (int elem = 0; elem < 10; elem++)
				{
				int shnum = Read2(mfile);
				unsigned prob = Read1(mfile);
				unsigned quant = Read1(mfile);
				Read2(mfile);
				equip.set(elem, shnum, prob, quant);
				}
			Monster_info::add_equip(equip);
			}
		mfile.close();
		}
	ifstream(occ);			// Read flags from occlude.dat.
	if (U7open2(occ, patch_name(PATCH_OCCLUDE), OCCLUDE, editing))
		{
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
					}
			}
		}
	}

/*
 *	Open/close file.
 */

Shapes_vga_file::Shapes_vga_file
	(
	const char *nm,			// Path to file.
	int u7drag,			// # from u7drag.h, or -1.
	const char *nm2			// Path to patch version, or 0.
	) : Vga_file(nm, u7drag, nm2), info_read(false), info(num_shapes)
	{
	}

Shapes_vga_file::~Shapes_vga_file()
	{
	}


void Shapes_vga_file::init(int min_info_size)
{
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_SHAPES))
		load(SHAPES_VGA, PATCH_SHAPES);
	else
		load(SHAPES_VGA);
	info_read = false;
	info.resize(0);
	info.resize(min_info_size > num_shapes ? min_info_size : num_shapes);
}

/*
 *	Make a spot for a new shape, and delete frames in existing shape.
 *
 *	Output:	->shape, or 0 if invalid shapenum.
 */

Shape *Shapes_vga_file::new_shape
	(
	int shapenum
	)
	{
	Shape *newshape = Vga_file::new_shape(shapenum);
	if (newshape && shapenum >= info.size())
		info.resize(shapenum + 1);
	return newshape;
	}
