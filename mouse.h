/*
 *	mouse.h - Mouse pointers.
 *
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "rect.h"
#include "dir.h"
#include "iwin8.h"
#include "vgafile.h"

/*
 *	Handle custom mouse pointers.
 */
class Mouse
{
protected:
	Shape_file pointers;		// Pointers from 'pointers.shp'.
	Game_window *gwin;		// Where to draw.
	Image_window8 *iwin;		// From gwin.
	Image_buffer *backup;		// Stores image below mouse shape.
	Rectangle box;			// Area backed up.
	Rectangle dirty;		// Dirty area from mouse move.
	int mousex, mousey;		// Last place where mouse was.
	int cur_framenum;		// Frame # of current shape.
	Shape_frame *cur;		// Current shape.
	unsigned char focus;		// 1 if we have focus.
	unsigned char onscreen;		// 1 if mouse is drawn on screen.
	static short short_arrows[8];	// Frame #'s of short arrows, indexed
					//   by direction (0-7, 0=east).
	static short med_arrows[8];	// Medium arrows.
	static short long_arrows[8];	// Frame #'s of long arrows.
	static short short_combat_arrows[8];	// Short red arrows
	static short med_combat_arrows[8];	// Medium red arrows
	void set_shape0(int framenum);	// Set shape without checking first.
	void Init();

public:
	enum Mouse_shapes		// List of shapes' frame #'s.
	{
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

	// Avatar speed (frame delay in 1/1000 secs.)
	enum Avatar_Speeds
	{
		slow_speed	= 166,
		medium_speed	= 100,
		fast_speed	= 50
	};
	int avatar_speed;

	static bool mouse_update;
	static Mouse* mouse;

	Mouse(Game_window *gw);
	Mouse(Game_window *gw, DataSource& shapes);
	~Mouse();
	
	void show();			// Paint it.
	void hide()			// Restore area under mouse.
		{
		if (onscreen)
			{
			onscreen = 0;
			iwin->put(backup, box.x, box.y);	
			dirty = box;	// Init. dirty to box.
			}
		}
	void set_shape(int framenum)	// Set to desired shape.
		{
		if (framenum != cur_framenum)
			set_shape0(framenum);
		}
	void set_shape(Mouse_shapes shape)
		{ set_shape(static_cast<int>(shape)); }
	Mouse_shapes get_shape()
		{ return (Mouse_shapes) cur_framenum; }
	void move(int x, int y);	// Move to new location (mouse motion).
	void blit_dirty()		// Blit dirty area.
		{ 			// But not in OpenGL.
		if (!GL_manager::get_instance())
			iwin->show(dirty.x - 1, dirty.y - 1, dirty.w + 2, 
							dirty.h + 2); 
		}
	void set_location(int x, int y);// Set to given location.
					// Flash desired shape for 1/2 sec.
	void flash_shape(Mouse_shapes flash);
					// Set to short arrow.
	int get_short_arrow(Direction dir)
		{ return (short_arrows[static_cast<int>(dir)]); }
					// Set to medium arrow.
	int get_medium_arrow(Direction dir)
		{ return (med_arrows[static_cast<int>(dir)]); }
					// Set to long arrow.
	int get_long_arrow(Direction dir)
		{ return (long_arrows[static_cast<int>(dir)]); }
					// Set to short combat mode arrow.
	int get_short_combat_arrow(Direction dir)
		{ return (short_combat_arrows[static_cast<int>(dir)]); }
					// Set to medium combat mode arrow.
	int get_medium_combat_arrow(Direction dir)
		{ return (med_combat_arrows[static_cast<int>(dir)]); }

	unsigned char is_onscreen() { return onscreen; }

	//inline const int get_mousex() const { return mousex; }
	//inline const int get_mousey() const { return mousey; }

	// Sets hand or speed cursors
	void set_speed_cursor();
};

#endif
