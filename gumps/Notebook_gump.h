/*
Copyright (C) 2000-2004 The Exult Team

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

#ifndef _NOTEBOOK_GUMP_H_
#define _NOTEBOOK_GUMP_H_

#include "Gump.h"
#include "font.h"
#include <vector>

using std::vector;

class One_note;

/*
 *	Info. for top of a page.
 */
class Notebook_top
{
	int notenum;
	int offset;
public:
	friend class Notebook_gump;
	Notebook_top(int n = 0, int o = 0) : notenum(n), offset(o)
		{  }
};

/*
 *	A notebook gump represents the in-game journal.
 */
class Notebook_gump : public Gump
{
	UNREPLICATABLE_CLASS(Notebook_gump);
	static vector<One_note *> notes;// The text.
					// Indexed by page#.
	static vector<Notebook_top> page_info;
	static Notebook_gump *instance;
	static bool initialized;
	int curpage;			// Current page # (from 0).
	Cursor_info cursor;		// Cursor loc. within current page.
					// Page turners:
	Gump_button *leftpage, *rightpage;

	static void initialize();
	void add_new();			// Add new note.
	int paint_page(Rectangle box, One_note *note, int start, int pagenum);
public:
	Notebook_gump();
	~Notebook_gump();
	static void clear();
	static Notebook_gump *create();
	static Notebook_gump *get_instance() { return instance; }
	void change_page(int delta);	// Page forward/backward.
					// Is a given point on a button?
	virtual Gump_button *on_button(int mx, int my);
	virtual void paint();		// Paint it and its contents.
	virtual bool handle_kbd_event(void *ev);
	static void write();		// Write it out to gamedat.
	static void read();		// Read it in.
};

#endif
