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

#ifndef _GUMP_BUTTON_H_
#define _GUMP_BUTTON_H_

#include "Gump_widget.h"

/*
 *	A pushable button on a gump:
 */
class Gump_button : public Gump_widget
{
	UNREPLICATABLE_CLASS(Gump_button);

protected:
	unsigned char pushed;		// 1 if in pushed state.

public:
	friend class Gump;
	Gump_button(Gump *par, int shnum, int px, int py, 
				ShapeFile shfile = SF_GUMPS_VGA)
		: Gump_widget(par, shnum, px, py, shfile), pushed(0)
		{  }
					// Is a given point on the checkmark?
	virtual int on_button(Game_window *gwin, int mx, int my)
		{ return on_widget(gwin, mx, my); }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin) = 0;
					// Or double-clicked.
	virtual void double_clicked(Game_window *gwin, int x, int y);
	virtual void push(Game_window *gwin);	// Redisplay as pushed.
	virtual void unpush(Game_window *gwin);
	virtual void paint(Game_window *gwin);
};

#endif
