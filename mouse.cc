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

short Mouse::short_arrows[8] = {10, 9, 8, 15, 14, 13, 12, 11};
short Mouse::med_arrows[8] = {18, 17, 16, 23, 22, 21, 20, 19};
short Mouse::long_arrows[8] = {26, 25, 24, 31, 30, 29, 28, 27};

/*
 *	Create.
 */

Mouse::Mouse
	(
	Game_window *gw			// Where to draw.
	) : pointers(POINTERS), gwin(gw), backup(0), cur(0), mousex(-1),
	    mousey(-1), iwin(gwin->get_win()), cur_framenum(0)
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
	backup = iwin->create_buffer(maxw, maxh);
	box.w = maxw;
	box.h = maxh;
	set_short_arrow(east);		// +++++For now.
	}

/*
 *	Delete.
 */

Mouse::~Mouse
	(
	)
	{
	delete backup;
	}

/*
 *	Show the mouse.
 */

void Mouse::show
	(
	)
	{
	if (!onscreen)
		{
		onscreen = 1;
					// Save background.
		iwin->get(backup, box.x, box.y);
					// Paint new location.
		gwin->paint_rle_shape(*cur, mousex, mousey);
		}
	}

/*
 *	Set to new shape.  Should be called after checking that frame #
 *	actually changed.
 */

void Mouse::set_shape0
	(
	int framenum
	)
	{
	cur_framenum = framenum;
	cur = pointers.get_frame(framenum); 
					// Set backup box to cover mouse.
	box.x = mousex - cur->get_xleft();
	box.y = mousey - cur->get_yabove();
	}

/*
 *	Set to an arbitrary location.
 */

void Mouse::set_location
	(
	int x, int y			// Mouse position.
	)
	{
	mousex = x;
	mousey = y;
	box.x = mousex - cur->get_xleft();
	box.y = mousey - cur->get_yabove();
	}

