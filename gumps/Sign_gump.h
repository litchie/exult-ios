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

#ifndef _SIGN_GUMP_H_
#define _SIGN_GUMP_H_

#include <string>
#include "Gump.h"

/*
 *	A sign showing runes.
 */
class Sign_gump : public Gump
{
	UNREPLICATABLE_CLASS(Sign_gump);

protected:
	std::string *lines;			// Lines of text.
	int num_lines;
	bool serpentine;

public:
	Sign_gump(int shapenum, int nlines);
	~Sign_gump();
					// Set a line of text.
	void add_text(int line, const std::string &txt);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
};

#endif
