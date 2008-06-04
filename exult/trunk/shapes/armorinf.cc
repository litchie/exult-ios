/**
 **	armorinf.cc - Information from 'armor.dat'.
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

#include "utils.h"
#include "armorinf.h"
using std::istream;

int Armor_info::get_base_strength()
	{
		// ++++The strength values are utter guesses.
	int strength = prot;
	if (immune)	// Double strength for any immunities? Give bonus for each?
		strength *= 2;
	return strength;
	}

/*
 *	Read in an armor-info entry from 'armor.dat'.
 *
 *	Output:	Shape # this entry describes.
 */

int Armor_info::read
	(
	std::istream& in,	// Input stream.
	int index,			// Line number (in some cases, this is the shapenum)
	int version,		// Data file version.
	bool bg				// Loading BG file.
	)
	{
	uint8 buf[10];			// Entry length.
	in.read((char *) buf, sizeof(buf));
	uint8 *ptr = buf;
	int shapenum = Read2(ptr);	// Bytes 0-1.
	if (buf[9] == 0xff)	// means delete entry.
		{
		set_invalid(true);
		return shapenum;
		}
	prot = *ptr++;			// Protection value.
	ptr++;				// Unknown.
	immune = *ptr++;		// Immunity flags.
					// Last 5 are unknown/unused.
	return shapenum;
	}
