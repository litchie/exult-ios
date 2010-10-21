/**
 **	sfxinf.cc - Sound Effect information from 'shape_info.txt'.
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
#include "exult_constants.h"
#include "sfxinf.h"
using std::istream;

bool SFX_info::read
	(
	std::istream& in,	// Input stream.
	int version,		// Data file version.
	Exult_Game game		// Loading BG file.
	)
	{
	sfxnum = ReadInt(in);
	if (sfxnum == -0xff)	// means delete entry.
		{
		set_invalid(true);
		return true;
		}
	if (version >= 2)
		{
		chance = ReadInt(in, 100);
		if (chance < 1 || chance > 100)
			chance = 100;
		range = ReadInt(in, 1);
		if (range < 1)
			range = 1;		// Sensible default.
		random = ReadInt(in, 0) != 0;
		extra = ReadInt(in, -1);
		}
	return true;
	}
