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
	    mousey(-1), focus(0)
	{
					// Get max. backup size.
	int cnt = pointers.get_num_frames();
	int maxw = 0, maxh = 0;
	for (int i = 0; i < cnt; i++)
		{
		Shape_frame *frame = pointers.get_frame(i);
		int w = frame->get_width(), h = frame->get_height();
		if (w > maxw)
			maxw = w;
		if (h > maxh)
			maxh = h;
		}
					// Create backup buffer.
	backup = gwin->get_win()->create_buffer(maxw, maxh);
	box.w = maxw;
	box.h = maxh;
	set_short_arrow(east);		// +++++For now.
	}

/*
 *	Set to new shape.
 */

void Mouse::set_shape
	(
	int framenum
	)
	{
	if (focus)			// Restore area under mouse.
		gwin->get_win()->put(backup, box.x, box.y);	
	cur = pointers.get_frame(framenum); 
					// Set backup box to cover mouse.
	box.x = mousex - cur->get_xleft();
	box.y = mousey - cur->get_yabove();
	if (!focus)
		return;
					// Save background.
	gwin->get_win()->get(backup, box.x, box.y);
					// Paint new location.
	gwin->paint_rle_shape(*cur, mousex, mousey);
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

/*
 *	Gain mouse coverage.
 */

void Mouse::gain_focus
	(
	int x, int y			// Mouse position.
	)
	{
	if (!focus)
		{
		focus = 1;
		mousex = x;
		mousey = y;
		box.x = mousex - cur->get_xleft();
		box.y = mousey - cur->get_yabove();
					// Save background.
		gwin->get_win()->get(backup, box.x, box.y);
					// Paint new location.
		gwin->paint_rle_shape(*cur, mousex, mousey);
		}
	}

/*
 *	Lose mouse coverage.
 */

void Mouse::lose_focus
	(
	)
	{
	if (focus)
		{
		focus = 0;
					// Restore area under mouse.
		gwin->get_win()->put(backup, box.x, box.y);	
		}
	}
