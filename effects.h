/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Effects.h - Special effects.
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

#ifndef INCL_EFFECTS
#define INCL_EFFECTS	1

#include "tqueue.h"

/*
 *	An animation from 'sprites.vga':
 */
class Sprites_effect : public Time_sensitive
	{
	int sprite_num;			// Which one.
	int frame_num;			// Current frame.
	int frames;			// # frames.
	int x, y;			// Location on screen.
public:
	Sprites_effect(int num, int xpos, int ypos);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	A text object is a message that stays on the screen for just a couple
 *	of seconds.  These are all kept in a single list, and managed by
 *	Game_window.
 */
class Text_effect : public Time_sensitive
	{
	Text_effect *next, *prev;	// All of them are chained together.
	char *msg;			// What to print.
	short tx, ty;			// Tile coords. within world of upper-
					//   left corner.
	short width, height;		// Dimensions of rectangle.
public:
	friend class Game_window;
	Text_effect(const char *m, int t_x, int t_y, int w, int h);
	virtual ~Text_effect()
		{ delete msg; }
					// At timeout, remove from screen.
	virtual void handle_event(unsigned long curtime, long udata);
	};

#endif

