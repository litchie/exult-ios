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

#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "Gump.h"
#include "misc_buttons.h"
#include "contain.h"
#include "objiter.h"

/*
 *	Initialize.
 */

void Gump::initialize
	(
	)
{

	if (is_paperdoll())
	{
		initialize2();
		return;
	}
		
	int checkx = 8, checky = 64;	// Default.
	int shnum = get_shapenum();
	if(shnum==game->get_shape("gumps/yesnobox"))
	{
		object_area = Rectangle(6, 6, 116, 28);
	}
	else if(shnum==game->get_shape("gumps/fileio"))
	{
		checkx = 8;
		checky = 150;
	}
	else if(shnum==game->get_shape("gumps/statsdisplay"))
	{
		object_area = Rectangle(0, 0, 0, 0);
		checkx = 6;
		checky = 136;
	}
	else if(shnum==game->get_shape("gumps/spellbook"))
	{
		object_area = Rectangle(36, 28, 102, 66);
		checkx = 7;
		checky = 54;
	}
	else if(shnum==game->get_shape("gumps/box"))
	{
		object_area = Rectangle(46, 28, 74, 32);
		checkx = 8;
		checky = 56;
	}
	else if(shnum==game->get_shape("gumps/crate"))
	{
		object_area = Rectangle(50, 20, 80, 24);
		checkx = 8;
		checky = 64;
	}
	else if(shnum==game->get_shape("gumps/barrel"))
	{
		object_area = Rectangle(32, 32, 40, 40);
		checkx = 12;
		checky = 124;
	}
	else if(shnum==game->get_shape("gumps/bag"))
	{
		object_area = Rectangle(48, 20, 66, 44);
		checkx = 8;
		checky = 66;
	}
	else if(shnum==game->get_shape("gumps/backpack"))
	{
		object_area = Rectangle(36, 36, 85, 40);
		checkx = 8;
		checky = 62;
	}
	else if(shnum==game->get_shape("gumps/basket"))
	{
		object_area = Rectangle(42, 32, 70, 26);
		checkx = 8;
		checky = 56;
	}
	else if(shnum==game->get_shape("gumps/chest"))
	{
		object_area = Rectangle(40, 18, 60, 37);
		checkx = 8;
		checky = 46;
	}
	else if(shnum==game->get_shape("gumps/shipshold"))
	{
		object_area = Rectangle(38, 10, 82, 80);
		checkx = 8;
		checky = 92;
	}
	else if(shnum==game->get_shape("gumps/drawer"))
	{
		object_area = Rectangle(36, 12, 70, 26);
		checkx = 8;
		checky = 46;
	}
	else if(shnum==game->get_shape("gumps/slider"))
	{
		object_area = Rectangle(0, 0, 0, 0);
		checkx = 6;
		checky = 30;
	}
	else if(shnum==game->get_shape("gumps/woodsign"))
	{
		object_area = Rectangle(0, 4, 196, 92);
	}
	else if(shnum==game->get_shape("gumps/tombstone"))
	{
		object_area = Rectangle(0, 8, 200, 112);
	}
	else if(shnum==game->get_shape("gumps/goldsign"))
	{
		object_area = Rectangle(0, 4, 232, 96);
	}
	else if(shnum==game->get_shape("gumps/body"))
	{
		object_area = Rectangle(36, 46, 84, 40);
		checkx = 8;
		checky = 70;
	}
	else if ((shnum >= 57 && shnum <= 68 && Game::get_game_type() == BLACK_GATE))
	{		// Character pictures:
		// Want whole rectangle.
		object_area = Rectangle(26, 0, 104, 132);
		checkx = 6;
		checky = 136;
	}
	else
		object_area = Rectangle(52, 22, 60, 40);

	checkx += 16; checky -= 12;
	check_button = new Checkmark_button(this, checkx, checky);
}

void Gump::initialize2
	(
	)
{
	int checkx = 8, checky = 64;	// Default.
	int shnum = get_shapenum();
	if (shnum == 123)
	{		// Character pictures:
				// Want whole rectangle.
		object_area = Rectangle(26, 0, 104, 140);
		checkx = 6;
		checky = 145;
	}
	else
		object_area = Rectangle(52, 22, 60, 40);

	checkx += 16; checky -= 12;
	check_button = new Checkmark_button(this, checkx, checky);
}

/*
 *	Create a gump.
 */

Gump::Gump
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum,			// Shape #.
	bool pdoll			// Use the shapes from paperdoll.flx
	) : ShapeID(shnum, 0), container(cont), x(initx), y(inity),
	paperdoll_shape (pdoll)
{
	initialize();
}

/*
 *	Create, centered on screen.
 */

Gump::Gump
	(
	Container_game_object *cont,	// Container it represents.
	int shnum			// Shape #.
	) : ShapeID(shnum, 0), container(cont), paperdoll_shape (false)
{
	Game_window *gwin = Game_window::get_game_window();
	Shape_frame *shape = gwin->get_gump_shape(shnum, 0);
	x = (gwin->get_width() - shape->get_width())/2;
	y = (gwin->get_height() - shape->get_height())/2;
	initialize();
}

/*
 *	Delete gump.
 */

Gump::~Gump()
{
	if( check_button )
	{
		delete check_button;
		check_button = 0;
	} 
}

/*
 *	Add this gump to the end of a chain.
 */

void Gump::append_to_chain
	(
	Gump *& chain		// Head.
	)
{
	next = 0;			// Put at end of chain.
	if (!chain)
	{
		chain = this;		// First one.
		return;
	}
	Gump *last;
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

void Gump::remove_from_chain
	(
	Gump *& chain		// Head.
	)
{
	if (chain == this)
		chain = next;
	else
	{
		Gump *p;		// Find prev. to this.
		for (p = chain; p->next != this; p = p->next)
			;
		p->next = next;
	}
}

/*
 *	Get screen rectangle for one of our objects.
 */

Rectangle Gump::get_shape_rect
	(
	Game_object *obj
	)
{
	Shape_frame *s = Game_window::get_game_window()->get_shape(*obj);
	return Rectangle(x + object_area.x + obj->get_cx() - s->get_xleft(), 
			 y + object_area.y + obj->get_cy() - s->get_yabove(), 
				 s->get_width(), s->get_height());
}

/*
 *	Get screen location of object within.
 */

void Gump::get_shape_location
	(
	Game_object *obj, 
	int& ox, int& oy
	)
{
	ox = x + object_area.x + obj->get_cx(),
	oy = y + object_area.y + obj->get_cy();
}

/*
 *	Find object a screen point is on.
 *
 *	Output:	Object found, or null.
 */

Game_object *Gump::find_object
	(
	int mx, int my			// Mouse pos. on screen.
	)
{
	int cnt = 0;
	Game_object *list[100];
	if (!container)
		return (0);
	Object_iterator next(container->get_objects());
	Game_object *obj;
	Shape_frame *s;
	Game_window *gwin = Game_window::get_game_window();

	int ox, oy;

	while ((obj = next.get_next()) != 0)
	{
		Rectangle box = get_shape_rect(obj);
		if (box.has_point(mx, my))
		{
			s = gwin->get_shape(*obj);
			get_shape_location(obj, ox, oy);
			if (s->has_point(mx-ox, my-oy))
				list[cnt++] = obj;
		}
		obj = obj->get_next();
	}
					// ++++++Return top item.
	return (cnt ? list[cnt - 1] : 0);
}

/*
 *	Get the entire screen rectangle covered by this gump and its contents.
 */

Rectangle Gump::get_dirty
	(
	)
{
	Game_window *gwin = Game_window::get_game_window();
	Rectangle rect = gwin->get_gump_rect(this);
	if (!container)
		return rect;
	Object_iterator next(container->get_objects());
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
	{
		Rectangle orect = get_shape_rect(obj);
		rect = rect.add(orect);
	}
	return rect;
}

/*
 *	Get object this belongs to.
 */

Game_object *Gump::get_owner()
{ 
	return container; 
}

/*
 *	Is a given screen point on the checkmark?
 *
 *	Output: ->button if so.
 */

Gump_button *Gump::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
{
	return (check_button->on_button(gwin, mx, my) ?
			check_button : 0);
}

/*
 *	Repaint checkmark, etc.
 */

void Gump::paint_button
	(
	Game_window *gwin,
	Gump_button *btn
	)
{
	if (btn)
		gwin->paint_gump(x + btn->x, y + btn->y, btn->shapenum, 
				btn->framenum + btn->pushed);
}

/*
 *	Add an object.  If mx, my, sx, sy are all -1, the object's position
 *	is calculated by 'paint()'.  If they're all -2, it's assumed that
 *	obj->cx, obj->cy are already correct.
 *
 *	Output:	0 if cannot add it.
 */

int Gump::add
	(
	Game_object *obj,
	int mx, int my,			// Mouse location.
	int sx, int sy			// Screen location of obj's hotspot.
	)
{
	if (!container || !container->has_room(obj))
		return (0);		// Full.
					// Dropping on same thing?
	Game_object *onobj = find_object(mx, my);
					// If possible, combine.

	if (onobj && onobj != obj && onobj->drop(obj))
		return (1);

	if (!container->add(obj))
		return (0);

					// Not a valid spot?
	if (sx == -1 && sy == -1 && mx == -1 && my == -1)
					// Let paint() set spot.
		obj->set_chunk(255, 255);
					// -2's mean cx, cy are already set.
	else if (sx != -2 && sy != -2 && mx != -2 && my != -2)
	{			// Put it where desired.
		sx -= x + object_area.x;// Get point rel. to object_area.
		sy -= y + object_area.y;
		Shape_frame *shape = Game_window::get_game_window()->get_shape(
									*obj);
					// But shift within range.
		if (sx - shape->get_xleft() < 0)
			sx = shape->get_xleft();
		else if (sx + shape->get_xright() > object_area.w)
			sx = object_area.w - shape->get_xright();
		if (sy - shape->get_yabove() < 0)
			sy = shape->get_yabove();
		else if (sy + shape->get_ybelow() > object_area.h)
			sy = object_area.h - shape->get_ybelow();
		obj->set_chunk(sx, sy);
	}
	return (1);
}

/*
 *	Remove object.
 */

void Gump::remove
	(
	Game_object *obj
	)
{
	container->remove(obj); 

	// Paint Objects
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.

	Game_window *gwin = Game_window::get_game_window();

	gwin->set_all_dirty();
	gwin->paint_dirty();
}

/*
 *	Paint on screen.
 */

void Gump::paint
	(
	Game_window *gwin
	)
{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum(), is_paperdoll());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
	if (!container)
		return;			// Empty.
	Object_list& objects = container->get_objects();
	if (objects.is_empty())
		return;			// Empty.
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.
	int cury = 0, curx = 0;
	int endy = box.h, endx = box.w;
	int loop = 0;			// # of times covering container.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
	{
		Shape_frame *shape = gwin->get_shape(*obj);
		int objx = obj->get_cx() - shape->get_xleft() + 
							1 + object_area.x;
		int objy = obj->get_cy() - shape->get_yabove() + 
							1 + object_area.y;
					// Does obj. appear to be placed?
		if (!object_area.has_point(objx, objy) ||
		    !object_area.has_point(objx + shape->get_xright() - 1,
					objy + shape->get_ybelow() - 1))
		{		// No.
			int px = curx + shape->get_width(),
			    py = cury + shape->get_height();
			if (px > endx)
				px = endx;
			if (py > endy)
				py = endy;
			obj->set_chunk(px - shape->get_xright(),
					py - shape->get_ybelow());
					// Mostly avoid overlap.
			curx += shape->get_width() - 1;
			if (curx >= endx)
			{
				cury += 8;
				curx = 0;
				if (cury >= endy)
					cury = 2*(++loop);
			}
		}
		gwin->paint_shape(box.x + obj->get_cx(),box.y + obj->get_cy(), 
				obj->get_shapenum(), obj->get_framenum());
		obj = obj->get_next();
	}
}

/*
 *	Close and delete.
 */

void Gump::close
	(
	Game_window *gwin
	)
{
	gwin->remove_gump(this);
}
