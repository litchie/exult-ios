/**
 **	Gumps.cc - Open containers and their contents.
 **
 **	Written: 3/4/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "gumps.h"
#include "gamewin.h"

/*
 *	Create a gump.
 */

Gump_object::Gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : container(cont), x(initx), y(inity), ShapeID(shnum, 0)
	{
cout << "Creating gump at " << initx << ", " << inity << '\n';
					// +++++Depends on shnum:
	switch (shnum)			// Different shapes.
		{
	case 1:				// Crate.
		object_area = Rectangle(58, 28, 80, 24);
		break;
	case 8:				// Barrel.
		object_area = Rectangle(40, 40, 40, 40);
		break;
	case 9:				// Bag.
		object_area = Rectangle(56, 28, 66, 44);
		break;
	case 10:			// Backpack.
		object_area = Rectangle(54, 44, 72, 40);
		break;
	case 22:			// Chest.
		object_area = Rectangle(48, 28, 60, 32);
		break;
	case 27:			// Drawer.
		object_area = Rectangle(46, 20, 70, 26);
		break;
	default:
		object_area = Rectangle(60, 30, 60, 40);
		}
	}

/*
 *	Add this gump to the end of a chain.
 */

void Gump_object::append_to_chain
	(
	Gump_object *& chain		// Head.
	)
	{
	next = 0;			// Put at end of chain.
	if (!chain)
		{
		chain = this;		// First one.
		return;
		}
	Gump_object *last;
	for (last = chain; last->next; last = last->next)
		;
	if (!last)			// First one?
		chain = this;
	else
		last->next = this;
	}

/*
 *	Remove from a chain.
 */

void Gump_object::remove_from_chain
	(
	Gump_object *& chain		// Head.
	)
	{
	if (chain == this)
		chain = next;
	else
		{
		Gump_object *p;		// Find prev. to this.
		for (p = chain; p->next != this; p = p->next)
			;
		p->next = next;
		}
	}

/*
 *	Paint on screen.
 */

void Gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
	Game_object *last_object = container->get_last_object();
	if (!last_object)
		return;			// Empty.
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.
	int cury = box.y, curx = box.x;
	int endy = box.y + box.h, endx = box.x + box.w;
	int loop = 0;			// # of times covering container.
	Game_object *obj = last_object;
	do				// First try is really rough.+++++
		{
		obj = obj->get_next();
		Shape_frame *shape = gwin->get_shape(*obj);
		int px = curx + shape->get_width(),
		    py = cury + shape->get_height();
		if (px > endx)
			px = endx;
		if (py > endy)
			py = endy;
		gwin->paint_shape(px, py, obj->get_shapenum(),
						obj->get_framenum());
		curx += 8;
		if (curx >= endx)
			{
			cury += 8;
			curx = box.x;
			if (cury >= endy)
				cury = box.y + 2*(++loop);
			}
		}
	while (obj != last_object);
	}
