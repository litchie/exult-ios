/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Mouse.h - Mouse pointers.
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
	
#ifndef INCL_MOUSE
#define INCL_MOUSE 1

#include "objs.h"
#include "vgafile.h"

class Image_buffer;

/*
 *	Handle custom mouse pointers.
 */
class Mouse
	{
	Shape_file pointers;		// Pointers from 'pointers.shp'.
	Game_window *gwin;		// Where to draw.
	Image_buffer *backup;		// Stores image below mouse shape.
	Rectangle box;			// Area backed up.
	int mousex, mousey;		// Last place where mouse was.
	Shape_frame *cur;		// Current shape.
	static short short_arrows[8];	// Frame #'s of short arrows, indexed
					//   by direction (0-7, 0=east).
	static short long_arrows[8];	// Frame #'s of long arrows.
public:
	Mouse(Game_window *gw);
	void move(int newx, int newy);	// Move to new location.
	void set_shape(int framenum)	// Set to desired shape.
		{ cur = pointers.get_frame(framenum); }
					// Set to short arrow.
	void set_short_arrow(Direction dir)
		{ set_shape(short_arrows[(int) dir]); }
					// Set to long arrow.
	void set_long_arrow(Direction dir)
		{ set_shape(long_arrows[(int) dir]); }
	};

#endif	/* INCL_MOUSE */
