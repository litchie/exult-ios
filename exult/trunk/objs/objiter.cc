/**
 **	Objiter.h - Game objects iterator.
 **
 **	Written: 5/27/2002 - JSF
 **/

/*
Copyright (C) 2002  The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "objiter.h"
#include "contain.h"
#include "gamewin.h"
#include "chunks.h"

/*
#ifdef _MSC_VER
#pragma optimize("t", off)
#endif
*/

/*
 *	Create to start after/before a given object within a chunk.
 */

template<class D> 
D_Recursive_object_iterator<D>::D_Recursive_object_iterator<D>
	(
	Game_object *start		// Start here.
	) : elems(Game_window::get_instance()->get_chunk(
		start->get_outermost())->get_objects()), child(0)
	{
					// Get what obj. is in (or itself).
	Game_object *owner = start->get_outermost();
	Game_object *obj;		// Find owner within its chunk.
	while ((obj = get_next()) != 0 && obj != owner)
		;
	if (!obj)
		return;			// Bad.  It wasn't found.
	if (obj != start)		// Given object contained?
					// Look within for it.
		while ((obj = get_next()) != 0 && obj != start)
			;
	}

/*
 *	Get next game object, going down recursively into containers.
 *
 *	Output:	Next in world, or 0 if done.
 */

template<class D> Game_object *D_Recursive_object_iterator<D>::get_next
	(
	)
	{
	Game_object *obj;
	if (child)			// Going through container?
		{
		obj = child->get_next();
		if (obj)
			return obj;
		delete child;
		child = 0;		// Child done.
		}
	obj = elems.get_next();		// Get next from our list.
	if (!obj)
		return 0;		// All done.
					// Is it a container?
	Container_game_object *c = 
			dynamic_cast<Container_game_object *> (obj);
	if (c)				// Container?  Set to go through it.
		child = new D_Recursive_object_iterator<D>(
							c->get_objects());
	return obj;
	}
