/*
Copyright (C) 2000 The Exult Team

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

#ifndef _GUMP_H_
#define _GUMP_H_

#include "exceptions.h"
#include "rect.h"
#include "shapeid.h"

class Checkmark_button;
class Container_game_object;
class Game_object;
class Game_window;
class Gump_button;
class Gump_manager;

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump : public ShapeID
{
	UNREPLICATABLE_CLASS(Gump);

protected:
	Gump() : ShapeID() {   };
	Container_game_object *container;// What this gump shows.
	int x, y;			// Location on screen.
	unsigned char shapenum;
	Rectangle object_area;		// Area to paint objects in, rel. to
					// Where the 'checkmark' goes.
	Checkmark_button *check_button;
	void set_object_area(Rectangle area, int checkx = 8, int checky = 64);
public:
	Gump(Container_game_object *cont, int initx, int inity, int shnum,
								ShapeFile shfile = SF_GUMPS_VGA);
					// Create centered.
	Gump(Container_game_object *cont, int shnum, ShapeFile shfile = SF_GUMPS_VGA);
	virtual ~Gump();
	int get_x()			// Get coords.
		{ return x; }
	int get_y()
		{ return y; }
	void set_pos(int newx, int newy)// Set new spot on screen.
		{
		x = newx;
		y = newy;
		}
	void set_pos();			// Set centered.
	Container_game_object *get_container()
		{ return container; }
	virtual Container_game_object *find_actor(int mx, int my)
		{ return 0; }
	inline Container_game_object *get_cont_or_actor(int mx, int my)
	{
		Container_game_object *ret = find_actor(mx, my);
		if (ret) return ret;
		return get_container();
	}
					// Get screen rect. of obj. in here.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& ox, int& oy);
					// Find obj. containing mouse point.
	virtual Game_object *find_object(int mx, int my);
	virtual Rectangle get_dirty();		// Get dirty rect. for gump+contents.
	virtual Game_object *get_owner();// Get object this belongs to.
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Paint button.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
			int sx = -1, int sy = -1, bool dont_check = false,
						bool combine = false);
	virtual void remove(Game_object *obj);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
					// Close (and delete).
	virtual void close(Game_window *gwin);
					// update the gump, if required
	virtual void update_gump (Game_window *gwin) { }
					// Can be dragged with mouse
	virtual bool is_draggable() const { return true; }
					// Close on end_gump_mode
	virtual bool is_persistent() const { return false; }
					// Show the hand cursor
	virtual bool no_handcursor() const { return false; }

	virtual bool has_point(int x, int y);
	virtual Rectangle get_rect();

};

/*
 *	A generic gump used by generic containers:
 */
class Container_gump : public Gump
{
	UNREPLICATABLE_CLASS(Container_gump);

	void initialize(int shnum);		// Initialize object_area.

public:
	Container_gump(Container_game_object *cont, int initx, int inity, int shnum,
			ShapeFile shfile = SF_GUMPS_VGA)
		: Gump(cont, initx, inity, shnum, shfile)
	{
		initialize(shnum);
	}
					// Create centered.
	Container_gump(Container_game_object *cont, int shnum, ShapeFile shfile = SF_GUMPS_VGA)
		: Gump(cont, shnum, shfile)
	{
		initialize(shnum);
	}

};

#endif
