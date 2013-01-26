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

#ifndef _GUMP_WIDGET_H_
#define _GUMP_WIDGET_H_

#include "exceptions.h"
#include "gump_types.h"
#include "shapeid.h"
#include "rect.h"

class Gump;
class Game_window;

/*
 *  A gump widget, such as a button or text field:
 */
class Gump_widget : public ShapeID {
	UNREPLICATABLE_CLASS_I(Gump_widget, ShapeID());

protected:
	Gump_widget() : parent(0) {  }
	Gump *parent;       // Who this is in.
	short x, y;         // Coords. relative to parent.

public:
	friend class Gump;
	friend class Spellbook_gump;
	friend class Spellscroll_gump;
	Gump_widget(Gump *par, int shnum, int px, int py, ShapeFile shfile = SF_GUMPS_VGA)
		: ShapeID(shnum, 0, shfile), parent(par), x(px), y(py)
	{  }
	virtual Gump_widget *clone(Gump *par) {
		return 0;
	}
	virtual ~Gump_widget() {  }
	// Is a given point on the widget?
	virtual int on_widget(int mx, int my);
	virtual int on_button(int mx, int my) {
		return 0;
	}
	virtual void paint();

	virtual Rectangle get_rect();
	// update the widget, if required
	virtual void update_widget() { }

	virtual bool is_draggable() {
		return true;
	}

};

#endif
