/**
 **	Spellbook.cc - Spellbook object.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "spellbook.h"
#include "gamewin.h"
#include "gamemap.h"
#include "utils.h"
#include "game.h"
#include "Gump_manager.h"
#include "databuf.h"

using std::memcpy;
using std::ostream;

/*
 *	Create a spellbook from Ireg data.
 */

Spellbook_object::Spellbook_object
	(
	int shapenum, int framenum,
	unsigned int shapex, unsigned int shapey, 
	unsigned int lft, 
	unsigned char *c,		// Circle spell flags.
	uint32 f			// Flags (unknown).
	) : Ireg_game_object(shapenum, framenum,
			shapex, shapey, lft), flags(f), bookmark(-1)
	{
	memcpy(circles, c, sizeof(circles));
	}

/*
 *	Add a spell.
 *
 *	Output:	0 if already have it, 1 if successful.
 */

int Spellbook_object::add_spell
	(
	int spell			// 0-71
	)
	{
	int circle = spell/8;
	int num = spell%8;		// # within circle.
	if (circles[circle] & (1<<num))
		return 0;		// Already have it.
	circles[circle] |= (1<<num);
	return 1;
	}

/*
 *	Show book when double-clicked.
 */

void Spellbook_object::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->get_gump_man()->add_gump(this, Game::get_game_type() == BLACK_GATE ? 43 : 38);
	}

/*
 *	Write out.
 */

void Spellbook_object::write_ireg
	(
	DataSource *out
	)
	{
	unsigned char buf[19];		// 18-byte entry + length-byte.
	buf[0] = 18;
	uint8 *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
	memcpy(ptr, &circles[0], 5);	// Store the way U7 does it.
	ptr += 5;
	*ptr++ = (get_lift()&15)<<4;	// Low bits?++++++
	memcpy(ptr, &circles[5], 4);	// Rest of spell circles.
	ptr += 4;
	Write4(ptr, flags);
	out->write((char*)buf, sizeof(buf));
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
	}

