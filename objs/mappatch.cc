/**
 **	Mappatch.cc - Patches to the game map.
 **
 **	Written: 10-18-2001
 **/

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

#include "mappatch.h"
#include "gamewin.h"
#include "objs.h"

/*
 *	Find (first) matching object.
 */

Game_object *Map_patch::find
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Object_vector vec;		// Pass mask=0xb0 to get any object.
	Game_object::find_nearby(vec, spec.loc, spec.shapenum, 0, 0xb0,
					spec.quality, spec.framenum);
	return vec.empty() ? 0 : vec.front();
	}

/*
 *	Apply by removing object.
 *
 *	Output:	false if no object found.
 */

bool Map_patch_remove::apply
	(
	)
	{
	Game_object *obj = find();
	if (!obj)
		return false;
	obj->remove_this();
	return true;
	}

/*
 *	Apply by moving/changing object.
 *
 *	Output:	false if no object found.
 */

bool Map_patch_modify::apply
	(
	)
	{
	Game_object *obj = find();
	if (!obj)
		return false;
	obj->remove_this(1);		// Remove but don't delete.
	if (mod.shapenum != c_any_shapenum)
		obj->set_shape(mod.shapenum);
	if (mod.framenum != c_any_framenum)
		obj->set_frame(mod.framenum);
	if (mod.quality != c_any_qual)
		obj->set_quality(mod.quality);
	obj->set_invalid();		// To add it back correctly.
	obj->move(mod.loc);
	return true;
	}

/*
 *	Delete all the patches.
 */

Map_patch_collection::~Map_patch_collection
	(
	)
	{
	for (map<int, list<Map_patch *>>::iterator it1 = patches.begin();
						it1 != patches.end(); it1++)
		{
		list<Map_patch *>& lst = (*it1).second;
		while (!lst.empty())
			delete lst.pop_front();
		}
	}

/*
 *	Add a new patch.
 */

void Map_patch_collection::add
	(
	Map_patch *p
	)
	{
					// Get superchunk coords.
	int sx = p->spec.loc.tx/c_tiles_per_schunk,
	    sy = p->spec.loc.ty/c_tiles_per_schunk;
					// Get superchunk # (0-143).
	int schunk = sy*c_num_schunks + sx;
	map<int, list<Map_patch *>>::iterator it1 = patches.find(schunk);
	if (it1 != patches.end())	// Found list for superchunk?
		{
		list<Map_patch *>& lst = (*it1).second;
		for (list<Map_patch *>::const_iterator it2 = lst.begin();
					it2 != lst.end(); it2++)
		(*it2)->apply();	// Apply each one in list.
		}
	}
