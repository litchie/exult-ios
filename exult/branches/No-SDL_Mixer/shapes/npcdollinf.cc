/**
 **	npcdollinf.cc - NPC Paperdoll information from 'paperdol_info.txt'.
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
#include "npcdollinf.h"
using std::istream;

extern int get_skinvar(std::string key);

int Paperdoll_npc::read
	(
	std::istream& in,	// Input stream.
	int index,			// Line number (in some cases, this is the shapenum)
	int version,		// Data file version.
	bool bg				// Loading BG file.
	)
	{
	int shapenum;
	if (in.peek() == '%')
		{
		std::string key = ReadStr(in);
			// We need these for Exult, but not for ES.
			// For now, a compromise/hack in that ES defines
			// a version of this function which always returns
			// -1, while Exult has another which forwards to
			// Shapeinfo_lookup::get_skinvar
		shapenum = get_skinvar(key);
		if (shapenum < 0)
			return -1;	// Don't bother.
		}
	else
		shapenum = ReadInt(in);
	
	int sexflag = ReadInt(in);
	if (sexflag == -0xff)	// means delete entry.
		{
		set_invalid(true);
		return shapenum;
		}
	is_female = sexflag != 0;
	translucent = ReadInt(in) != 0;
	body_shape = ReadInt(in);
	body_frame = ReadInt(in);
	head_shape = ReadInt(in);
	head_frame = ReadInt(in);
	head_frame_helm = ReadInt(in);
	arms_shape = ReadInt(in);
	arms_frame[0] = ReadInt(in);
	arms_frame[1] = ReadInt(in);
	arms_frame[2] = ReadInt(in);
	gump_shape = ReadInt(in, -1);
	return shapenum;
	}
