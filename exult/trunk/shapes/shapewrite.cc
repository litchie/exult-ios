/*
 *	shapewrite.cc - Write out the shape 'info' files.
 *
 *	Note:  Currently only used by ExultStudio.
 *
 *  Copyright (C) 2002  The Exult Team
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
#include "shapeinf.h"
#include "monstinf.h"
#include "utils.h"
#include "exceptions.h"

using std::ifstream;
using std::ios;

/*
 *	Write out data files about shapes.
 */

void Shapes_vga_file::write_info
	(
	bool bg				// True if BlackGate.
	)
	{
	int i, cnt;
	bool have_patch_path = is_system_path_defined("<PATCH>");
	assert(have_patch_path);

	// ShapeDims
	// Starts at 0x96'th shape.
	ofstream shpdims;
	U7open(shpdims, PATCH_SHPDIMS);
	for (i = 0x96; i < num_shapes; i++)
		{
		shpdims.put((char&) info[i].shpdims[0]);
		shpdims.put((char&) info[i].shpdims[1]);
		}

	// WGTVOL
	ofstream wgtvol;
	U7open(wgtvol, PATCH_WGTVOL);
	for (i = 0; i < num_shapes; i++)
		{
		wgtvol.put((char&) info[i].weight);
		wgtvol.put((char&) info[i].volume);
		}

	// TFA
	ofstream tfa;
	U7open(tfa, PATCH_TFA);
	for (i = 0; i < num_shapes; i++)
		tfa.write((char*)&info[i].tfa[0], 3);

	ofstream ready;
	U7open(ready, PATCH_READY);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].ready_type != 255)
			cnt++;	//++++Got to init ready_type to 255!!!!!
	ready.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].ready_type != 255)
			{
			Write2(ready, i);	// Shape #.
			ready.put(info[i].ready_type);
			for (int j = 0; j < 6; j++)
				ready.put(0);	// 6 0's.
			}
	ready.close();

	ofstream armor;
	U7open(armor, PATCH_ARMOR);
//++++++Need an Armor_info class!!
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].armor != 0)
			cnt++;
	armor.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].armor != 0)
			{
			Write2(armor, i);
			armor.put(armor);//+++Points for now.
			for (int j = 0; j < 7; j++) //++++++
				armor.put(0);
			}
	armor.close();
	ofstream weapon;
	U7open(weapon, PATCH_WEAPONS);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon)
			cnt++;
	weapon.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon)
			info[i].weapon->write(weapon, bg);
	weapon.close();	

	ofstream ammo;
	U7open(ammo, PATCH_AMMO);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].ammo)	//+++++Ammo field instead of table.
			cnt++;
	ammo.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].ammo)
			info[i].ammo->write(ammo);
	ammo.close();

	// Write data about drawing the weapon in an actor's hand
	ofstream wihh;
	U7open(wihh, PATCH_WIHH);
	cnt = 0;			// Keep track of actual entries.
	for (i = 0; i < 1024; i++)
		{
		if (info[i].weapon_offsets == 0)
			Write2(wihh, 0);// None for this shape.
		else			// Write where it will go.
			Write2(wihh, 2*1024 + 64*(cnt++));
	for (i = 0; i < 1024; i++)
		if (info[i].weapon_offsets)
				// There are two bytes per frame: 64 total
			wihh.write(info[i].weapon_offsets, 64);
	wihh.close();
	ofstream mfile;			// Now get monster info.
	U7open(mfile, MONSTERS_PATCH);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].monstinf)
			cnt++;
	mfile.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].monstinf)
			info[i].monstinf->write(mfile);
	mfile.close();

	U7open(mfile, EQUIP_PATCH);	// Write 'equip.dat'.
	mfile.put(Monster_info::equip_cnt);
	for (i = 0; i < Monster_info::equip_cnt; i++)
		{
		Equip_record& rec = Monster_info::equip[i];
					// 10 elements/record.
		for (int elem = 0; elem < 10; elem++)
			{
			write2(mfile, rec.elements[i].shapenum);
			mfile.put(rec.elements[i].probability);
			mfile.put(rec.elements[i].quantity);
			Write2(mfile, 0);
			}
	mfile.close();

	ofstream(occ);			// Write occlude.dat.
	U7open(occ, OCCLUDE_PATCH);
	unsigned char occbits[128];	// 1024 bit flags.
	for (i = 0; i < sizeof(occbits); i++)
		{
		unsigned char bits = 0;
		int shnum = i*8;	// Check each bit.
		for (int b = 0; b < 8; b++)
			if (info[shnum + b].occludes_flag)
				bits |= (1<<b);
		occbits[i] = bits;	
		}
	occ.write((char *)occbits, sizeof(occbits));
	}
