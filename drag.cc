/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Drag.cc - Dragging objects in Game_window.
 **
 **	Written: 3/2/2000 - JSF
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

#include <iostream.h>	/* Debugging */
#include "gamewin.h"

/*
 *	Begin a possible drag when the mouse button is depressed.
 *
 *	Output:	1 if object selected for dragging, else 0.
 */

int Game_window::start_dragging
	(
	int x, int y			// Position in window.
	)
	{
	dragging = 0;
	Game_object *found[100];	// See what was clicked on.
	int cnt = find_objects(x, y, found);
	if (!cnt)
		return (0);
	dragging = found[cnt - 1];	// Store object.
	dragging_mousex = x;
	dragging_mousey = y;
					// Get coord. where painted.
	get_shape_location(dragging, dragging_paintx, dragging_painty);
	dragging_cx = dragging->get_cx();
	dragging_cy = dragging->get_cy();
	dragging_rect = Rectangle(0, 0, 0, 0);
	return (1);
	}

/*
 *	Mouse was moved while dragging.
 */

void Game_window::drag
	(
	int x, int y			// Mouse pos. in window.
	)
	{
	const int pad = 8;
	if (!dragging)
		return;
	if (dragging_rect.w == 0)
		{			// First motion.
					// Store original pos. on screen.
		dragging_rect = get_shape_rect(dragging);
		dragging_rect.x -= pad;	// Make a little bigger.
		dragging_rect.y -= pad;
		dragging_rect.w += 2*pad;
		dragging_rect.h += 2*pad;
					// Remove from actual position.
		get_objects(dragging_cx, dragging_cy)->remove(dragging);
		}
	win->set_clip(0, 0, get_width(), get_height());
	Rectangle rect = clip_to_win(dragging_rect);
	paint(rect);			// Paint over last place shown.
	int deltax = x - dragging_mousex, deltay = y - dragging_mousey;
	dragging_mousex = x;
	dragging_mousey = y;
					// Shift to new position.
	dragging_rect.shift(deltax, deltay);
	dragging_paintx += deltax;
	dragging_painty += deltay;
	paint_shape(win, dragging_paintx, dragging_painty, 
			dragging->get_shapenum(), dragging->get_framenum());
	win->clear_clip();
	}

/*
 *	Mouse was released, so drop object.
 */

void Game_window::drop_dragged
	(
	int x, int y			// Mouse pos.
	)
	{
	if (!dragging)
		return;
	drag(x, y);			// Get object to mouse pos.
//	Chunk_object_list *chunk = get_objects(dragging_cx, dragging_cy);
	//+++++For now, just put it back.
	get_objects(dragging_cx, dragging_cy)->add(dragging);
	dragging = 0;
	paint();
	}
