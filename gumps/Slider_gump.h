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

#ifndef _SLIDER_GUMP_H_
#define _SLIDER_GUMP_H_

#include "Modal_gump.h"

class Slider_button;

/*
 *	A slider for choosing a number.
 */
class Slider_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(Slider_gump,Modal_gump(0,0,0,0));

protected:
					// The arrows at each end:
	Slider_button *left_arrow, *right_arrow;
	int diamondx;			// Rel. pos. where diamond is shown.
	static short diamondy;
	int min_val, max_val;		// Max., min. values to choose from.
	int step_val;			// Amount to step by.
	int val;			// Current value.
	unsigned char dragging;		// 1 if dragging the diamond.
	int prev_dragx;			// Prev. x-coord. of mouse.
	void set_val(int newval);	// Set to new value.
					// Coords:
	static short leftbtnx, rightbtnx, btny;
	static short xmin, xmax;

	ShapeID	diamond;		// Diamond

public:
	Slider_gump(int mival, int mxval, int step, int defval);
	~Slider_gump();
	int get_val()			// Get last value set.
		{ return val; }
					// An arrow was clicked on.
	void clicked_arrow(Slider_button *arrow);

	void move_diamond(int dir);

					// Paint it and its contents.
	virtual void paint();
	virtual void close()
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void mouse_drag(int mx, int my);
	virtual void key_down(int chr, SDL_Event& ev); // Character typed.

	virtual void mousewheel_up();
	virtual void mousewheel_down();
};

#endif
