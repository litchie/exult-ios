/*
 *  Copyright (C) 2000-2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _FILE_GUMP_H_
#define _FILE_GUMP_H_

#include "Modal_gump.h"

class Gump_text;

/*
 *	The file save/load box:
 */
class File_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(File_gump,Modal_gump(0,0,0,0));

protected:
	static short textx, texty;	// Where to draw first text field.
	static short texth;		// Distance down to next text field.
	static short btn_rows[2];	// y-coord of each button row.
	static short btn_cols[3];	// x-coord of each button column.
	Gump_text *names[10];		// 10 filename slots.
	Gump_button *buttons[6];	// 2 rows, 3 cols of buttons.
	Gump_text *pushed_text;		// Text mouse is down on.
	Gump_text *focus;		// Text line that has focus.
	unsigned char restored;		// Set to 1 if we restored a game.

public:
	File_gump();
	~File_gump();
					// Find savegame index of text field.
	int get_save_index(Gump_text *txt);
	void remove_focus();		// Unfocus text.
	void load();			// 'Load' was clicked.
	void save();			// 'Save' was clicked.
	void quit();			// 'Quit' was clicked.
					// Handle one of the toggles.
	int toggle_option(Gump_button *btn);
	int restored_game()		// 1 if user restored.
		{ return restored; }
					// Paint it and its contents.
	virtual void paint();
	virtual void close()
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void text_input(int chr, int unicode); // Character typed.
};

#endif
