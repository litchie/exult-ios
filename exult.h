/*
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EXULT_H
#define EXULT_H

#include "mouse.h"

class Actor;
class Cheat;
class Configuration;
class Game_window;
class KeyBinder;
class Tile_coord;
class Paintable;

/*
 *	Get a click, or, optionally, a keyboard char.
 *
 *	Output:	0 if user hit ESC.
 *		Chr gets keyboard char., or 0 if it's was a mouse click.
 */

extern int Get_click
	(
	int& x, int& y,			// Location returned (if not ESC).
	Mouse::Mouse_shapes shape,	// Mouse shape to use.
	char *chr = 0,			// Char. returned if not null.
	bool drag_ok = false,		// Can drag while here.
	Paintable *paint = 0		// Paint over everything else.
	);

/*
 *	Make a screenshot of the current screen display
 */
extern void make_screenshot
	(
	bool silent	= false		// If false, will display a success/failure message
	);

/*
 *	Wait for someone to stop walking.  If a timeout is given, at least
 *	one animation cycle will still always occur.
 */

extern void Wait_for_arrival
	(
	Actor *actor,			// Whom to wait for.
	Tile_coord dest,		// Where he's going.
	long maxticks = 0		// Max. # msecs. to wait, or 0.
	);

extern void change_gamma (bool down);
extern void increase_resolution();
extern void decrease_resolution();

typedef enum 
{
	QUIT_TIME_NO = 0,
	QUIT_TIME_YES = 1,
	QUIT_TIME_RESTART = 2
} quitting_time_enum;

extern KeyBinder *keybinder;
extern Configuration *config;

extern quitting_time_enum quitting_time;


#endif
