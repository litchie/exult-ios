/*
 *	drag.cc - Dragging objects in Game_window.
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

#include <iostream>	/* Debugging */
#include "gamewin.h"
#include "drag.h"
#include "Gump_button.h"
#include "Gump.h"
#include "mouse.h"
#include "paths.h"
#include "actors.h"
#include "cheat.h"
#include "chunks.h"
#include "Audio.h"
#include "Gump_manager.h"
#include "ucmachine.h"
#include "barge.h"

using std::cout;
using std::endl;

/*
 *	Begin a possible drag.
 */

Dragging_info::Dragging_info
	(
	int x, int y			// Mouse position.
	) : obj(0), gump(0), button(0), old_pos(-1, -1, -1),
	    old_foot(0, 0, 0, 0), quantity(0),
	    readied_index(-1), mousex(x), mousey(y), rect(0, 0, 0, 0),
	    save(0), okay(false)
	{
	Game_window *gwin = Game_window::get_game_window();
					// First see if it's a gump.
	gump = gwin->get_gump_man()->find_gump(x, y);
	if (gump)
		{
		obj = gump->find_object(x, y);
		if (obj)
			gump->get_shape_location(obj,
					paintx, painty);
		else if ((button = gump->on_button(gwin, x, y)) != 0)
			{
			gump = 0;
			if (!button->is_draggable())
				return;
			button->push(gwin);
					// Pushed button, so make noise.
			Audio::get_ptr()->play_sound_effect(
					Audio::game_sfx(96));
			gwin->set_painted();
			}
		else if (gump->is_draggable())
			{		// Dragging whole gump.
			paintx = gump->get_x();
			painty = gump->get_y();
	cout << "(x,y) rel. to gump is (" << (x-paintx) << ", " <<
					(y-painty) << ")"<<endl;
			}
		else 			// the gump isn't draggable
			return;
		}
	else				// Not found in gump?
		{
		obj = gwin->find_object(x, y);
		if (!obj)
			return;
					// Get coord. where painted.
		gwin->get_shape_location(obj, paintx, painty);
		old_pos = obj->get_tile();
		old_foot = obj->get_footprint();
		}
	if (obj)
		quantity = obj->get_quantity();
	okay = true;
	}

/*
 *	Delete dragging info.
 */

Dragging_info::~Dragging_info
	(
	)
	{
	delete save;
	}

/*
 *	First motion.
 *
 *	Output:	false if failed.
 */

bool Dragging_info::start
	(
	int x, int y			// Mouse position.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (x - mousex <= 2 && mousex - x <= 2 &&
	    y - mousey <= 2 && mousey - y <= 2)
		return (false);		// Wait for greater motion.
	if (obj)
		{			// Don't want to move walls.
		if (!cheat.in_hack_mover() && !obj->is_dragable())
			{
			Mouse::mouse->flash_shape(Mouse::tooheavy);
			obj = 0;
			return (false);
			}
		Game_object *owner = obj->get_outermost();
		if (owner == obj)
			{
		    	if (!cheat.in_hack_mover() && 
				!Fast_pathfinder_client::is_grabable(
				   gwin->get_main_actor()->get_tile(), 
							obj->get_tile()))
				{
				Mouse::mouse->flash_shape(Mouse::blocked);
				obj = 0;
				return (false);
				}
			}
		else			// Inside something else?  Set lift.
			obj->set_lift(owner->get_lift());
		}
					// Store original pos. on screen.
	rect = gump ? (obj ? gump->get_shape_rect(obj) : gump->get_dirty())
					: gwin->get_shape_rect(obj);
	if (gump)			// Remove from actual position.
		if (obj)
			{
			Container_game_object *owner = gump->get_cont_or_actor(x,y);
			if (owner)
				readied_index = owner->find_readied(obj);
			gump->remove(obj);
			}
		else
			gwin->get_gump_man()->remove_gump(gump);
	else {			// +++++Really should remove_this(1),
//		gwin->get_chunk(old_pos.tx/c_tiles_per_chunk,
//				old_pos.ty/c_tiles_per_chunk)->remove(obj);
		obj->remove_this(true);	// This SHOULD work (jsf 21-12-01).
	}
					// Make a little bigger.
	int pad = obj ? 8 : 12;
	rect.x -= pad;		
	rect.y -= pad;
	rect.w += 2*pad;
	rect.h += 2*pad;
	Rectangle crect = gwin->clip_to_win(rect);
	gwin->paint(crect);		// Paint over obj's. area.
					// Create buffer to backup background.
	save = gwin->get_win()->create_buffer(rect.w, rect.h);
	return true;
	}

/*
 *	Mouse was moved while dragging.
 *
 *	Output:	true iff movement started/continued.
 */

bool Dragging_info::moved
	(
	int x, int y			// Mouse pos. in window.
	)
	{
	if (!obj && !gump)
		return (false);
	Game_window *gwin = Game_window::get_game_window();
	if (rect.w == 0)
		{
		if (!start(x, y))
			return false;
		}
	else				// Not first time?  Restore beneath.
		gwin->get_win()->put(save, rect.x, rect.y);
	int deltax = x - mousex, deltay = y - mousey;
	mousex = x;
	mousey = y;
					// Shift to new position.
	rect.shift(deltax, deltay);
					// Save background.
	gwin->get_win()->get(save, rect.x, rect.y);
	paintx += deltax;
	painty += deltay;
	if (obj)
		gwin->paint_shape(paintx, painty, obj->get_shape(),
						obj->is_translucent());
	else				// Obj whole gump.
		{
		gump->set_pos(paintx, painty);
		gump->paint(gwin);
		}
	gwin->set_painted();
	return (true);
	}

/*
 *	Mouse was released, so drop object. 
 *      Return true iff the dropping mouseclick has been handled. 
 *		(by buttonpress, drag)
 */

bool Dragging_info::mouse_up
	(
	int x, int y,			// Mouse pos.
	bool moved			// has mouse moved from starting pos?
	)
	{
	bool handled = moved;
	Game_window *gwin = Game_window::get_game_window();
	if (button)
		{
		button->unpush(gwin);
		if (button->on_button(gwin, x, y))
					// Clicked on button.
			button->activate(gwin);
		handled = true;
		}
	else if (!obj)			// Only dragging a gump?
		{
		if (!gump)
			return handled;
		if (!moved)		// A click just raises it to the top.
			gwin->get_gump_man()->remove_gump(gump);
		gwin->get_gump_man()->add_gump(gump);
		}
	else if (!moved)		// For now, if not moved, leave it.
		return handled;
	else if (!drop(x, y))		// Drop it.
		put_back();		// Wasn't (all) moved.
	gwin->paint();
	return handled;
	}

/*
 *	Check weight.
 *
 *	Output:	false if too heavy, with mouse flashed.
 */

static bool Check_weight
	(
	Game_window *gwin,
	Game_object *to_drop,
	Game_object *owner		// Who the new owner will be.
	)
	{
	if (cheat.in_hack_mover())	// hack-mover  -> no weight checking
		return true;

	if (!owner)
		return true;
	owner = owner->get_outermost();
	if (owner != gwin->get_main_actor() && owner->get_party_id() < 0)
		return true;		// Not a party member, so okay.
	int wt = owner->get_weight() + to_drop->get_weight();
	if (wt/10 > owner->get_max_weight())
		{
		Mouse::mouse->flash_shape(Mouse::tooheavy);
		return false;
		}
	return true;
	}

/*
 *	Put back object where it came from.
 */

void Dragging_info::put_back
	(
	)
	{
	if (gump)			// Put back remaining/orig. piece.
					// And don't check for volume!
		gump->add(obj, -2, -2, -2, -2, true);
	else
		obj->move(old_pos);
	obj = 0;			// Just to be safe.
	}

/*
 *	Drop at given position.
 *	++++++NOTE:  Potential problems here with 'to_drop' being deleted by
 *		call to add().  Probably add() should provide feedback if obj.
 *		is combined with another.
 *
 *	Output:	False if put_back() should be called.
 */

bool Dragging_info::drop
	(
	int x, int y			// Mouse position.
	)
	{
	extern int Prompt_for_number(int, int, int, int);
	Game_window *gwin = Game_window::get_game_window();
					// Get orig. loc. info.
	int oldcx = old_pos.tx/c_tiles_per_chunk, 
	    oldcy = old_pos.ty/c_tiles_per_chunk;
	bool dropped_in_something = false;	// For detecting theft.
	Game_object *to_drop = obj;	// If quantity, split it off.
					// Being liberal about taking stuff:
	int okay_to_move = to_drop->get_flag(Obj_flags::okay_to_take);
	int old_top = old_pos.tz + gwin->get_info(obj).get_3d_height();
					// First see if it's a gump.
	Gump *on_gump = gwin->get_gump_man()->find_gump(x, y);
					// Don't prompt if within same gump.
	if (quantity > 1 && (!on_gump || on_gump != gump))
		quantity = Prompt_for_number(0, quantity, 1, quantity);
	if (quantity <= 0)
		return false;
	if (quantity < obj->get_quantity())
		{			// Need to drop a copy.
		to_drop = gwin->create_ireg_object(
				obj->get_shapenum(), obj->get_framenum());
		to_drop->modify_quantity(quantity - 1);
		if (okay_to_move)	// Make sure copy is okay to take.
			to_drop->set_flag(Obj_flags::okay_to_take);
		}
	if (on_gump)			// Dropping on a gump?
		{
		if (!Check_weight(gwin, to_drop, 
					on_gump->get_cont_or_actor(x,y)))
			return false;
					// Add, and allow to combine.
		if (!on_gump->add(to_drop, x, y, paintx, painty, false, true))
			{
			if (to_drop != obj)
				{	// Watch for partial drop.
				int nq = to_drop->get_quantity();
				if (nq < quantity)
					obj->modify_quantity(quantity - nq);
				}
			Mouse::mouse->flash_shape(Mouse::wontfit);
			return false;
			}
		}
	else
		{			// Was it dropped on something?
		int max_lift = cheat.in_hack_mover() ? 13 :
					gwin->get_main_actor()->get_lift() + 4;
		int lift;
		Game_object *found = gwin->find_object(x, y);
		bool dropped = false;	// 1 when dropped.
		if (found && found != obj)
			{
			if (!Check_weight(gwin, to_drop, found))
				return false;
			if (found->drop(to_drop))
				dropped = dropped_in_something = true;
					// Try to place on 'found'.
			else if ((lift = found->get_lift() +
				     gwin->get_info(found).get_3d_height()) <= 
								max_lift)
				dropped = gwin->drop_at_lift(to_drop, 
							paintx, painty, lift);
			}
					// Find where to drop it.
		for (lift = obj->get_outermost()->get_lift(); 
					!dropped && lift <= max_lift; lift++)
			dropped = gwin->drop_at_lift(to_drop, paintx, 
								painty, lift);
		if (!dropped)
			{
			Mouse::mouse->flash_shape(Mouse::blocked);
			Audio::get_ptr()->play_sound_effect(
							Audio::game_sfx(76));
			return false;
			}
		}
	if (!gump)			// Do eggs where it came from.
		gwin->get_chunk(oldcx, oldcy)->activate_eggs(obj,
			    old_pos.tx, old_pos.ty, old_pos.tz, 
					old_pos.tx, old_pos.ty);
	else if (readied_index >= 0)
					// Do 'unreadied' usecode.
		gump->get_cont_or_actor(x,y)->call_readied_usecode(
			gwin, readied_index, obj, Usecode_machine::unreadied);
	if (on_gump)			// Do 'readied' usecode.
		{
		Container_game_object *owner = on_gump->get_cont_or_actor(x,y);
		int index = owner ? owner->find_readied(obj) : -1;
		if (index >= 0)
			owner->call_readied_usecode(gwin, index,
					obj, Usecode_machine::readied);
		}
					// On a barge?
	Barge_object *barge = gwin->get_moving_barge();
	if (barge)
		barge->set_to_gather();	// Refigure what's on barge.
					// Check for theft.
	if (!okay_to_move && !cheat.in_hack_mover() &&
		    (gump != on_gump || dropped_in_something ||
					// Moving:
		     (!gump && to_drop->get_tile().distance(old_pos) > 2)))
		gwin->theft();			
	if (to_drop == obj)		// Whole thing?
		{			// Watch for stuff on top of it.
		if (old_foot.w > 0)
			Map_chunk::gravity(old_foot, old_top);
		return true;		// All done.
		}
					// Subtract quantity moved.
	obj->modify_quantity(-quantity);
	return false;			// Put back the rest.
	}

/*
 *	Begin a possible drag when the mouse button is depressed.  Also detect
 *	if the 'close' checkmark on a gump is being depressed.
 *
 *	Output:	true iff object selected for dragging
 */

bool Game_window::start_dragging
	(
	int x, int y			// Position in window.
	)
	{
	delete dragging;
	dragging = new Dragging_info(x, y);
	if (dragging->okay)
		return (true);		// Success, so far.
	delete dragging;
	dragging = 0;
	return false;
	}

/*
 *	Mouse moved while dragging.
 */

bool Game_window::drag
	(
	int x, int y			// Mouse position in window.
	)
	{
	return dragging ? dragging->moved(x, y) : false;
	}


/*
 *	Mouse was released, so drop object. 
 *      Return true iff the dropping mouseclick has been handled. 
 *		(by buttonpress, drag)
 *	Output:	MUST set dragging = 0.
 */

bool Game_window::drop_dragged
	(
	int x, int y,			// Mouse pos.
	bool moved			// has mouse moved from starting pos?
	)
	{
	if (!dragging)
		return false;
	bool handled = dragging->mouse_up(x, y, moved);
	delete dragging;
	dragging = 0;
	return handled;
	}

/*
 *	Try to drop at a given lift.  Note:  None of the drag state variables
 *	may be used here, as it's also called from the outside.
 *
 *	Output:	true if successful.
 */

bool Game_window::drop_at_lift
	(
	Game_object *to_drop,
	int x, int y,			// Pixel coord. in window.
	int at_lift
	)
	{
	x += at_lift*4 - 1;		// Take lift into account, round.
	y += at_lift*4 - 1;
	int tx = (scrolltx + x/c_tilesize)%c_num_tiles;
	int ty = (scrollty + y/c_tilesize)%c_num_tiles;
	int cx = tx/c_tiles_per_chunk;
	int cy = ty/c_tiles_per_chunk;
	Map_chunk *chunk = get_chunk(cx, cy);
	int lift;			// Can we put it here?
	Shape_info& info = shapes.get_info(to_drop->get_shapenum());
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int max_drop, move_flags;
	if (cheat.in_map_editor())
		{
		max_drop = at_lift - cheat.get_edit_lift();
//		max_drop = max_drop < 0 ? 0 : max_drop;
		if (max_drop < 0)	// Below lift we're editing?
			return false;
		move_flags = MOVE_WALK|MOVE_MAPEDIT;
		}
	else
		{			// Allow drop of 5;
		max_drop = 5;
		move_flags = MOVE_WALK;
		}
	if (!Map_chunk::is_blocked(info.get_3d_height(), at_lift,
		tx - xtiles + 1, ty - ytiles + 1, xtiles, ytiles, 
			lift, move_flags, max_drop) && 
	   (cheat.in_hack_mover() ||
					// Check for path to location.
	    Fast_pathfinder_client::is_grabable(
		main_actor->get_tile(), Tile_coord(tx, ty, lift))))
		{
		to_drop->set_invalid();
		to_drop->move(tx, ty, lift);
cout << "Dropping object at (" << tx << ", " << ty << ", " << lift
							<< ")"<<endl;
					// On an egg?
		chunk->activate_eggs(to_drop, tx, ty, lift, tx, ty);

		if (to_drop == main_actor) {
			center_view(to_drop->get_tile());
			paint();
		}

		return (true);
		}
	return (false);
	}

