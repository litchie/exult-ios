/*
Copyright (C) 2000-2013 The Exult Team

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

#ifndef _TEXT_GUMP_H_
#define _TEXT_GUMP_H_

#include "Gump.h"

/*
 *  A text gump is the base class for books and scrolls.
 */
class Text_gump : public Gump {
	UNREPLICATABLE_CLASS_I(Text_gump, Gump());

protected:
	char *text;         // The text.
	int textlen;            // Length of text.
	int curtop;         // Offset of top of current page.
	int curend;         // Offset past end of current page(s).
	int font;       // The shape in fonts.vga to use

public:
	Text_gump(int shapenum, int fnt = 4) : Gump(0, shapenum),
		text(0), textlen(0), curtop(0), curend(0), font(fnt)
	{  }
	virtual ~Text_gump() {
		delete [] text;
	}
	void add_text(const char *str); // Append text.
	int paint_page(Rectangle const &box, int start);
	// Next page of book/scroll.
	int show_next_page();
};

#endif
