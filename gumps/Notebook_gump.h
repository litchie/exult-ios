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
#include <vector>

using std::vector;

class One_note
{
	int day, hour;			// Game time when note was written.
	int lat, lng;			// Latitute, longitude where written.
	char *text;			// Text, 0-delimited.
	int textlen;			// Length, not counting ending NULL.
public:
	friend class Notebook_gump;
	One_note() : day(0), hour(0), lat(0), lng(0), text(0), textlen(0)
		{  }
	void set(int d, int h, int la, int ln, char *txt = 0);
	One_note(int d, int h, int la, int ln, char *txt = 0) : text(0)
		{ set(d, h, la, ln, txt); }
	~One_note()
		{ delete [] text; }
};

/*
 *	Info. for top left page.
 */
class Notebook_top_left
{
	int notenum;
	int offset;
public:
	friend class Notebook_gump;
	Notebook_top_left(int n = 0, int o = 0) : notenum(n), offset(o)
		{  }
};

/*
 *	A notebook gump represents the in-game journal.
 */
class Notebook_gump : public Gump
{
	UNREPLICATABLE_CLASS(Notebook_gump);
	static vector<One_note *> notes;// The text.
					// Indexed by page#/2:
	static vector<Notebook_top_left> page_info;
	static bool initialized;
	int curpage;			// Page # of current top-left.
					// Page turners:
	Gump_button *leftpage, *rightpage;

	int paint_page(Rectangle box, One_note *note, int start);
public:
	Notebook_gump();
	~Notebook_gump();
	static Notebook_gump *create();
	void change_page(int delta);	// Page forward/backward.
					// Is a given point on a button?
	virtual Gump_button *on_button(int mx, int my);
	virtual void paint();		// Paint it and its contents.
};

#endif
