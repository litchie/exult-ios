/*
Copyright (C) 2000-2002 The Exult Team

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

#ifndef _MODAL_GUMP_H_
#define _MODAL_GUMP_H_

#include "Gump.h"

/*
 *	A modal gump object represents a 'dialog' that grabs the mouse until
 *	the user clicks okay.
 */
class Modal_gump : public Gump
{
	UNREPLICATABLE_CLASS(Modal_gump);

protected:
	bool done;			// true when user clicks checkmark.
	bool restore_background;
	Gump_button *pushed;		// Button currently being pushed.

public:
	Modal_gump(Container_game_object *cont, int initx, int inity, 
			   int shnum, ShapeFile shfile = SF_GUMPS_VGA)
		: Gump(cont, initx, inity, shnum, shfile), done(false), restore_background(true), pushed(0)
		{  }
					// Create centered.
	Modal_gump(Container_game_object *cont, int shnum, 
			   ShapeFile shfile = SF_GUMPS_VGA)
		: Gump(cont, shnum, shfile), done(false), restore_background(true), pushed(0)
		{  }
	bool is_done()
		{ return done; }
	bool want_restore_background()
		{ return restore_background; }
					// Handle events:
	virtual void mouse_down(int mx, int my) = 0;
	virtual void mouse_up(int mx, int my) = 0;
	virtual void mousewheel_down() { }
	virtual void mousewheel_up() { }
	virtual void mouse_drag(int mx, int my)
		{  }
	virtual void key_down(int chr) // Key pressed
		{  }
    virtual void text_input(int chr, int unicode)	// Character typed
        {  }
};

#endif
