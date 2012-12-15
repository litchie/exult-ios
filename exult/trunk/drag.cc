/*
 *	drag.cc - Dragging objects in Game_window.
 *
 *  Copyright (C) 2000-2011  The Exult Team
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

#include <iostream>	/* Debugging */
#include "gamewin.h"
#include "gamemap.h"
#include "mouse.h"
#include "drag.h"
#include "Gump_button.h"
#include "Gump.h"
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
 *	Create for a given (newly created) object.
 */

Dragging_info::Dragging_info
	(
	Game_object *newobj		// Object NOT in world.  This is
					//   dropped, or deleted.
	) : obj(newobj), is_new(true), gump(0), button(0), old_pos(-1, -1, -1),
	    old_foot(0, 0, 0, 0), old_lift(-1), quantity(obj->get_quantity()),
	    readied_index(-1), mousex(-1), mousey(-1), paintx(-1000), painty(-1000),
	    mouse_shape(Mouse::mouse->get_shape()), rect(0, 0, 0, 0),
	    save(0), okay(true), possible_theft(false)
	{
	rect = gwin->get_shape_rect(obj);
	rect.enlarge(8);		// Make a little bigger.
					// Create buffer to backup background.
	//save = gwin->get_win()->create_buffer(rect.w, rect.h);
	}

/*
 *	Begin a possible drag.
 */

Dragging_info::Dragging_info
	(
	int x, int y			// Mouse position.
	) : obj(0), is_new(false), gump(0), button(0), old_pos(-1, -1, -1),
	    old_foot(0, 0, 0, 0), old_lift(-1), quantity(0),
	    readied_index(-1), mousex(x), mousey(y),
	    mouse_shape(Mouse::mouse->get_shape()), rect(0, 0, 0, 0),
	    save(0), okay(false), possible_theft(false)
	{
					// First see if it's a gump.
	gump = gumpman->find_gump(x, y);
	if (gump)
		{
		obj = gump->find_object(x, y);
		if (obj)
			{		// Save location info.
			gump->get_shape_location(obj, paintx, painty);
			old_pos = Tile_coord(obj->get_tx(), obj->get_ty(), 0);
			}
		else if ((button = gump->on_button(x, y)) != 0)
			{
			gump = 0;
			if (!button->is_draggable())
				return;
			button->push(1);
					// Pushed button, so make noise.
			Audio::get_ptr()->play_sound_effect(
					Audio::game_sfx(73));
			gwin->set_painted();
			}
		else if (gump->is_draggable())
			{		// Dragging whole gump.
			paintx = gump->get_x();
			painty = gump->get_y();
			cout << "(x,y) rel. to gump is (" << (x-paintx) << ", " <<
							(y-painty) << ")"<<endl;
			}
		else			// the gump isn't draggable
			return;
		}
	else if (x >0 && y > 0 && x < gwin->get_width() && y < gwin->get_height())	// Not found in gump?
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
		{
		quantity = obj->get_quantity();
					// Save original lift.
		old_lift = obj->get_outermost()->get_lift();
		}
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
	int deltax = abs(x - mousex),
		deltay = abs(y - mousey);
	if (deltax <= 2 && deltay <= 2)
		return (false);		// Wait for greater motion.
	if (obj)
		{			// Don't want to move walls.
		if (!cheat.in_hack_mover() && !obj->is_dragable() &&
					      !obj->get_owner())
			{
			Mouse::mouse->flash_shape(Mouse::tooheavy);
			obj = 0;
			gump = 0;
			okay = false;
			return (false);
			}
		Game_object *owner = obj->get_outermost();
		if (owner == obj)
			{
			if (!cheat.in_hack_mover() && 
			    !Fast_pathfinder_client::is_grabable(gwin->get_main_actor(), obj))
				{
				Mouse::mouse->flash_shape(Mouse::blocked);
				obj = 0;
				okay = false;
				return (false);
				}
			}
		}
	Mouse::mouse->set_shape(Mouse::hand);
					// Store original pos. on screen.
	rect = gump ? (obj ? gump->get_shape_rect(obj) : gump->get_dirty())
					: gwin->get_shape_rect(obj);
	if (gump)			// Remove from actual position.
		{
		if (obj)
			{
			Container_game_object *owner = 
						gump->get_cont_or_actor(x,y);
					// Get the object
			Game_object *owner_obj = 
					gump->get_owner()->get_outermost(); 
			Main_actor *main_actor = gwin->get_main_actor();
			// Check the range
			if (!cheat.in_hack_mover() &&
			    !Fast_pathfinder_client::is_grabable(main_actor, owner_obj))
				{
				obj = 0;
				gump = 0;
				okay = false;
				Mouse::mouse->flash_shape(Mouse::outofrange);
				return false;
				}
			if (owner)
				readied_index = owner->find_readied(obj);
			gump->remove(obj);
			}
		else
			gumpman->remove_gump(gump);
		}
	else
		obj->remove_this(true);	// This SHOULD work (jsf 21-12-01).
					// Make a little bigger.
	//rect.enlarge(c_tilesize + obj ? 0 : c_tilesize/2);
	rect.enlarge(deltax > deltay ? deltax : deltay);
	
	Rectangle crect = gwin->clip_to_win(rect);
	gwin->paint(crect);		// Paint over obj's. area.
					// Create buffer to backup background.
	//save = gwin->get_win()->create_buffer(rect.w, rect.h);
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
	if (rect.w == 0)
		{
		if (!start(x, y))
			return false;
		}
	else if (save)				// Not first time?  Restore beneath.
		gwin->get_win()->put(save, rect.x, rect.y);
	else
		gwin->add_dirty(gwin->clip_to_win(rect));
	gwin->set_painted();
	int deltax = x - mousex, deltay = y - mousey;
	mousex = x;
	mousey = y;
					// Shift to new position.
	rect.shift(deltax, deltay);
	paintx += deltax;
	painty += deltay;
	if (gump && !obj)			// Dragging a gump?
		gump->set_pos(paintx, painty);
	gwin->add_dirty(gwin->clip_to_win(rect));
	return (true);
	}

/*
 *	Paint object being moved.
 */

void Dragging_info::paint
	(
	)
	{
	if (!rect.w)			// Not moved enough yet?
		return;
	if (save)			// Save background.
		gwin->get_win()->get(save, rect.x, rect.y);
	if (obj)
		{
		if (obj->get_flag(Obj_flags::invisible))
			obj->paint_invisible(paintx, painty);
		else
			obj->paint_shape(paintx, painty);
		}
	else if (gump)
		{
		gump->paint();
		}
	}

/*
 *	Mouse was released, so drop object. 
 *      Return true iff the dropping mouseclick has been handled. 
 *		(by buttonpress, drag)
 */

bool Dragging_info::drop
	(
	int x, int y,			// Mouse pos.
	bool moved			// has mouse moved from starting pos?
	)
	{
	bool handled = moved;
	Mouse::mouse->set_shape(mouse_shape);
	if (button)
		{
		button->unpush(1);
		if (button->on_button(x, y))
					// Clicked on button.
			button->activate(1);
		handled = true;
		}
	else if (!obj)			// Only dragging a gump?
		{
		if (!gump)
			return handled;
		if (!moved)		// A click just raises it to the top.
			gumpman->remove_gump(gump);
		gumpman->add_gump(gump);
		}
	else if (!moved)		// For now, if not moved, leave it.
		return handled;
	else if (!drop(x, y))		// Drop it.
		put_back();		// Wasn't (all) moved.
	obj = 0;			// Clear so we don't paint them.
	gump = 0;
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
	if (!owner->get_flag(Obj_flags::in_party))
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
		{			// And don't check for volume!
					// Restore saved vals.
		obj->set_shape_pos(old_pos.tx, old_pos.ty);
		// 1st try with dont_check==false so usecode gets called.
		if (!gump->add(obj, -2, -2, -2, -2, false))
			gump->add(obj, -2, -2, -2, -2, true);
		}
	else if (is_new)
		{
		obj->set_invalid();	// It's not in the world.
		obj->remove_this();
		}
	else				// Normal object.  Put it back.
		obj->move(old_pos);
	obj = 0;			// Just to be safe.
	is_new = false;
	}

/*
 *	Drop object on a gump.
 *
 *	Output:	False if not (all) of object was dropped.
 */

bool Dragging_info::drop_on_gump
	(
	int x, int y,			// Mouse position.
	Game_object *to_drop,		// == obj if whole thing.
	Gump *on_gump			// Gump to drop it on.
	)
	{
	Game_object *owner_obj = on_gump->get_owner();
	if (owner_obj) owner_obj = owner_obj->get_outermost();
	Main_actor *main_actor = gwin->get_main_actor(); 
	// Check the range
	if (owner_obj && !cheat.in_hack_mover() &&
		!Fast_pathfinder_client::is_grabable(main_actor, owner_obj))
		{			// Object was not grabable
		Mouse::mouse->flash_shape(Mouse::outofrange);
		return false;
		}
	if (!Check_weight(gwin, to_drop, on_gump->get_cont_or_actor(x,y)))
		return false;
	if (on_gump != gump)		// Not moving within same gump?
		possible_theft = true;
					// Add, and allow to combine.
	if (!on_gump->add(to_drop, x, y, paintx, painty, false, true))
		{			// Failed.
		if (to_drop != obj)
			{		// Watch for partial drop.
			int nq = to_drop->get_quantity();
			if (nq < quantity)
				obj->modify_quantity(quantity - nq);
			}
		Mouse::mouse->flash_shape(Mouse::wontfit);
		return false;
		}
	return true;
	}

/*
 *	See if there's something blocking an object at a given point.
 */

static bool Is_inaccessible
	(
	Game_window *gwin,
	Game_object *obj,
	int x, int y
	)
	{
	Game_object *block = gwin->find_object(x, y);
	if (block && block != obj && !block->is_dragable())
		return true;
	else
		return false;
	}

/*
 *	Drop object onto the map.
 *
 *	Output:	False if not (all) of object was dropped.
 */

bool Dragging_info::drop_on_map
	(
	int x, int y,			// Mouse position.
	Game_object *to_drop		// == obj if whole thing.
	)
	{
		// Attempting to drop off screen?
	if (x < 0 || y < 0 || x >= gwin->get_width() || y >= gwin->get_height())
	{
		Mouse::mouse->flash_shape(Mouse::redx);
		Audio::get_ptr()->play_sound_effect(Audio::game_sfx(76));
		return false;
	}

	int max_lift = cheat.in_hack_mover() ? 255 :
					gwin->get_main_actor()->get_lift() + 5;
	int skip = gwin->get_render_skip_lift();
	if (max_lift >= skip)		// Don't drop where we cannot see.
		max_lift = skip - 1;
					// Drop where we last painted it.
	int posx = paintx, posy = painty;
	if (posx == -1000)		// Unless we never painted.
		{ posx = x; posy = y; }
	int lift;
					// Was it dropped on something?
	Game_object *found = gwin->find_object(x, y);
	int dropped = 0;		// 1 when dropped.
	if (found && found != obj)
		{
		if (!Check_weight(gwin, to_drop, found))
			return false;
		if (found->drop(to_drop))
			{
			dropped = 1;
			possible_theft = true;
			}
					// Try to place on 'found'.
		else if ((lift = found->get_lift() +
			     found->get_info().get_3d_height()) <= max_lift)
			dropped = gwin->drop_at_lift(to_drop,posx, posy, lift);
		else
			{		// Too high.
			Mouse::mouse->flash_shape(Mouse::redx);
			Audio::get_ptr()->play_sound_effect(
							Audio::game_sfx(76));
			return false;
			}
		}
					// Find where to drop it, but stop if
					//   it will end up hidden (-1).
	for (lift = old_lift; !dropped && lift <= max_lift; lift++)
		dropped = gwin->drop_at_lift(to_drop, posx, posy, lift);

	if (dropped <= 0)
		{
		Mouse::mouse->flash_shape(Mouse::blocked);
		Audio::get_ptr()->play_sound_effect(Audio::game_sfx(76));
		return false;
		}
					// Moved more than 2 tiles.
	if (!gump && !possible_theft &&
				to_drop->get_tile().distance(old_pos) > 2)
		possible_theft = true;
	return true;
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
					// Get orig. loc. info.
	int oldcx = old_pos.tx/c_tiles_per_chunk, 
	    oldcy = old_pos.ty/c_tiles_per_chunk;
	Game_object *to_drop = obj;	// If quantity, split it off.
					// Being liberal about taking stuff:
	int okay_to_move = to_drop->get_flag(Obj_flags::okay_to_take);
	int old_top = old_pos.tz + obj->get_info().get_3d_height();
					// First see if it's a gump.
	Gump *on_gump = gumpman->find_gump(x, y);
					// Don't prompt if within same gump 
					//or if alternate drop is enabled (ctrl inverts).

	Uint8 *keystate = SDL_GetKeyState(NULL);
	bool drop = (keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) ? 
	             gwin->get_alternate_drop() : !gwin->get_alternate_drop();
	bool temp = obj->get_flag(Obj_flags::is_temporary);

	if (quantity > 1 && (!on_gump || on_gump != gump) && drop)
		quantity = gumpman->prompt_for_number(0, quantity, 1, quantity);
		
	if (quantity <= 0)
		return false;
	if (quantity < obj->get_quantity())
		{			// Need to drop a copy.
		to_drop = gmap->create_ireg_object(
				obj->get_shapenum(), obj->get_framenum());
		to_drop->modify_quantity(quantity - 1);
		if (okay_to_move)	// Make sure copy is okay to take.
			to_drop->set_flag(Obj_flags::okay_to_take);
		if (temp)
			to_drop->set_flag(Obj_flags::is_temporary);
		}
					// Drop it.
	if (!(on_gump ? drop_on_gump(x, y, to_drop, on_gump)
			  : drop_on_map(x, y, to_drop)))
		return false;
					// Make a 'dropped' sound.
	Audio::get_ptr()->play_sound_effect(Audio::game_sfx(74));
	if (!gump)			// Do eggs where it came from.
		gmap->get_chunk(oldcx, oldcy)->activate_eggs(obj,
			    old_pos.tx, old_pos.ty, old_pos.tz, 
					old_pos.tx, old_pos.ty);
					// Special:  BlackSword in SI.
	else if (readied_index >= 0 && obj->get_shapenum() == 806)
					// Do 'unreadied' usecode.
		gump->get_cont_or_actor(x,y)->call_readied_usecode(
			readied_index, obj, Usecode_machine::unreadied);
#if 0	/* Now done in Actor::drop */
	if (on_gump)			// Do 'readied' usecode.
		{
		Container_game_object *owner = on_gump->get_cont_or_actor(x,y);
		int index = owner ? owner->find_readied(obj) : -1;
		if (index >= 0)
			owner->call_readied_usecode(index,
					obj, Usecode_machine::readied);
		}
#endif
					// On a barge?
	Barge_object *barge = gwin->get_moving_barge();
	if (barge)
		barge->set_to_gather();	// Refigure what's on barge.
					// Check for theft.
	if (!okay_to_move && !cheat.in_hack_mover() && possible_theft &&
	    !gwin->is_in_dungeon())
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
	bool handled = dragging->drop(x, y, moved);
	delete dragging;
	dragging = 0;
	return handled;
	}

/*
 *	Try to drop at a given lift.  Note:  None of the drag state variables
 *	may be used here, as it's also called from the outside.
 *
 *	Output:	1 if successful.
 *		0 if blocked
 *		-1 if it would end up hidden by a non-moveable object.
 */

int Game_window::drop_at_lift
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
	Map_chunk *chunk = map->get_chunk(cx, cy);
	int lift;			// Can we put it here?
	Shape_info& info = to_drop->get_info();
	int frame = to_drop->get_framenum();
	int xtiles = info.get_3d_xtiles(frame), ytiles = info.get_3d_ytiles(frame);
	int max_drop, move_flags;
	if (cheat.in_hack_mover())
		{
		max_drop = at_lift - cheat.get_edit_lift();
//		max_drop = max_drop < 0 ? 0 : max_drop;
		if (max_drop < 0)	// Below lift we're editing?
			return 0;
		move_flags = MOVE_WALK|MOVE_MAPEDIT;
		}
	else
		{			// Allow drop of 5;
		max_drop = 5;
		move_flags = MOVE_WALK;
		}
	if (Map_chunk::is_blocked(info.get_3d_height(), at_lift,
		tx - xtiles + 1, ty - ytiles + 1, xtiles, ytiles, 
			lift, move_flags, max_drop) ||
	   (!cheat.in_hack_mover() &&
					// Check for path to location.
	    !Fast_pathfinder_client::is_grabable(main_actor, 
			Tile_coord(tx, ty, lift))))
		return 0;

	to_drop->set_invalid();
	to_drop->move(tx, ty, lift);
	Rectangle rect = get_shape_rect(to_drop);
					// Avoid dropping behind walls.
	if (Is_inaccessible(this, to_drop, rect.x + 2, rect.y + 2) &&
	    Is_inaccessible(this, to_drop, 
				rect.x + rect.w - 3, rect.y + 2) &&
	    Is_inaccessible(this, to_drop, 
				rect.x + 2, rect.y + rect.h - 3) &&
	    Is_inaccessible(this, to_drop, 
				rect.x + rect.w - 3, rect.y + rect.h - 3) &&
		Is_inaccessible(this, to_drop, 
				rect.x + (rect.w >> 1), rect.y + (rect.h >> 1)))
		{
		to_drop->remove_this(true);
		return -1;
		}
#ifdef DEBUG
	cout << "Dropping object at (" << tx << ", " << ty << ", " << lift
			 << ")"<<endl;
#endif
					// On an egg?
	chunk->activate_eggs(to_drop, tx, ty, lift, tx, ty);

	if (to_drop == main_actor) {
		center_view(to_drop->get_tile());
		paint();
	}
	return (1);
	}

