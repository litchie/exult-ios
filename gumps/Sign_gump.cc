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

#include "Sign_gump.h"
#include "gamewin.h"


/*
 *	Create a sign gump.
 */

Sign_gump::Sign_gump
	(
	int shapenum,
	int nlines			// # of text lines.
	) : Gump(0, shapenum), num_lines(nlines)
{
	lines = new std::string[num_lines];
}

/*
 *	Delete sign.
 */

Sign_gump::~Sign_gump
	(
	)
{
	delete [] lines;
}

/*
 *	Add a line of text.
 */

void Sign_gump::add_text
	(
	int line,
	const std::string &txt
	)
{
	if (line < 0 || line >= num_lines)
		return;
	lines[line] = txt;
}

/*
 *	Paint sign.
 */

void Sign_gump::paint
	(
	Game_window *gwin
	)
{
	int font = 1;			// Normal runes.
	if (get_shapenum() == 0x33)
		font = 6;		// Embossed.
					// Get height of 1 line.
	int lheight = gwin->get_text_height(font);
					// Get space between lines.
	int lspace = (object_area.h - num_lines*lheight)/(num_lines + 1);
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
	int ypos = y + object_area.y;	// Where to paint next line.
	for (int i = 0; i < num_lines; i++)
	{
		ypos += lspace;
		if (lines[i].empty())
			continue;
		gwin->paint_text(font, lines[i].c_str(),
			x + object_area.x + 
				(object_area.w - 
				    gwin->get_text_width(font, lines[i].c_str()))/2,
			ypos);
		ypos += lheight;
	}
	gwin->set_painted();
}
