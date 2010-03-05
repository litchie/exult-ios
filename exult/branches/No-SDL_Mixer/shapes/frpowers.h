/**
 **	frpowers.h - Frame-based powers from 'shape_info.txt'.
 **
 **	Written: 06/01/2008 - Marzo
 **/

#ifndef INCL_FRPOWERS_H
#define INCL_FRPOWERS_H	1

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
 *	Information about frame names.
 *	This is meant to be stored in a totally ordered vector.
 */

namespace Frame_powers
	{
	enum Enum_Frame_power_flags
		{
		fp_poison_safe = 0,
		fp_charm_safe,
		fp_sleep_safe,
		fp_paralysis_safe,
		fp_curse_safe,
		fp_power_safe,
		fp_death_safe,
		fp_cant_die,
		fp_cold_immune,
		fp_doesnt_eat,
		fp_swamp_safe
		};
	enum Enum_Frame_powers
		{
		poison_safe = ((unsigned)1 << fp_poison_safe),
		charm_safe = ((unsigned)1 << fp_charm_safe),
		sleep_safe = ((unsigned)1 << fp_sleep_safe),
		paralysis_safe = ((unsigned)1 << fp_paralysis_safe),
		curse_safe = ((unsigned)1 << fp_curse_safe),
		power_safe = ((unsigned)1 << fp_power_safe),
		death_safe = ((unsigned)1 << fp_death_safe),
		cant_die = ((unsigned)1 << fp_cant_die),
		cold_immune = ((unsigned)1 << fp_cold_immune),
		doesnt_eat = ((unsigned)1 << fp_doesnt_eat),
		swamp_safe = ((unsigned)1 << fp_swamp_safe)
		};
	}

class Frame_powers_info : public Base_info
	{
	short			frame;		// Frame for which this applies or -1 for any.
	unsigned short	powers;		// Bit field with the powers granted.
public:
	friend class Shape_info;
	Frame_powers_info()
		: Base_info()
		{  }
	Frame_powers_info(short f, unsigned short pow, bool p = false,
			bool m = false, bool s = false, bool inv = false)
		{ set(f, pow, p, m, s, inv); }
	Frame_powers_info(const Frame_powers_info& other)
		: frame(other.frame), powers(other.powers)
		{ info_flags = other.info_flags; }
		// Read in from file.
	int read(std::istream& in, int index, int version, bool bg);
					// Write out.
	void write(std::ostream& out, int shapenum, bool bg);
	void set(short f, unsigned short pow, bool p = false,
			bool m = false, bool s = false, bool inv = false)
		{
		frame = f; powers = pow;
		set_patch(p); set_modified(m); set_static(s); set_invalid(inv);
		}
	void invalidate()
		{ powers = 0; set_invalid(true); }
	int get_frame() const
		{ return frame; }
	bool get_power(int tf) const
		{ return (powers & (1 << tf)) != 0; }
	int get_powers() const
		{ return powers; }
	void set_powers(unsigned short pow)
		{
		if (powers != pow)
			{
			set_modified(true);
			powers = pow;
			}
		}
	bool operator<(const Frame_powers_info& other) const
		{
		unsigned short frame1 = (unsigned short)frame,
				frame2 = (unsigned short)other.frame;
		return (frame1 < frame2);
		}
	bool operator==(const Frame_powers_info& other) const
		{ return this == &other || (!(*this < other) && !(other < *this)); }
	bool operator!=(const Frame_powers_info& other) const
		{ return !(*this == other); }
	void set(const Frame_powers_info& other)
		{	// Assumes *this == other.
			// No need to guard against self-assignment.
			// Do NOT copy modified or static flags.
		set_patch(other.from_patch());
		set_invalid(other.is_invalid());
		set_powers(other.powers);
		}
	static const int get_entry_size()
		{ return -1; }
	};

#endif
