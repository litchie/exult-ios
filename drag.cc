/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Drag.cc - Dragging objects in Game_window.
 **
 **	Written: 3/2/2000 - JSF
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

#include <iostream.h>	/* Debugging */
#include "gamewin.h"
#include "gumps.h"
#include "mouse.h"
#include "paths.h"
#include "actors.h"

extern bool hack_mover;
extern Mouse *mouse;

/*
 *	Begin a possible drag when the mouse button is depressed.  Also detect
 *	if the 'close' checkmark on a gump is being depressed.
 *
 *	Output:	1 if object selected for dragging, else 0.
 */

int Game_window::start_dragging
	(
	int x, int y			// Position in window.
	)
	{
	dragging = 0;
	dragging_gump = 0;
	dragging_gump_button = 0;
	dragging_mousex = x;
	dragging_mousey = y;
	dragging_rect = Rectangle(0, 0, 0, 0);
	delete dragging_save;
	dragging_save = 0;
					// First see if it's a gump.
	dragging_gump = find_gump(x, y);
	if (dragging_gump)
		{
		dragging = dragging_gump->find_object(x, y);
		if (dragging)
			dragging_gump->get_shape_location(dragging,
					dragging_paintx, dragging_painty);
		else if ((dragging_gump_button = 
				dragging_gump->on_button(this, x, y)) != 0)
			{
			dragging_gump = 0;
			dragging_gump_button->push(this);
			painted = 1;
			}
		else
			{		// Dragging whole gump.
			dragging_paintx = dragging_gump->get_x();
			dragging_painty = dragging_gump->get_y();
cout << "(x,y) rel. to gump is (" << (x-dragging_paintx) << ", " <<
		(y-dragging_painty) << ")"<<endl;
			}
		}
	else if (!dragging)		// Not found in gump?
		{
		dragging = find_object(x, y);
		if (!dragging)
			return (0);
					// Get coord. where painted.
		get_shape_location(dragging, dragging_paintx, dragging_painty);
		}
	return (1);
	}

/*
 *	Mouse was moved while dragging.
 *
 *	Output:	1 if movement started/continued.
 */

int Game_window::drag
	(
	int x, int y			// Mouse pos. in window.
	)
	{
	if (!dragging && !dragging_gump)
		return (0);
	if (dragging_rect.w == 0)
		{			// First motion.
		if (x - dragging_mousex <= 2 && dragging_mousex - x <= 2 &&
		    y - dragging_mousey <= 2 && dragging_mousey - y <= 2)
			return (0);	// Wait for greater motion.
		if (dragging)
			{		// Don't want to move walls.
			if (!hack_mover && !dragging->is_dragable())	
				{
				mouse->flash_shape(Mouse::tooheavy);
				dragging = 0;
				return (0);
				}
			Game_object *owner = dragging->get_outermost();
			if (owner == dragging)
				{
			    	if (!hack_mover && !Fast_pathfinder_client::is_grabable(
					main_actor->get_abs_tile_coord(),
					dragging->get_abs_tile_coord()))
					{
					mouse->flash_shape(Mouse::blocked);
					dragging = 0;
					return (0);
					}
				}
			else		// Inside something else?  Set lift.
				dragging->set_lift(owner->get_lift());
			}
					// Store original pos. on screen.
		dragging_rect = dragging_gump ?
			(dragging ? dragging_gump->get_shape_rect(dragging)
				  : get_gump_rect(dragging_gump))
			: get_shape_rect(dragging);
					// Remove from actual position.
		if (dragging_gump)
			if (dragging)
				dragging_gump->remove(dragging);
			else
				dragging_gump->remove_from_chain(
							open_gumps);
		else
			get_objects(dragging->get_cx(), 
				dragging->get_cy())->remove(dragging);
					// Make a little bigger.
		int pad = dragging ? 8 : 12;
		dragging_rect.x -= pad;		
		dragging_rect.y -= pad;
		dragging_rect.w += 2*pad;
		dragging_rect.h += 2*pad;
		Rectangle rect = clip_to_win(dragging_rect);
		paint(rect);		// Paint over obj's. area.
					// Create buffer to backup background.
		dragging_save = win->create_buffer(dragging_rect.w,
							dragging_rect.h);
		}
	else				// Not first time?  Restore beneath.
		win->put(dragging_save, dragging_rect.x, dragging_rect.y);
	int deltax = x - dragging_mousex, deltay = y - dragging_mousey;
	dragging_mousex = x;
	dragging_mousey = y;
					// Shift to new position.
	dragging_rect.shift(deltax, deltay);
					// Save background.
	win->get(dragging_save, dragging_rect.x, dragging_rect.y);
	dragging_paintx += deltax;
	dragging_painty += deltay;
	if (dragging)
		paint_shape(dragging_paintx, dragging_painty, 
			dragging->get_shapenum(), dragging->get_framenum());
	else				// Dragging whole gump.
		{
		dragging_gump->set_pos(dragging_paintx, dragging_painty);
		dragging_gump->paint(this);
		}
	painted = 1;
	return (1);
	}

/*
 *	Mouse was released, so drop object.
 */

void Game_window::drop_dragged
	(
	int x, int y,			// Mouse pos.
	int moved			// 1 if mouse moved from starting pos.
	)
	{
	if (dragging_gump_button)
		{
		dragging_gump_button->unpush(this);
		if (dragging_gump_button->on_button(this, x, y))
					// Clicked on button.
			dragging_gump_button->activate(this);
		dragging_gump_button = 0;
		}
	else if (!dragging)		// Only dragging a gump?
		{
		if (!dragging_gump)
			return;
		if (!moved)		// A click just raises it to the top.
			dragging_gump->remove_from_chain(open_gumps);
		dragging_gump->append_to_chain(open_gumps);
		}
	else if (!moved)		// For now, if not moved, leave it.
		return;
	else
		drop(x, y);		// Drop it.
	dragging = 0;
	dragging_gump = 0;
	delete dragging_save;
	dragging_save = 0;
	paint();
	}

/*
 *	Drop at given position.
 */

void Game_window::drop
	(
	int x, int y			// Mouse position.
	)
	{
	extern int Prompt_for_number(int, int, int, int);
					// Get orig. loc. info.
	int oldcx = dragging->get_cx(), oldcy = dragging->get_cy();
	int oldtx = oldcx*tiles_per_chunk + dragging->get_tx(),
	    oldty = oldcy*tiles_per_chunk + dragging->get_ty();
	int dropped = 0;		// 1 when dropped.
	Game_object *to_drop = dragging;// If quantity, split it off.
					// Save original footprint.
	Rectangle old_foot(0, 0, 0, 0);
	if (!dragging_gump)		// Get old footprint, top in world.
		old_foot = dragging->get_footprint();
	int old_top = dragging->get_lift() + 
					get_info(dragging).get_3d_height();
					// First see if it's a gump.
	Gump_object *on_gump = find_gump(x, y);
					// Check for quantity.
	int dragging_quantity = dragging->get_quantity();
					// Don't prompt if within same gump.
	if (dragging_quantity == 1 || (on_gump && on_gump == dragging_gump) ||
	      (dragging_quantity = Prompt_for_number(0, 
			dragging_quantity, 1, dragging_quantity)) > 0)
		{
		if (dragging_quantity < dragging->get_quantity())
			{		// Need to drop a copy.
			to_drop = new Ireg_game_object(to_drop->get_shapenum(),
					to_drop->get_framenum(), 0, 0, 0);
			to_drop->modify_quantity(dragging_quantity - 1);
			}
		if (on_gump)		// Dropping on a gump?
			{
			if (!(dropped = on_gump->add(to_drop, x, y,
					dragging_paintx, dragging_painty)))
				mouse->flash_shape(Mouse::wontfit);
			}
		else
			{		// Was it dropped on something?
			int max_lift = main_actor->get_lift() + 4;
			int lift;
			Game_object *found = find_object(x, y);
			if (found && found != dragging && found->drop(to_drop))
				dropped = 1;
					// Try to place on 'found'.
			else if (found && (lift = found->get_lift() +
				get_info(found).get_3d_height()) <= max_lift &&
				drop_at_lift(to_drop, lift))
				dropped = 1;
			else
				{	// Find where to drop it.
				Game_object *outer = dragging->get_outermost();
				for (lift = outer->get_lift(); 
					!dropped && lift <= max_lift; lift++)
					dropped = drop_at_lift(to_drop, lift);
				if (!dropped)
					mouse->flash_shape(Mouse::blocked);
				}
			}
		}
	if (dropped)			// Successful?
		{
		if (!dragging_gump)	// Do eggs where it came from.
			get_objects(oldcx, oldcy)->activate_eggs(dragging,
			    oldtx, oldty, dragging->get_lift(), oldtx, oldty);
		if (to_drop == dragging)// Whole thing?
			{		// Watch for stuff on top of it.
			if (old_foot.w > 0)
				Chunk_object_list::gravity(old_foot, old_top);
			return;		// All done.
			}
		else			// Subtract quantity moved.
			dragging->modify_quantity(-dragging_quantity);
		}
	if (dragging_gump)		// Put back remaining/orig. piece.
		dragging_gump->add(dragging, -2, -2, -2, -2);
	else
		get_objects(dragging->get_cx(), 
				dragging->get_cy())->add(dragging);
	}

/*
 *	Try to drop at a given lift.
 *
 *	Output:	1 if successful.
 */

int Game_window::drop_at_lift
	(
	Game_object *to_drop,
	int at_lift
	)
	{
					// Take lift into account, round.
	int x = dragging_paintx + at_lift*4 - 1; // + tilesize/2;
	int y = dragging_painty + at_lift*4 - 1; // + tilesize/2;
	int tx = scrolltx + x/tilesize;
	int ty = scrollty + y/tilesize;
	int cx = (scrolltx + x/tilesize)/tiles_per_chunk;
	int cy = (scrollty + y/tilesize)/tiles_per_chunk;
	Chunk_object_list *chunk = get_objects(cx, cy);
	int lift;			// Can we put it here?
	Shape_info& info = shapes.get_info(to_drop->get_shapenum());
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
					// Allow drop up to 5.
	if (!Chunk_object_list::is_blocked(info.get_3d_height(), at_lift,
		tx - xtiles + 1, ty - ytiles + 1, xtiles, ytiles, 
					lift, MOVE_FLY, 5) && (hack_mover ||
					// Check for path to location.
	    Fast_pathfinder_client::is_grabable(
		main_actor->get_abs_tile_coord(), Tile_coord(tx, ty, lift))))
		{
		to_drop->set_lift(lift);
		int rtx = tx%tiles_per_chunk, rty = ty%tiles_per_chunk;
		to_drop->set_shape_pos(rtx, rty);
cout << "Dropping object at (" << tx << ", " << ty << ", " << lift
							<< ")"<<endl;
		chunk->add(to_drop);
					// On an egg?
		chunk->activate_eggs(to_drop, tx, ty, lift, tx, ty);
		return (1);
		}
	return (0);
	}
