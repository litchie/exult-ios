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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "game.h"
#include "Jawbone_gump.h"
#include "contain.h"
#include "gamewin.h"
#include "objiter.h"
#include "misc_buttons.h"
#include "jawbone.h"

const int toothx[19] = { 34, 32, 31, 31, 28, 31, 27, 31, 40, 50,
						 57, 63, 72, 70, 75, 82, 83, 87, 0 };

const int toothy[19] = { 19, 30, 37, 44, 52, 57, 66, 77, 82, 84,
						 80, 71, 69, 61, 50, 42, 36, 32, 0 };

Jawbone_gump::Jawbone_gump
	(
	Jawbone_object *cont,	// Jawbone it represents.
	int initx, int inity 		// Coords. on screen.
	) : Gump(cont, initx, inity, game->get_shape("gumps/jawbone")),
		jawbone(cont)
{
	set_object_area(Rectangle(0, 0, 138, 116), 10, 109);
}


int Jawbone_gump::add(Game_object *obj, int mx, int my,	int sx, int sy,
					  bool dont_check)
{
	// Jawbone_object handles all the checks required
	return jawbone->add(obj, dont_check);
}

void Jawbone_gump::paint(Game_window *gwin)
{
	// Paint gump itself
	gwin->paint_shape(x, y, get_shape());

	// Paint red "checkmark".
	check_button->paint(gwin);

	jawbone->find_teeth();

	int i;	// Blame MSVC
	for (i=0; i<9; i++)
		if (jawbone->teeth[i])
			paint_tooth(gwin, i);
	for (i=17; i>8; i--)
		if (jawbone->teeth[i])
			paint_tooth(gwin, i);
}

void Jawbone_gump::paint_tooth(Game_window* gwin, int index)
{
	Shape_frame *shape = gwin->get_gump_shape(
								game->get_shape("gumps/tooth"), index);

	int objx = toothx[index];
	int objy = toothy[index];

	gwin->paint_shape(x + objx, y + objy, shape);
}

Game_object *Jawbone_gump::find_object(int mx, int my)
{
	Game_window* gwin = Game_window::get_game_window();

	jawbone->find_teeth();

	// get position relative to gump
	mx -= x;
	my -= y;

	int i;	// Blame MSVC

	// reverse of drawing order
	for (i=9; i<18; i++)
		if (jawbone->teeth[i] && on_tooth(mx, my, i)) {
			// set correct position (otherwise tooth won't be on mouse cursor)
			set_to_spot(jawbone->teeth[i], mx, my);
			return jawbone->teeth[i];
		}
	for (i=8; i>=0; i--)
		if (jawbone->teeth[i] && on_tooth(mx, my, i)) {
			// set correct position (otherwise tooth won't be on mouse cursor)
			set_to_spot(jawbone->teeth[i], mx, my);
			return jawbone->teeth[i];
		}

	return 0;
}

bool Jawbone_gump::on_tooth(int sx, int sy, int index)
{
	Game_window* gwin = Game_window::get_game_window();

	Shape_frame *shape = gwin->get_gump_shape(game->get_shape("gumps/tooth"), 
											  index);

	int objx = toothx[index];
	int objy = toothy[index];
	
	Rectangle r = gwin->get_shape_rect (shape, 0, 0);
	
	if (r.has_point (sx - objx, sy - objy) && 
			shape->has_point (sx - objx, sy - objy))
		return true;

	return false;
}

void Jawbone_gump::set_to_spot(Game_object *obj, int sx, int sy)
{
	if (obj->get_shapenum() != 559)  // not a serpent tooth?
		return;

	int fr = obj->get_framenum();
	Game_window *gwin = Game_window::get_game_window();
	
	// Get shape.
	Shape_frame *shape = gwin->get_shape(*obj);
	
	// Height and width
	int w = shape->get_width(), h = shape->get_height();
	
	// Set object's position.
	obj->set_chunk(sx + shape->get_xleft() - w/2,
		sy + shape->get_yabove() - h/2);
}

