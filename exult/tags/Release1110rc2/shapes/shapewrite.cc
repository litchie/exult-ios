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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "shapevga.h"
#include "shapeinf.h"
#include "monstinf.h"
#include "utils.h"
#include "exceptions.h"

using std::ifstream;
using std::ios;
using std::ofstream;

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
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].armor != 0)
			cnt++;
	armor.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].armor != 0)
			info[i].armor->write(i, armor);
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
			info[i].weapon->write(i, weapon, bg);
	weapon.close();	

	ofstream ammo;
	U7open(ammo, PATCH_AMMO);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].ammo)
			cnt++;
	ammo.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].ammo)
			info[i].ammo->write(i, ammo);
	ammo.close();

	// Write data about drawing the weapon in an actor's hand
	ofstream wihh;
	U7open(wihh, PATCH_WIHH);
	cnt = 0;			// Keep track of actual entries.
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon_offsets == 0)
			Write2(wihh, 0);// None for this shape.
		else			// Write where it will go.
			Write2(wihh, 2*1024 + 64*(cnt++));
	for (i = 0; i < num_shapes; i++)
		if (info[i].weapon_offsets)
				// There are two bytes per frame: 64 total
			wihh.write((char *)(info[i].weapon_offsets), 64);
	wihh.close();
	ofstream mfile;			// Now get monster info.
	U7open(mfile, PATCH_MONSTERS);
	cnt = 0;
	for (i = 0; i < num_shapes; i++)
		if (info[i].monstinf)
			cnt++;
	mfile.put(cnt);
	for (i = 0; i < num_shapes; i++)
		if (info[i].monstinf)
			info[i].monstinf->write(i, mfile);
	mfile.close();

	U7open(mfile, PATCH_EQUIP);	// Write 'equip.dat'.
	cnt = Monster_info::get_equip_cnt();
	mfile.put(cnt);
	for (i = 0; i < cnt; i++)
		{
		Equip_record& rec = Monster_info::get_equip(i);
					// 10 elements/record.
		for (int e = 0; e < 10; e++)
			{
			Equip_element& elem = rec.get(e);
			Write2(mfile, elem.get_shapenum());
			mfile.put(elem.get_probability());
			mfile.put(elem.get_quantity());
			Write2(mfile, 0);
			}
		}
	mfile.close();

	ofstream(occ);			// Write occlude.dat.
	U7open(occ, PATCH_OCCLUDE);
	unsigned char occbits[128];	// 1024 bit flags.
					// +++++This could be rewritten better!
	memset(&occbits[0], 0, sizeof(occbits));
	for (i = 0; i < sizeof(occbits); i++)
		{
		unsigned char bits = 0;
		int shnum = i*8;	// Check each bit.
		for (int b = 0; b < 8; b++)
			if (shnum + b >= num_shapes)
				break;
			else if (info[shnum + b].occludes_flag)
				bits |= (1<<b);
		occbits[i] = bits;	
		}
	occ.write((char *)occbits, sizeof(occbits));
	}

/*
 *	Write out a weapon-info entry to 'weapons.dat'.
 */

void Weapon_info::write
	(
	int shapenum,
	std::ostream& out,		// Write to here.
	bool bg
	)
	{
	uint8 buf[21];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);		// Bytes 0-1.
	Write2(ptr, ammo);
	Write2(ptr, projectile);
	*ptr++ = damage;
	unsigned char flags0 = (damage_type<<4) |
				((m_explodes ? 1 : 0)<<1);
	*ptr++ = flags0;
	*ptr++ = (range<<3) | (uses<<1);
	unsigned char flags1 = m_returns ? 1 : 0;
	*ptr++ = flags1;
	*ptr++ = actor_frames;
	*ptr++ = powers;
	*ptr++ = 0;			// ??
	Write2(ptr, usecode);
					// BG:  Subtracted 1 from each sfx.
	int sfx_delta = bg ? -1 : 0;
	Write2(ptr, sfx - sfx_delta);
	Write2(ptr, hitsfx + sfx_delta);
					// Last 2 bytes unknown/unused.
	Write2(ptr, 0);
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out an ammo-info entry to 'ammo.dat'.
 */

void Ammo_info::write
	(
	int shapenum,
	std::ostream& out		// Write to here.
	)
	{
	uint8 buf[13];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	Write2(ptr, family_shape);
	Write2(ptr, type2);
	*ptr++ = damage;
	Write2(ptr, 0);			// Unknown.
	*ptr++ = damage_type<<4;
	*ptr++ = powers;
	Write2(ptr, 0);			// Unknown.
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out an armor-info entry to 'armor.dat'.
 */

void Armor_info::write
	(
	int shapenum,
	std::ostream& out		// Write to here.
	)
	{
	uint8 buf[10];			// Entry length.
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	*ptr++ = prot;			// Protection value.
	*ptr++ = 0;			// Unknown.
	*ptr++ = immune;		// Immunity flags.
       	Write4(ptr, 0);			// Last 5 are unknown/unused.
	*ptr = 0;
	out.write((char *) buf, sizeof(buf));
	}

/*
 *	Write out monster info. to 'monsters.dat'.
 */

void Monster_info::write
	(
	int shapenum,
	std::ostream& out
	)
	{
	uint8 buf[25];		// Entry length.
	memset(&buf[0], 0, sizeof(buf));
	uint8 *ptr = buf;
	Write2(ptr, shapenum);
	*ptr++ = strength << 2;
	*ptr++ = dexterity << 2;
	*ptr++ = (intelligence << 2) | (m_poison_safe ? 1 : 0);
	*ptr++ = (combat << 2) | alignment;
	*ptr++ = (armor << 4) | (m_splits ? 1 : 0) | (m_cant_die ? 2 : 0);
	*ptr++ = 0;			// Unknown.
	*ptr++ = (weapon << 4) | reach;
	*ptr++ = flags;			// Byte 9.
	*ptr++ = vulnerable;
	*ptr++ = immune;
	*ptr++ = (m_cant_yell ? (1<<5) : 0) |
		 (m_cant_bleed ? (1<<6) : 0);
	*ptr++ = 0;			// Unknown.
	*ptr++ = equip_offset;
	out.write((char *) buf, sizeof(buf));
	}

