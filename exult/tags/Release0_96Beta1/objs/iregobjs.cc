/**
 **	Iregobjs.cc - Ireg (moveable) game objects.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include "contain.h"
#include "gamewin.h"
#include "chunks.h"
#include "cheat.h"

using std::ostream;

/*
 *	Paint at given spot in world.
 */

void Ireg_game_object::paint
	(
	Game_window *gwin
	)
	{
	int x, y;
	gwin->get_shape_location(this, x, y);
	if (flags & (1L << Obj_flags::invisible))
		gwin->paint_invisible(x, y, this->get_shape());
	else
		gwin->paint_shape(x, y, *this);
	}

/*
 *	Move to a new absolute location.  This should work even if the old
 *	location is invalid (cx=cy=255).
 */

void Ireg_game_object::move
	(
	int newtx, 
	int newty, 
	int newlift
	)
	{
	if (owner)			// Watch for this.
		{
		owner->remove(this);
		set_invalid();		// So we can safely move it back.
		}
	Game_object::move(newtx, newty, newlift);
	}

/*
 *	Remove an object from its container, or from the world.
 *	The object is deleted.
 */

void Ireg_game_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	if (owner)			// In a bag, box, or person.
		owner->remove(this);
	else				// In the outside world.
		{
		Map_chunk *chunk = 
			Game_window::get_game_window()->get_chunk_safely(
								cx, cy);
		if (chunk)
			chunk->remove(this);
		}
	if (!nodel)
	{
		cheat.clear_this_grabbed_actor((Actor*)this);	// Could be an actor
		Game_window::get_game_window()->delete_object(this);
	}
	}

/*
 *	Can this be dragged?
 */

int Ireg_game_object::is_dragable
	(
	) const
	{
	Game_window *gwin = Game_window::get_game_window();
					// 0 weight means 'too heavy'.
	return gwin->get_info(this).get_weight() > 0;
	}

/*
 *	Write out.
 */

void Ireg_game_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[11];		// 10-byte entry + length-byte.
	buf[0] = 10;
	write_common_ireg(&buf[1]);
	buf[5] = (get_lift()&15)<<4;
	buf[6] = get_quality();
					// Special case for 'quantity' items:
	if (get_flag(Obj_flags::okay_to_take) &&
	    Game_window::get_game_window()->get_info(this).has_quantity())
		buf[6] |= 0x80;

	buf[7] = (get_flag(Obj_flags::is_temporary) != 0);
	out.write((char*)buf, sizeof(buf));
					// Write scheduled usecode.
	Game_window::write_scheduled(out, this);	
	}
