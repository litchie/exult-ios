/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Spells.cc - Spellbook handling.
 **
 **	Written: 5/29/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

/*
 *	Defines in 'gumps.vga':
 */
const int SPELLS = 33;			// First group of 9 spells.
const int TURNINGPAGE = 41;		// Animation?? (4 frames).
const int BOOKMARK = 42;		// Red ribbon, 5 frames.
const int SPELLBOOK = 43;		// The book itself.
const int LEFTPAGE = 44;		// At top-left of left page.
const int RIGHTPAGE = 45;		// At top-right of right page.

// +++++JUST want to get these down now.  May make them methods later:

/*
 *	Get shape, frame for a given spell #.  There are 8 shapes, each
 *	containing 9 frames, where frame # = spell circle #.
 */
inline int Get_spell_gump_shape
	(
	int spell,			// Spell # (as used in Usecode).
	int& shape,			// Shape returned.
	int& frame			// Frame returned.
	)
	{
	if (spell < 0 || spell >= 0x48)
		return 0;
	shape = spell%8;
	frame = spell/8;
	return (1);
	}

/*
 *	Get usecode function for a given spell:
 */
int Get_usecode(int spell)
	{ return 0x640 + spell; }
