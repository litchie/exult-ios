/*
 *	delobjs.cc - Game objects that have been removed, but need deleting.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <vector>
#include "objs.h"
#include <SDL_timer.h>
#include "delobjs.h"

using std::vector;

struct Obj_with_time
	{
	Game_object *obj;
	unsigned int ticks;
	Obj_with_time(Game_object *o, unsigned int t) : obj(o), ticks(t)
		{  }
	};

/*
 *	Remove and delete all objects.
 */
void Deleted_objects::flush
	(
	)
	{
	typedef vector<Obj_with_time> Obj_time_list;

	if (empty())
		return;
	Obj_time_list keep;
	keep.reserve(100);
					// Wait at least 3 minutes.
	unsigned int curtime = SDL_GetTicks();
	for(std::map<Game_object *,unsigned int,Less_objs>::iterator X = 
					begin(); X != end(); ++X)
		{
		Game_object *obj = (*X).first;
		int ticks = (*X).second;
		if (ticks < curtime)
			delete obj;
		else
			keep.push_back(Obj_with_time(obj, ticks));
		}
	clear();			// Clear map.
	for (Obj_time_list::iterator it = keep.begin(); it != keep.end(); ++it)
		(*this)[(*it).obj] = (*it).ticks;
	}

