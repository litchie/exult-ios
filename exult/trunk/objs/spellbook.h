/**
 **	Spellbook.h - Spellbook object.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team.

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

#ifndef INCL_SPELLBOOK
#define INCL_SPELLBOOK	1

#include "iregobjs.h"

/*
 *	A spellbook:
 */
class Spellbook_object : public Ireg_game_object
	{
	unsigned char circles[9];	// Spell-present flags for each circle.
	uint32 flags;		// Unknown at present.
	int bookmark;			// Spell # that bookmark is on, or -1.
public:
	friend class Spellbook_gump;
					// Create from ireg. data.
	Spellbook_object(int shapenum, int framenum, unsigned int shapex,
		unsigned int shapey, unsigned int lft, unsigned char *c,
		uint32 f);
	int add_spell(int spell);	// Add a spell.
					// Run usecode function.
	virtual void activate(int event = 1);
					// Write out to IREG file.
	virtual void write_ireg(DataSource* out);
				// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();
	};

#endif
