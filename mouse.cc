/*
 *	mouse.cc - Mouse pointers.
 *
 *  Copyright (C) 2000-2002  The Exult Team
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

#include "SDL_mouse.h"
#include "SDL_timer.h"
#include "mouse.h"
#include "gamewin.h"
#include "fnames.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "barge.h"
#include "actors.h"
#include "cheat.h"
#include "combat_opts.h"
#include "combat.h"
#include "schedule.h" /* To get Schedule::combat */

#ifndef _MSC_VER
using std::max;
#endif

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
	) : gwin(gw), iwin(gwin->get_win()),backup(0),box(0,0,0,0),dirty(0,0,0,0), cur_framenum(0),cur(0),avatar_speed(100*gwin->get_std_delay()/slow_speed_factor)
{
	SDL_GetMouseState(&mousex, &mousey);
	mousex /= gwin->get_fastmouse() ? 1 : iwin->get_scale();
	mousey /= gwin->get_fastmouse() ? 1 : iwin->get_scale();
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_POINTERS))
		pointers.load(PATCH_POINTERS);
	else
		pointers.load(POINTERS);
	Init();
	set_shape(get_short_arrow(east));		// +++++For now.
}
	
Mouse::Mouse
	(
	Game_window *gw,		// Where to draw.
	DataSource &shapes
	) : gwin(gw), iwin(gwin->get_win()),backup(0),box(0,0,0,0),dirty(0,0,0,0),cur_framenum(0),cur(0),avatar_speed(100*gwin->get_std_delay()/slow_speed_factor)
{
	SDL_GetMouseState(&mousex, &mousey);
	mousex /= gwin->get_fastmouse() ? 1 : iwin->get_scale();
	mousey /= gwin->get_fastmouse() ? 1 : iwin->get_scale();
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
//		cur->paint_rle(iwin->get_ib8(), mousex, mousey);
		cur->paint_rle(mousex, mousey);
	}
}

/*
 *	Move cursor
 */

void Mouse::move(int x, int y) {
	bool warp = false;
	if (x >= gwin->get_width()) {
		x = gwin->get_width() - 1;
		warp = true;
	}
	if (y >= gwin->get_height()) {
		y = gwin->get_height() - 1;
		warp = true;
	}
	if (warp)
		SDL_WarpMouse(x, y);
#ifdef DEBUG
	if (onscreen)
		std::cerr << "Trying to move mouse while onscreen!" << std::endl;
#endif
	// Shift to new position.
	box.shift(x - mousex, y - mousey);
	dirty = dirty.add(box);	// Enlarge dirty area.
	mousex = x;
	mousey = y;
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
	while (!cur)			// For newly-created games.
		cur = pointers.get_frame(--framenum);
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
	Mouse_shapes saveshape = get_shape();
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
	Game_window *gwin = Game_window::get_instance();
	Gump_manager *gump_man = gwin->get_gump_man();
   
	int cursor = dontchange;
	int ax, ay;			// Get Avatar/barge screen location.

    // Check if we are in dont_move mode, in this case display the hand cursor
	if (gwin->main_actor_dont_move())
	        cursor = hand;
			/* Can again, optionally move in gump mode. */
	else if (gump_man->gump_mode()) {
		if (gump_man->gumps_dont_pause_game())
		{
    		Gump *gump = gump_man->find_gump(mousex, mousey);
        
			if (gump && !gump->no_handcursor())
				cursor = hand;
		}
		else cursor = hand;
	}

	else if (gwin->get_dragging_gump()) cursor = hand;

    else if (cheat.in_map_editor()) 
    {
	switch (cheat.get_edit_mode())
	{
	case Cheat::move:
	    cursor = hand; break;
	case Cheat::paint:
	    cursor = short_combat_arrows[4]; break;	// Short S red arrow.
	case Cheat::paint_chunks:
	    cursor = med_combat_arrows[0]; break;	// Med. N red arrow.
#if 0
	case Cheat::select:
	    cursor = short_arrows[7]; break;		// Short NW green.
	case Cheat::hide:
	    cursor = redx; break;
#endif
	case Cheat::combo_pick:
	    cursor = greenselect; break;
	}
    }
    else if (Combat::is_paused())
	cursor = short_combat_arrows[0];	// Short N red arrow.
    if (cursor == dontchange)
    {
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
	Rectangle gamewin_dims = gwin->get_win_rect();
	float speed_section = max( max( -static_cast<float>(dx)/ax, static_cast<float>(dx)/(gamewin_dims.w-ax)), max(static_cast<float>(dy)/ay, -static_cast<float>(dy)/(gamewin_dims.h-ay)) );

	/* If there is a hostile NPC nearby, the avatar isn't allowed to
	 * move very fast
	 * Note that the range at which this occurs in the original is
	 * less than the "potential target" range- that is, if I go into
	 * combat mode, even when I'm allowed to run at full speed,
	 * I'll sometime charge off to kill someone "too far away"
	 * to affect a speed limit.
	 * I don't know whether this is taken into account by 
	 * get_nearby_npcs, but on the other hand, its a negligible point.
	 */
	Actor_queue nearby;
	gwin->get_nearby_npcs( nearby );

	bool nearby_hostile = false;
	for( Actor_queue::const_iterator it = nearby.begin(); it != nearby.end(); ++it ) {
		Actor *actor = *it;

		if( actor->get_alignment() >= Npc_actor::hostile && 
		    actor->get_schedule_type() == Schedule::combat && 
		    static_cast<Combat_schedule*>(actor->get_schedule())->
						has_started_battle() &&
		    !actor->is_dead() ) {
			/* TODO- I think invisibles still trigger the
			 * slowdown, verify this. */
			nearby_hostile = true;
			break; /* No need to bother checking the rest :P */
		}
	}

        if( speed_section < 0.4 )
        {
            if( gwin->in_combat() )
                cursor = get_short_combat_arrow( dir );
            else
                cursor = get_short_arrow( dir );
            avatar_speed = 100*gwin->get_std_delay()/slow_speed_factor;
        }
        else if( speed_section < 0.8 || gwin->in_combat() || nearby_hostile )
        {
            if( gwin->in_combat() )
                cursor = get_medium_combat_arrow( dir );
            else
                cursor = get_medium_arrow( dir );
            if( gwin->in_combat() || nearby_hostile )
                avatar_speed = 100*gwin->get_std_delay()/medium_combat_speed_factor;
            else
                avatar_speed = 100*gwin->get_std_delay()/medium_speed_factor;
        }
        else /* Fast - NB, we can't get here in combat mode; there is no
              * long combat arrow, nor is there a fast combat speed. */
        {           
	    cursor = get_long_arrow( dir );
            avatar_speed = 100*gwin->get_std_delay()/fast_speed_factor;
        }
    }
    
    if (cursor != dontchange)
        set_shape(cursor);
}
