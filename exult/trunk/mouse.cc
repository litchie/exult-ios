/*
 *	mouse.cc - Mouse pointers.
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_mouse.h"
#include "SDL_timer.h"
#include "mouse.h"
#include "gamewin.h"
#include "fnames.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "barge.h"
#include "actors.h"

short Mouse::short_arrows[8] = {8, 9, 10, 11, 12, 13, 14, 15};
short Mouse::med_arrows[8] = {16, 17, 18, 19, 20, 21, 22, 23};
short Mouse::long_arrows[8] = {24, 25, 26, 27, 28, 29, 30, 31};

short Mouse::short_combat_arrows[8] = {32, 33, 34, 35, 36, 37, 38, 39};
short Mouse::med_combat_arrows[8] = {40, 41, 42, 43, 44, 45, 46, 47};

Mouse* Mouse::mouse = 0;
bool Mouse::mouse_update = false;

/*
 *	Create.
 */

Mouse::Mouse
	(
	Game_window *gw			// Where to draw.
	) : gwin(gw), iwin(gwin->get_win()),backup(0),cur_framenum(0),cur(0),avatar_speed(slow_speed)
{
	SDL_GetMouseState(&mousex, &mousey);
	mousex /= iwin->get_scale();
	mousey /= iwin->get_scale();
	pointers.load(POINTERS);
	Init();
	set_shape(get_short_arrow(east));		// +++++For now.
}
	
Mouse::Mouse
	(
	Game_window *gw,		// Where to draw.
	DataSource &shapes
	) : gwin(gw), iwin(gwin->get_win()),backup(0),cur_framenum(0),cur(0),avatar_speed(slow_speed) 
{
	SDL_GetMouseState(&mousex, &mousey);
	mousex /= iwin->get_scale();
	mousey /= iwin->get_scale();
	pointers.load(&shapes);
	Init();
	set_shape0(0);
}
	
void Mouse::Init()
{
	int cnt = pointers.get_num_frames();
	int maxleft = 0, maxright = 0, maxabove = 0, maxbelow = 0;
	for (int i = 0; i < cnt; i++)
	{
		Shape_frame *frame = pointers.get_frame(i);
		int xleft = frame->get_xleft(), xright = frame->get_xright();
		int yabove = frame->get_yabove(), ybelow = frame->get_ybelow();
		if (xleft > maxleft)
			maxleft = xleft;
		if (xright > maxright)
			maxright = xright;
		if (yabove > maxabove)
			maxabove = yabove;
		if (ybelow > maxbelow)
			maxbelow = ybelow;
	}
	int maxw = maxleft + maxright, maxh = maxabove + maxbelow;
					// Create backup buffer.
	backup = iwin->create_buffer(maxw, maxh);
	box.w = maxw;
	box.h = maxh;
	
	onscreen = 0;                   // initially offscreen
}

/*
 *	Delete.
 */

Mouse::~Mouse
	(
	)
{
	delete backup;
}

/*
 *	Show the mouse.
 */

void Mouse::show
	(
	)
{
	if (!onscreen)
	{
		onscreen = 1;
					// Save background.
		iwin->get(backup, box.x, box.y);
					// Paint new location.
		cur->paint_rle(iwin->get_ib8(), mousex, mousey);
	}
}

/*
 *	Set to new shape.  Should be called after checking that frame #
 *	actually changed.
 */

void Mouse::set_shape0
	(
	int framenum
	)
{
	cur_framenum = framenum;
	cur = pointers.get_frame(framenum); 
					// Set backup box to cover mouse.
	box.x = mousex - cur->get_xleft();
	box.y = mousey - cur->get_yabove();
	dirty = dirty.add(box);		// Update dirty area.
}

/*
 *	Set to an arbitrary location.
 */

void Mouse::set_location
	(
	int x, int y			// Mouse position.
	)
{
	mousex = x;
	mousey = y;
	box.x = mousex - cur->get_xleft();
	box.y = mousey - cur->get_yabove();
}

/*
 *	Flash a desired shape for about 1/2 second.
 */

void Mouse::flash_shape
	(
	Mouse_shapes flash
	)
{
	Mouse::Mouse_shapes saveshape = get_shape();
	hide();
	set_shape(flash);
	show();
	gwin->show(1);
	SDL_Delay(600);
	hide();
	gwin->paint();
	set_shape(saveshape);
	gwin->set_painted();
}


/*
 *	Set default cursor
 */

void Mouse::set_speed_cursor()
{
	Game_window *gwin = Game_window::get_game_window();

	int cursor = dontchange;
	int ax, ay;			// Get Avatar/barge screen location.

	Barge_object *barge = gwin->get_moving_barge();
	if (barge)
	{			// Use center of barge.
		gwin->get_shape_location(barge, ax, ay);
		ax -= barge->get_xtiles()*(c_tilesize/2);
		ay -= barge->get_ytiles()*(c_tilesize/2);
	}
	else				
		gwin->get_shape_location(gwin->get_main_actor(), ax, ay);

	int dy = ay - mousey, dx = mousex - ax;
	Direction dir = Get_direction(dy, dx);
	int dist = dy*dy + dx*dx;
	if (dist < 40*40)
	{
		if(gwin->in_combat())
			cursor = get_short_combat_arrow(dir);
		else
			cursor = get_short_arrow(dir);
		avatar_speed = slow_speed;
	}
	else if (dist < 75*75)
	{
		if(gwin->in_combat())
			cursor = get_medium_combat_arrow(dir);
		else
			cursor = get_medium_arrow(dir);
		avatar_speed = medium_speed;
	}
	else
	{		// No long arrow in combat: use medium
		if(gwin->in_combat())
			cursor = get_medium_combat_arrow(dir);
		else
			cursor = get_long_arrow(dir);
		avatar_speed = fast_speed;
	}

	Gump_manager *gump_man = gwin->get_gump_man();
	Gump *gump = 0;

	if (gump_man->showing_gumps())
		if (gump = gump_man->find_gump(mousex, mousey))
			if (!gump->no_handcursor())
				cursor = hand;

	set_shape(cursor);
}
