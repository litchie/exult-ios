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
#include "imagewin.h"

/*
 *	Handle custom mouse pointers.
 */
class Mouse
	{
	Shape_file pointers;		// Pointers from 'pointers.shp'.
	Game_window *gwin;		// Where to draw.
	Image_window *iwin;		// From gwin.
	Image_buffer *backup;		// Stores image below mouse shape.
	Rectangle box;			// Area backed up.
	int mousex, mousey;		// Last place where mouse was.
	int cur_framenum;		// Frame # of current shape.
	Shape_frame *cur;		// Current shape.
	unsigned char focus;		// 1 if we have focus.
	unsigned char onscreen;		// 1 if mouse is drawn on screen.
	static short short_arrows[8];	// Frame #'s of short arrows, indexed
					//   by direction (0-7, 0=east).
	static short med_arrows[8];	// Medium arrows.
	static short long_arrows[8];	// Frame #'s of long arrows.
	void set_shape0(int framenum);	// Set shape without checking first.
public:
	Mouse(Game_window *gw);
	~Mouse();
	enum Mouse_shapes {		// List of shapes' frame #'s.
		dontchange = 1000,	// Flag to not change.
		hand = 0,
		redx = 1,
		greenselect = 2,	// For modal select.
		tooheavy = 3,
		outofrange = 4,
		outofammo = 5,
		wontfit = 6,
		hourglass = 7,
		greensquare = 23,
		blocked = 49
		};
	void show();			// Paint it.
	void hide()			// Restore area under mouse.
		{
		if (onscreen)
			{
			onscreen = 0;
			iwin->put(backup, box.x, box.y);	
			}
		}
	void set_shape(int framenum)	// Set to desired shape.
		{
		if (framenum != cur_framenum)
			set_shape0(framenum);
		}
	void set_shape(Mouse_shapes shape)
		{ set_shape((int) shape); }
	Mouse_shapes get_shape()
		{ return (Mouse_shapes) cur_framenum; }
	void move(int x, int y)		// Move to new location (mouse motion).
		{
#if DEBUG
		if (onscreen)
			cerr << "Trying to move mouse while onscreen!\n";
#endif
					// Shift to new position.
		box.shift(x - mousex, y - mousey);
		mousex = x;
		mousey = y;
		}
	void set_location(int x, int y);// Set to given location.
					// Set to short arrow.
	void set_short_arrow(Direction dir)
		{ set_shape(short_arrows[(int) dir]); }
					// Set to medium arrow.
	void set_medium_arrow(Direction dir)
		{ set_shape(med_arrows[(int) dir]); }
					// Set to long arrow.
	void set_long_arrow(Direction dir)
		{ set_shape(long_arrows[(int) dir]); }

	unsigned char is_onscreen() { return onscreen; }
	};

#endif	/* INCL_MOUSE */
