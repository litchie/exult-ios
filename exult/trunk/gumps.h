/**
 **	Gumps.h - Open containers and their contents.
 **
 **	Written: 3/4/2000 - JSF
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

#ifndef INCL_GUMPS
#define INCL_GUMPS

#include "objs.h"

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump_object : public ShapeID
	{
	Gump_object *next;		// ->next to draw.
	Container_game_object *container;// What this gump shows.
	int x, y;			// Location on screen.
	unsigned char shapenum;
	Rectangle object_area;		// Area to paint objects in, rel. to
					//   hot spot of Gump_object.
public:
	Gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);
	int get_x()			// Get coords.
		{ return x; }
	int get_y()
		{ return y; }
					// Append to end of chain.
	void append_to_chain(Gump_object *& chain);
					// Remove from chain.
	void remove_from_chain(Gump_object *& chain);
	Gump_object *get_next()		// (Chain ends with ->next == 0.)
		{ return next; }
	Container_game_object *get_container()
		{ return container; }
	void add(Game_object *obj)
		{ container->add(obj); }
	void remove(Game_object *obj)
		{ container->remove(obj); }
					// Get screen rect. of obj. in here.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& ox, int& oy)
		{
		ox = x + object_area.x + obj->cx,
		oy = y + object_area.y + obj->cy;
		}
					// Find objs. containing mouse point.
	int find_objects(Game_window *gwin, int mx, int my,
						Game_object **list);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

#endif	/* INCL_GUMPS */
