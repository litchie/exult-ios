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

#ifndef _GUMPSHAPEFILE
#define _GUMPSHAPEFILE
enum Gumpshapefile { GSF_GUMPS_VGA, GSF_EXULT_FLX };
#endif

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump : public ShapeID
{
	UNREPLICATABLE_CLASS(Gump);

protected:
	Gump() : ShapeID(), paperdoll_shape (false) {   };
	Gump *next;		// ->next to draw.
	Container_game_object *container;// What this gump shows.
	int x, y;			// Location on screen.
	Gumpshapefile shapefile;
	unsigned char shapenum;
	Rectangle object_area;		// Area to paint objects in, rel. to
					// Where the 'checkmark' goes.
	Checkmark_button *check_button;
	void initialize();		// Initialize object_area.
	void initialize2();		// Initialize object_area (paperdoll).
	bool paperdoll_shape;

public:
	Gump(Container_game_object *cont, int initx, int inity, 
								int shnum, bool pdoll = false,
								Gumpshapefile shfile = GSF_GUMPS_VGA);
					// Create centered.
	Gump(Container_game_object *cont, int shnum,
								Gumpshapefile shfile = GSF_GUMPS_VGA);
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
					// Append to end of chain.
	void append_to_chain(Gump *& chain);
					// Remove from chain.
	void remove_from_chain(Gump *& chain);
	Gump *get_next()		// (Chain ends with ->next == 0.)
		{ return next; }
	Container_game_object *get_container()
		{ return container; }
					// Get screen rect. of obj. in here.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& ox, int& oy);
					// Find obj. containing mouse point.
	virtual Game_object *find_object(int mx, int my);
	Rectangle get_dirty();		// Get dirty rect. for gump+contents.
	virtual Game_object *get_owner();// Get object this belongs to.
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Paint button.
	virtual void paint_button(Game_window *gwin, Gump_button *btn);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
						int sx = -1, int sy = -1);
	virtual void remove(Game_object *obj);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
					// Close (and delete).
	virtual void close(Game_window *gwin);
					// Use the paperdoll shapes?
	bool is_paperdoll() const { return paperdoll_shape; }

	virtual int get_shapefile() const { return shapefile; }
};

#endif
