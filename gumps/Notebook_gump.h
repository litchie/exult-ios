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
	char *text;			// Text, len.
	int textlen;
public:
	friend class Notebook_gump;
	One_note(int d, int h, int la, int ln, char *txt = 0, int len = 0);
	~One_note()
		{ delete [] text; }
};

/*
 *	A notebook gump represents the in-game journal.
 */
class Notebook_gump : public Gump
{
	UNREPLICATABLE_CLASS(Notebook_gump);
	static vector<One_note> notes;	// The text.
	static bool initialized;
	int curnote;			// Current note at top of left page.
	int curoff;			// Offset within curnote at top.
public:
	Notebook_gump();
	~Notebook_gump() {  }
	static Notebook_gump *create();
	virtual void paint();		// Paint it and its contents.
};

#endif
