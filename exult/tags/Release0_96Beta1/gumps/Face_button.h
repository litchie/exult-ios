/*
Copyright (C) 2001 The Exult Team

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

#ifndef _FACE_BUTTON_H_
#define _FACE_BUTTON_H_

#include "Gump_button.h"

class Actor;
class Game_window;

/*
 *	A character's face, that will open inventory when clicked.
 */
class Face_button : public Gump_button
{
protected:
	Actor *actor;			// Who this represents.
public:
	Face_button(Gump *par, int px, int py, Actor *a);
	virtual Actor *get_actor() { return actor; }
	virtual void double_clicked(Game_window *gwin, int x, int y);
	virtual void activate(Game_window *gwin) {}

	virtual void push(Game_window *gwin) {}
	virtual void unpush(Game_window *gwin) {}
	virtual void update_widget(Game_window *gwin);
};

#endif
