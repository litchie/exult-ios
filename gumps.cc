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

/*
 *	Statics:
 */
short Actor_gump_object::diskx = 122, Actor_gump_object::disky = 132;
short Actor_gump_object::heartx = 122, Actor_gump_object::hearty = 114;
short Stats_gump_object::textx = 123;
short Stats_gump_object::texty[10] = {17, 26, 35, 46, 55, 67, 76, 86,
							95, 104};

/*
 *	Is a given screen point on this button?
 */

int Gump_button::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	mx -= parent->get_x() + x;	// Get point rel. to gump.
	my -= parent->get_y() + y;
	Shape_frame *cshape = gwin->get_gump_shape(shapenum, 0);
	return (cshape->has_point(mx, my));
	}

/*
 *	Redisplay as 'pushed'.
 */

void Gump_button::push
	(
	Game_window *gwin
	)
	{
	pushed = 1;
	parent->paint_button(gwin, this);
	}

/*
 *	Redisplay as 'unpushed'.
 */

void Gump_button::unpush
	(
	Game_window *gwin
	)
	{
	pushed = 0;
	parent->paint_button(gwin, this);
	}

/*
 *	Handle click on a 'checkmark'.
 */

void Checkmark_gump_button::activate
	(
	Game_window *gwin
	)
	{
	gwin->remove_gump(parent);	// (This kills ourself.)
	}

/*
 *	Handle click on a heart.
 */

void Heart_gump_button::activate
	(
	Game_window *gwin
	)
	{
	gwin->show_gump(parent->get_container(), STATSDISPLAY);
	}

/*
 *	Handle click on a diskette.
 */

void Disk_gump_button::activate
	(
	Game_window *gwin
	)
	{
	cout << "Diskette clicked.\n";
	}

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
	int checkx = 8, checky = 64;	// Default.

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
	case STATSDISPLAY:
		object_area = Rectangle(0, 0, 0, 0);
		checkx = 6; checky = 136;
		break;
	default:
					// Character pictures:
		if (shnum >= 57 && shnum <= 68)
			{		// Want whole rectangle.
			object_area = Rectangle(26, 0, 104, 132);
			checkx = 6; checky = 136;
			}
		else
			object_area = Rectangle(52, 22, 60, 40);
		}
	checkx += 16; checky -= 12;
	check_button = new Checkmark_gump_button(this, checkx, checky);
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
 *	Find object a screen point is on.
 *
 *	Output:	Object found, or null.
 */

Game_object *Gump_object::find_object
	(
	Game_window *gwin,
	int mx, int my			// Mouse pos. on screen.
	)
	{
	int cnt = 0;
	Game_object *list[100];
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
					// ++++++Return top item.
	return (cnt ? list[cnt - 1] : 0);
	}

/*
 *	Is a given screen point on the checkmark?
 *
 *	Output: ->button if so.
 */

Gump_button *Gump_object::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	return (check_button->on_button(gwin, mx, my) ?
			check_button : 0);
	}

/*
 *	Repaint checkmark.
 */

void Gump_object::paint_button
	(
	Game_window *gwin,
	Gump_button *btn
	)
	{
	gwin->paint_gump(x + btn->x, y + btn->y, btn->shapenum, btn->pushed);
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
	paint_button(gwin, check_button);
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
		int objx = obj->cx - shape->get_xleft() + object_area.x;
		int objy = obj->cy - shape->get_yabove() + object_area.y;
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
 *	Output:	Index, or -1 if unsuccessful.
 */

int Actor_gump_object::find_closest
	(
	int mx, int my,			// Mouse point in window.
	int only_empty			// Only allow empty spots.
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
		if (dsquared < closest_squared && (!only_empty ||
						    !spots[i].obj))
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
	heart_button = new Heart_gump_button(this, heartx, hearty);
	disk_button = new Disk_gump_button(this, diskx, disky);
					// Set spot locations.
	spots[(int) head].x = 114; spots[(int) head].y = 10;
	spots[(int) back].x = 115; spots[(int) back].y = 24;
	spots[(int) lhand].x = 115; spots[(int) lhand].y = 55;
	spots[(int) rhand].x = 37; spots[(int) rhand].y = 56;
	spots[(int) legs].x = 114; spots[(int) legs].y = 85;
	spots[(int) feet].x = 76; spots[(int) feet].y = 98;
	spots[(int) lfinger].x = 116; spots[(int) lfinger].y = 70;
	spots[(int) rfinger].x = 35; spots[(int) rfinger].y = 70;
					// Store objs. in their spots.
	Game_object *last_object = container->get_last_object();
	if (!last_object)
		return;			// Empty.
	Game_object *obj = last_object;
	do
		{
		obj = obj->get_next();
		int ox, oy;		// Get screen location.
		get_shape_location(obj, ox, oy);
					// Find an empty spot.
		int index = find_closest(ox, oy, 1);
		if (index >= 0)		// And if it fails???
			add_to_spot(obj, index);
		}
	while (obj != last_object);
	}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Actor_gump_object::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	Gump_button *btn = Gump_object::on_button(gwin, mx, my);
	if (btn)
		return btn;
	else if (heart_button->on_button(gwin, mx, my))
		return heart_button;
	else if (disk_button->on_button(gwin, mx, my))
		return disk_button;
	return 0;
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
	Actor_gump_spot *spot = &spots[index];
	if (spot->obj)			// Already something there?
		{			// Try to put into container.
		if (spot->obj->drop(obj))
			return (1);
					// Try again for an empty spot.
		index = find_closest(mx, my, 1);
		if (index < 0)
			return (0);
		}
	container->add(obj);		// Add to whom we represent.
	add_to_spot(obj, index);
	return (1);
	}

/*
 *	Add object to an empty spot.
 */

void Actor_gump_object::add_to_spot
	(
	Game_object *obj,
	int index			// Spot index.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor_gump_spot *spot = &spots[index];
	spot->obj = obj;		// Put in spot. ++++Check types??
					// Get shape info.
	Shape_frame *shape = gwin->get_shape(*obj);
	int w = shape->get_width(), h = shape->get_height();
					// Set object's position.
	obj->cx = spot->x + shape->get_xleft() - w/2 - object_area.x;
	obj->cy = spot->y + shape->get_yabove() - h/2 - object_area.y;
					// Shift if necessary.
	int x0 = obj->cx - shape->get_xleft(), 
	    y0 = obj->cy - shape->get_yabove();
	if (x0 < 0)
		obj->cx -= x0;
	if (y0 < 0)
		obj->cy -= y0;
	int x1 = x0 + w, y1 = y0 + h;
	if (x1 > object_area.w)
		obj->cx -= x1 - object_area.w;
	if (y1 > object_area.h)
		obj->cy -= y1 - object_area.h;
	}

/*
 *	Remove an object.
 */

void Actor_gump_object::remove
	(
	Game_object *obj
	)
	{
	Gump_object::remove(obj);
					// Find its spot.
	for (int i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		if (spots[i].obj == obj)
			{
			spots[i].obj = 0;
			break;
			}
	}

/*
 *	Paint on screen.
 */

void Actor_gump_object::paint
	(
	Game_window *gwin
	)
	{
	Gump_object::paint(gwin);	// Paint gump & objects.
					// Paint buttons.
	paint_button(gwin, heart_button);
	paint_button(gwin, disk_button);
	}

/*
 *	Show a number.
 */

static void Paint_num
	(
	Game_window *gwin,
	int num,
	int x,				// Coord. of right edge of #.
	int y				// Coord. of top of #.
	)
	{
	const int font = 2;
	char buf[20];
  	sprintf(buf, "%d", num);
	gwin->paint_text(font, buf, x - gwin->get_text_width(font, buf), y);
	}

/*
 *	Paint on screen.
 */

void Stats_gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
	Actor *act = get_actor();	// Show statistics.
	Paint_num(gwin, act->get_property(Actor::strength),
						x + textx, y + texty[0]);
	Paint_num(gwin, act->get_property(Actor::dexterity),
						x + textx, y + texty[1]);
	Paint_num(gwin, act->get_property(Actor::intelligence),
						x + textx, y + texty[2]);
  	Paint_num(gwin, act->get_property(Actor::combat),
						x + textx, y + texty[3]);
  	Paint_num(gwin, act->get_property(Actor::magic),
						x + textx, y + texty[4]);
  	Paint_num(gwin, act->get_property(Actor::health),
						x + textx, y + texty[5]);
  	Paint_num(gwin, act->get_property(Actor::mana),
						x + textx, y + texty[6]);
  	Paint_num(gwin, act->get_property(Actor::exp),
						x + textx, y + texty[7]);
	//++++Level?
  	Paint_num(gwin, act->get_property(Actor::training),
						x + textx, y + texty[9]);
	}
