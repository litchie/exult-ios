/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Mouse.cc - Mouse pointers.
 **
 **	Written: 3/15/2000 - JSF
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

#include "mouse.h"
#include "gamewin.h"
#include "fnames.h"

short Mouse::short_arrows[8] = {0, 1, 2, 3, 4, 5, 6, 7};	//++++Guessing.
short Mouse::long_arrows[8] = {8, 9, 10, 11, 12, 13, 14, 15};

/*
 *	Create.
 */

Mouse::Mouse
	(
	Game_window *gw			// Where to draw.
	) : pointers(POINTERS), gwin(gw), backup(0), cur(0), mousex(-1),
	    mousey(-1)
	{
	}

/*
 *	Move it.
 */

void Mouse::move
	(
	int x, int y			// New location.
	)
	{
					// Restore area under mouse.
	gwin->get_win()->put(backup, box.x, box.y);	
	int deltax = x - mousex, deltay = y - mousey;
	mousex = x;
	mousey = y;
					// Shift to new position.
	box.shift(deltax, deltay);
					// Save background.
	gwin->get_win()->get(backup, box.x, box.y);
					// Paint new location.
	gwin->paint_rle_shape(*cur, mousex, mousey);

	}

