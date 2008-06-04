/**
 **	armorinf.h - Information from 'armor.dat'.
 **
 **	Written: 06/01/2008 - Marzo
 **/

#ifndef INCL_ARMORINF_H
#define INCL_ARMORINF_H	1

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

#include "baseinf.h"
using std::istream;

class Shape_info;

/*
 *	Armor:
 */
class Armor_info : public Base_info
	{
	unsigned char prot;		// Protection value.
	unsigned char immune;		// Weapon_data::damage_type bits.
public:
	friend class Shape_info;
	Armor_info() : Base_info() {  }
		// Read in from file.
	int read(std::istream& in, int index, int version, bool bg);
					// Write out.
	void write(std::ostream& out, int shapenum, bool bg);
	static const int get_entry_size()
		{ return 10; }
	unsigned char get_prot() const
		{ return prot; }
	void set_prot(unsigned char p)
		{
		if (prot != p)
			{
			set_modified(true);
			prot = p;
			}
		}
	unsigned char get_immune() const
		{ return immune; }
	void set_immune(unsigned char i)
		{
		if (immune != i)
			{
			set_modified(true);
			immune = i;
			}
		}
	static int get_info_flag()
		{ return 4; }
	int get_base_strength();
	int get_base_xp_value()
		{ return prot; }
	};

#endif
