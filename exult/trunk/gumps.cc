/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
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

const int CHECKMARK = 2;		// Shape # in gumps.vga for checkmark.
const int DISK = 2;			// Diskette shape #.+++++++++
const int STATS = 2;			// Stats button shape #.+++++++++

/*
 *	Statics:
 */
short Actor_gump_object::diskx = 20, Actor_gump_object::disky = 20;//++++++++
short Actor_gump_object::statx = 30, Actor_gump_object::staty = 30;


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
	checkx = 8; checky = 64;	//++++default.

	switch (shnum)			// Different shapes.
		{
	case 1:				// Crate.
		object_area = Rectangle(50, 20, 80, 24);
		checkx = 8; checky = 64;
		break;
	case 8:				// Barrel.
		object_area = Rectangle(32, 32, 40, 40);
		checkx = 12; checky = 124;
		break;
	case 9:				// Bag.
		object_area = Rectangle(48, 20, 66, 44);
		checkx = 8; checky = 66;
		break;
	case 10:			// Backpack.
		object_area = Rectangle(36, 36, 72, 40);
		checkx = 8; checky = 62;
		break;
	case 11:			// Basket.
		object_area = Rectangle(42, 32, 70, 26);
		checkx = 8; checky = 56;
		break;
	case 22:			// Chest.
		object_area = Rectangle(40, 20, 60, 32);
		checkx = 8; checky = 46;
		break;
	case 27:			// Drawer.
		object_area = Rectangle(38, 12, 70, 26);
		checkx = 8; checky = 46;
		break;
	default:
					// Character pictures:
		if (shnum >= 57 && shnum <= 68)
			{		// +++++Want whole rectangle.
			object_area = Rectangle(32, 4, 92, 126);
			checkx = 6; checky = 136;
			}
		else
			object_area = Rectangle(52, 22, 60, 40);
		}
	checkx += 16; checky -= 12;
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
 *	Get screen rectangle for one of our objects.
 */

Rectangle Gump_object::get_shape_rect
	(
	Game_object *obj
	)
	{
	Shape_frame *s = Game_window::get_game_window()->get_shape(*obj);
	return Rectangle(x + object_area.x + obj->cx - s->get_xleft(), 
			 y + object_area.y + obj->cy - s->get_yabove(), 
				 s->get_width(), s->get_height());
	}

/*
 *	Find objects a screen point is on.
 *
 *	Output:	# of objects stored, last one being the highest.
 */

int Gump_object::find_objects
	(
	Game_window *gwin,
	int mx, int my,			// Mouse pos. on screen.
	Game_object **list		// Objects found are stored here.
	)
	{
	int cnt = 0;
	Game_object *last_object = container->get_last_object();
	if (!last_object)
		return (0);
	Game_object *obj = last_object;
	do
		{
		obj = obj->get_next();
		Rectangle box = get_shape_rect(obj);
		if (box.has_point(mx, my))
			list[cnt++] = obj;
		}
	while (obj != last_object);
	return (cnt);
	}

/*
 *	Is a given screen point on the checkmark?
 */

int Gump_object::on_checkmark
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	mx -= x + checkx;		// Get point rel. to gump.
	my -= y + checky;
	Shape_frame *cshape = gwin->get_gump_shape(CHECKMARK, 0);
	return (cshape->has_point(mx, my));
	}

/*
 *	Repaint checkmark pushed.
 */

void Gump_object::push_checkmark
	(
	Game_window *gwin
	)
	{
	gwin->paint_gump(x + checkx, y + checky, CHECKMARK, 1);
	}

/*
 *	Add an object.
 *
 *	Output:	0 if cannot add it.
 */

int Gump_object::add
	(
	Game_object *obj,
	int mx, int my			// Screen location.
	)
	{
	container->add(obj);
	mx -= x + object_area.x;	// Get point rel. to object_area.
	my -= y + object_area.y;
	if (mx < 0)			// Not a valid spot?
					// Let paint() set spot.
		obj->cx = obj->cy = 255;	
	else
		{			// Put it where desired.
		obj->cx = mx;
		obj->cy = my;
		}
	return (1);
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
					// Paint red "checkmark".
	gwin->paint_gump(x + checkx, y + checky, CHECKMARK, 0);
	Game_object *last_object = container->get_last_object();
	if (!last_object)
		return;			// Empty.
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.
	int cury = 0, curx = 0;
	int endy = box.h, endx = box.w;
	int loop = 0;			// # of times covering container.
	Game_object *obj = last_object;
	do				// First try is really rough.+++++
		{
		obj = obj->get_next();
		Shape_frame *shape = gwin->get_shape(*obj);
		int objx = obj->cx - shape->get_xleft();
		int objy = obj->cy - shape->get_yabove();
					// Does obj. appear to be placed?
		if (!object_area.has_point(objx, objy) ||
		    !object_area.has_point(objx + shape->get_width() - 2,
					objy + shape->get_height() - 2))
			{		// No.
			int px = curx + shape->get_width(),
			    py = cury + shape->get_height();
			if (px > endx)
				px = endx;
			if (py > endy)
				py = endy;
			obj->cx = px - shape->get_width() + 
					shape->get_xleft();
			obj->cy = py - shape->get_height() + 
					shape->get_yabove();
			curx += 8;
			if (curx >= endx)
				{
				cury += 8;
				curx = 0;
				if (cury >= endy)
					cury = 2*(++loop);
				}
			}
		gwin->paint_shape(box.x + obj->cx,
				  box.y + obj->cy, 
						obj->get_shapenum(),
						obj->get_framenum());
		}
	while (obj != last_object);
	}

/*
 *	Find the index of the closest 'spot' to a mouse point.
 *
 *	Output:	Index.
 */

int Actor_gump_object::find_closest
	(
	int mx, int my			// Mouse point in window.
	)
	{
	mx -= x; my -= y;		// Get point rel. to us.
	long closest_squared = 1000000;	// Best distance squared.
	int closest = -1;		// Best index.
	for (int i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		{
		int dx = mx - spots[i].x, dy = my - spots[i].y;
		long dsquared = dx*dx + dy*dy;
					// Better than prev.?
		if (dsquared < closest_squared)
			{
			closest_squared = dsquared;
			closest = i;
			}
		}
	return (closest);
	}

/*
 *	Create the gump display for an actor.
 */

Actor_gump_object::Actor_gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump_object(cont, initx, inity, shnum)
	{
	// +++++++++Init spot locations, ids.
	}

/*
 *	Add an object.
 *
 *	Output:	0 if cannot add it.
 */

int Actor_gump_object::add
	(
	Game_object *obj,
	int mx, int my			// Screen location.
	)
	{
					// Find index of closest spot.
	int index = find_closest(mx, my);
	Actor_gump_spot& spot = spots[index];
	if (spot.obj)			// Already something there?
					// Try to put into container.
		return (spot.obj->drop(obj));
	spot.obj = obj;			// Put in spot. ++++Check types??
	container->add(obj);		// Add to whom we represent.
					// Set object's position.
	obj->cx = spot.x - object_area.x;
	obj->cy = spot.y - object_area.y;
	return (1);
	}

/*
 *	Paint on screen.
 */

void Actor_gump_object::paint
	(
	Game_window *gwin
	)
	{
	Gump_object::paint(gwin);	// Paint objects, checkmark.
					// Paint buttons.
	gwin->paint_gump(x + checkx, y + checky, CHECKMARK, 0);
	gwin->paint_gump(x + diskx, y + disky, DISK, 0);
	gwin->paint_gump(x + statx, y + staty, STATS, 0);
	}
