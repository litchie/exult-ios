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

#ifndef _GUMPSHAPEFILE
#define _GUMPSHAPEFILE
enum Gumpshapefile { GSF_GUMPS_VGA, GSF_EXULT_FLX };
#endif

class Gump;
class Game_window;

/*
 *	A gump widget, such as a button or text field:
 */
class Gump_widget
{
	UNREPLICATABLE_CLASS(Gump_widget);

protected:
	Gump_widget() : parent(0) {  }
	Gump *parent;		// Who this is in.
	Gumpshapefile shapefile;
	int shapenum;			// In "gumps.vga".
	int framenum;			// Frame # (usually 0) when unpushed.
	short x, y;			// Coords. relative to parent.

public:
	friend class Gump;
	friend class Spellbook_gump;
	friend class Spellscroll_gump;
	Gump_widget(Gump *par, int shnum, int px, int py,
				Gumpshapefile shfile = GSF_GUMPS_VGA)
		: parent(par), shapenum(shnum), framenum(0), x(px), y(py),
		shapefile(shfile)
		{  }
					// Is a given point on the widget?
	int on_widget(Game_window *gwin, int mx, int my);
};

#endif
