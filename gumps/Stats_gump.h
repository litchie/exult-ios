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

#ifndef _STATS_GUMP_H_
#define _STATS_GUMP_H_

#include "Gump.h"
#include "misc_buttons.h"

class Actor;

/*
 *	A rectangular area showing a character's statistics:
 */
class Stats_gump : public Gump
{
	UNREPLICATABLE_CLASS(Stats_gump);

protected:
	Actor *get_actor()
		{ return (Actor *) container; }
	static short textx;		// X-coord. of where to write.
	static short texty[10];		// Y-coords.

public:
	Stats_gump(Container_game_object *cont, int initx, int inity);
	~Stats_gump()
		{  }
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
			int sx = -1, int sy = -1, bool dont_check = false,
						bool combine = false)
		{ return 0; }		// Can't drop onto it.
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);

	virtual Game_object *find_object(int mx, int my)
		{ return 0; }
};

#endif
