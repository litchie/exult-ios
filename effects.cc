/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Effects.cc - Special effects.
 **
 **	Written: 5/25/2000 - JSF
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

#include "effects.h"
#include "gamewin.h"

/*
 *	Create an animation from the 'sprites.vga' file.
 */

Sprites_effect::Sprites_effect
	(
	int num,			// Index.
	int px, int py			// Screen location.
	) : sprite_num(num), frame_num(0), tx(px), ty(py)
	{
	Game_window *gwin = Game_window::get_game_window();
	frames = gwin->get_sprite_num_frames(num);
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
	}

/*
 *	Animation.
 */

void Sprites_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 50;		// Delay between frames.
	Game_window *gwin = Game_window::get_game_window();
	if (frame_num == frames)	// At end?
		{			// Remove & delete this.
		gwin->remove_effect(this);
		gwin->paint();
		return;
		}
	Sprites_effect::paint(gwin);	// Render.
	gwin->set_painted();
	frame_num++;			// Next frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Sprites_effect::paint
	(
	Game_window *gwin
	)
	{
	gwin->paint_sprite((tx - gwin->get_scrolltx())*tilesize,
		(ty - gwin->get_scrollty())*tilesize, sprite_num, frame_num);
	}

/*
 *	Create a text object.
 */

Text_effect::Text_effect
	(
	const char *m, 			// A copy is made.
	int t_x, int t_y, 		// Abs. tile coords.
	int w, int h
	) : msg(strdup(m)), tx(t_x), ty(t_y), width(w), height(h)
	{
	}

/*
 *	Remove from screen.
 */

void Text_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = (Game_window *) udata;
					// Repaint slightly bigger rectangle.
	Rectangle rect((tx - gwin->get_scrolltx() - 1)*tilesize,
		       (ty - gwin->get_scrollty() - 1)*tilesize,
			width + 2*tilesize, height + 2*tilesize);
					// Intersect with screen.
	rect = gwin->clip_to_win(rect);
	gwin->remove_effect(this);	// Remove & delete this.
	if (rect.w > 0 && rect.h > 0)	// Watch for negatives.
		gwin->paint(rect.x, rect.y, rect.w, rect.h);

	}

/*
 *	Render.
 */

void Text_effect::paint
	(
	Game_window *gwin
	)
	{
	const char *ptr = msg;
	if (*ptr == '@')
		ptr++;
	int len = strlen(ptr);
	if (ptr[len - 1] == '@')
		len--;
	gwin->paint_text(0, ptr, len, (tx - gwin->get_scrolltx())*tilesize,
				(ty - gwin->get_scrollty())*tilesize);
	}
