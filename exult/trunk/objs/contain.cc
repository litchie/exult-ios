/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Contain.h - Container objects.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include "contain.h"
#include "gamewin.h"
#include "objiter.h"
#include "game.h"
#include "ucmachine.h"
#include "files/utils.h"

using std::rand;
using std::ostream;

/*
 *	Delete all contents.
 */

Container_game_object::~Container_game_object
	(
	)
	{
	}

/*
 *	Remove an object.
 */

void Container_game_object::remove
	(
	Game_object *obj
	)
	{
	if (objects.is_empty())
		return;
	volume_used -= obj->get_volume();
	obj->set_owner(0);
	objects.remove(obj);
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Container_game_object::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume/recursion check.
	)
	{
	if (obj->get_shapenum() == get_shapenum())
		return (0);		// Can't put a bag in a bag.
	int objvol = obj->get_volume();
	int maxvol;			// Note:  NPC's have 0 volume.
	if (!dont_check)
		{
		if ((maxvol = get_max_volume()) > 0 &&
						objvol + volume_used > maxvol)
			return (0);	// Doesn't fit.
		Game_object *parent = this;
		do			// Watch for snake eating itself.
			if (obj == parent)
				return 0;
		while ((parent = parent->get_owner()) != 0);
		}
	volume_used += objvol;
	obj->set_owner(this);		// Set us as the owner.
	objects.append(obj);		// Append to chain.
					// Guessing:
	if (get_flag(Obj_flags::okay_to_take))
		obj->set_flag(Obj_flags::okay_to_take);
	return 1;
	}

/*
 *	Change shape of a member.
 */

void Container_game_object::change_member_shape
	(
	Game_object *obj,
	int newshape
	)
	{
	int oldvol = obj->get_volume();
	obj->set_shape(newshape);
					// Update total volume.
	volume_used += obj->get_volume() - oldvol;
	}

/*
 *	Recursively add a quantity of an item to those existing in
 *	this container, and create new objects if necessary.
 *
 *	Output:	Delta decremented # added.
 */

int Container_game_object::add_quantity
	(
	int delta,			// Quantity to add.
	int shapenum,			// Shape #.
	int qual,			// Quality, or c_any_qual for any.
	int framenum,			// Frame, or c_any_framenum for any.
	int dontcreate			// If 1, don't create new objs.
	)
	{
					// Get volume of 1 object.
	int objvol = Game_window::get_game_window()->get_info(
			shapenum).get_volume();
	int maxvol = get_max_volume();	// 0 means anything (NPC's?).
	int roomfor = maxvol && objvol ? (maxvol - volume_used)/objvol : 20000;
	int todo = delta < roomfor ? delta : roomfor;
	Game_object *obj;
	if (!objects.is_empty())
		{			// First try existing items.
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			{
			if (obj->get_shapenum() == shapenum &&
		    	 (framenum == c_any_framenum || obj->get_framenum() == framenum))
					// ++++++Quality???
				{
				int used = 
				    todo - obj->modify_quantity(todo);
				todo -= used;
				delta -= used;
				}
			}
		next.reset();			// Now try recursively.
		while ((obj = next.get_next()) != 0)
			delta = obj->add_quantity(
					delta, shapenum, qual, framenum, 1);
		}
	if (!delta || dontcreate)	// All added?
		return (delta);
	else
		return (create_quantity(delta, shapenum, qual,
				framenum == c_any_framenum ? 0 : framenum));
	}

/*
 *	Recursively create a quantity of an item.
 *
 *	Output:	Delta decremented # added.
 */

int Container_game_object::create_quantity
	(
	int delta,			// Quantity to add.
	int shapenum,			// Shape #.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame.
	)
	{
					// Get volume of 1 object.
	int objvol = Game_window::get_game_window()->get_info(
			shapenum).get_volume();
	int maxvol = get_max_volume();	// 0 means anything (NPC's?).
	int roomfor = maxvol && objvol ? (maxvol - volume_used)/objvol : 20000;
	int todo = delta < roomfor ? delta : roomfor;
	while (todo)			// Create them here first.
		{
		Game_object *newobj = new Ireg_game_object(shapenum, framenum,
								0, 0, 0);
		if (!add(newobj))
			{
			delete newobj;
			break;
			}
		if (qual != c_any_qual)	// Set desired quality.
			newobj->set_quality(qual);
		todo--; delta--;
		if (todo > 0)
			{
			int used = 
				todo - newobj->modify_quantity(todo);
			todo -= used;
			delta -= used;
			}
		}
	if (!delta)			// All done?
		return (0);
					// Now try those below.
	Game_object *obj;
	if (objects.is_empty())
		return (delta);
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		delta = obj->create_quantity(delta, shapenum, qual, framenum);
	return (delta);
	}		

/*
 *	Recursively remove a quantity of an item from those existing in
 *	this container.
 *
 *	Output:	Delta decremented by # removed.
 */

int Container_game_object::remove_quantity
	(
	int delta,			// Quantity to remove.
	int shapenum,			// Shape #.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame, or c_any_framenum for any.
	)
	{
	if (objects.is_empty())
		return delta;		// Empty.
	Game_object *obj = objects.get_first();
	Game_object *next;
	int done = 0;
	while (!done && delta)
		{
		next = obj->get_next();	// Might be deleting obj.
		if (obj->get_shapenum() == shapenum &&
		    (qual == c_any_qual || obj->get_quality() == qual) &&
		    (framenum == c_any_framenum || obj->get_framenum() == framenum))
			delta = -obj->modify_quantity(-delta);
					// Still there?
		if (next->get_prev() == obj)
					// Do it recursively.
			delta = obj->remove_quantity(delta, shapenum, 
							qual, framenum);
		obj = next;
		done = (!obj || obj == objects.get_first());
		}
	return (delta);
	}

/*
 *	Find and return a desired item.
 *
 *	Output:	->object if found, else 0.
 */

Game_object *Container_game_object::find_item
	(
	int shapenum,			// Shape #.
	int qual,			// Quality, or c_any_qual for any. ???+++++
	int framenum			// Frame, or c_any_framenum for any.
	)
	{
	if (objects.is_empty())
		return 0;		// Empty.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if (obj->get_shapenum() == shapenum &&
		    (framenum == c_any_framenum || obj->get_framenum() == framenum))
					// ++++++Quality???
			return (obj);

					// Do it recursively.
		Game_object *found = obj->find_item(shapenum, qual, framenum);
		if (found)
			return (found);
		}
	return (0);
	}

/*
 *	Run usecode when double-clicked.
 */

void Container_game_object::activate
	(
	Usecode_machine *umachine,
	int event
	)
	{
	int shnum = get_shapenum();
	Game_window *gwin = Game_window::get_game_window();

	if (Game::get_game_type() == BLACK_GATE)  switch(shnum)	// Watch for gumps.
	{
		case 405:			// Ship's hold
		gwin->show_gump(this, game->get_shape("gumps/shipshold"));
		return;

		case 406:			// Nightstand.
		case 407:			// Desk.
		case 283:
		case 203:
		case 416:			// Chest of drawers.
		case 679:
		gwin->show_gump(this, game->get_shape("gumps/drawer"));
		return;

		case 400:			// Bodies.
		case 414:
		case 762:
		case 778:
		case 892:
		case 507: 			// Bones
		gwin->show_gump(this, game->get_shape("gumps/body"));
		return;

		case 800:			// Chest.
		gwin->show_gump(this, game->get_shape("gumps/chest"));
		return;

		case 801:			// Backpack.
		gwin->show_gump(this, game->get_shape("gumps/backpack"));
		return;

		case 799:			// Unsealed box
		gwin->show_gump(this, game->get_shape("gumps/box"));
		return;

		case 802:			// Bag.
		gwin->show_gump(this, game->get_shape("gumps/bag"));
		return;

		case 803:			// Basket.
		gwin->show_gump(this, game->get_shape("gumps/basket"));
		return;
	
		case 804:			// Crate.
		gwin->show_gump(this, game->get_shape("gumps/crate"));
		return;

		case 819:			// Barrel.
		gwin->show_gump(this, game->get_shape("gumps/barrel"));
		return;
	}
	else if (Game::get_game_type() == SERPENT_ISLE) switch(shnum)	// Watch for gumps.
	{
		case 405:			// Ship's hold
		gwin->show_gump(this, game->get_shape("gumps/shipshold"));
		return;

		case 406:			// Nightstand.
		case 407:			// Desk.
		case 283:
		case 416:			// Chest of drawers.
		case 679:
		gwin->show_gump(this, game->get_shape("gumps/drawer"));
		return;

		case 400:			// Bodies.
		case 402:
		case 414:
		case 762:
		case 778:
		case 892:
		case 507: 			// Bones
		gwin->show_gump(this, game->get_shape("gumps/body"));
		return;

		case 800:			// Chest.
		if (get_quality() >= 251)	// Trapped?
			{		// Run normal usecode fun.
			umachine->call_usecode(shnum, this,
				(Usecode_machine::Usecode_events) event);
			return;
			}
						// FALL THROUGH to 486.
		case 486:			// Usecode container.
		gwin->show_gump(this, game->get_shape("gumps/chest"));
		return;

		case 801:			// Backpack.
		gwin->show_gump(this, game->get_shape("gumps/backpack"));
		return;

		case 799:			// Unsealed box
		gwin->show_gump(this, game->get_shape("gumps/box"));
		return;

		case 802:			// Bag.
		gwin->show_gump(this, game->get_shape("gumps/bag"));
		return;

		case 803:			// Basket.
		gwin->show_gump(this, game->get_shape("gumps/basket"));
		return;
	
		case 804:			// Crate.
		gwin->show_gump(this, game->get_shape("gumps/crate"));
		return;

		case 819:			// Barrel.
		gwin->show_gump(this, game->get_shape("gumps/barrel"));
		return;

		case 297:			// Hollow Tree
		gwin->show_gump(this, game->get_shape("gumps/tree"));
		return;

		case 555:			// Serpent Jawbone
		gwin->show_gump(this, game->get_shape("gumps/jawbone"));
		return;
	}

					// Try to run normal usecode fun.
	umachine->call_usecode(shnum, this,
				(Usecode_machine::Usecode_events) event);
	}

/*
 *	Get (total) weight.
 */

int Container_game_object::get_weight
	(
	)
	{
	int wt = Game_object::get_weight();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		wt += obj->get_weight();
	return wt;
	}

/*
 *	Drop another onto this.
 *
 *	Output:	0 to reject, 1 to accept.
 */

int Container_game_object::drop
	(
	Game_object *obj
	)
	{
	if (!get_owner())		// Only accept if inside another.
		return (0);
	return (add(obj));		// We'll take it.
	}

/*
 *	Recursively count all objects of a given shape.
 */

int Container_game_object::count_objects
	(
	int shapenum,			// Shape#, or c_any_shapenum for any.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame#, or c_any_framenum for any.
	)
	{
	int total = 0;
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if ((shapenum == c_any_shapenum || obj->get_shapenum() == shapenum) &&
		    (framenum == c_any_framenum || obj->get_framenum() == framenum) &&
		    (qual == c_any_qual || obj->get_quality() == qual))
			{		// Check quantity.
			int quant = obj->get_quantity();
			total += quant;
			}
					// Count recursively.
		total += obj->count_objects(shapenum, qual, framenum);
		}
	return (total);
	}

/*
 *	Recursively get all objects of a given shape.
 */

int Container_game_object::get_objects
	(
	Game_object_vector& vec,			// Objects returned here.
	int shapenum,			// Shape#, or c_any_shapenum for any.
	int qual,			// Quality, or c_any_qual for any.
	int framenum			// Frame#, or c_any_framenum for any.
	)
	{
	int vecsize = vec.size();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		if ((shapenum == c_any_shapenum || obj->get_shapenum() == shapenum) &&
		    (qual == c_any_qual || obj->get_quality() == qual) &&
		    (framenum == c_any_framenum || obj->get_framenum() == framenum))
			vec.push_back(obj);
					// Search recursively.
		obj->get_objects(vec, shapenum, qual, framenum);
		}
	return (vec.size() - vecsize);
	}

/*
 *	Being attacked.
 *
 *	Output:	0 if destroyed, else object itself.
 */

Game_object *Container_game_object::attacked
	(
	Actor *attacker,
	int weapon_shape,		// Weapon shape, or 0 to use readied.
	int ammo_shape
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int wpoints = attack_object(gwin, attacker, weapon_shape, ammo_shape);
	if (wpoints < 4)
		return this;		// Fail.
					// Guessing:
	if (rand()%90 >= wpoints)
		return this;		// Failed.
	Tile_coord pos = get_abs_tile_coord();
	gwin->add_dirty(this);
	remove_this(1);			// Remove, but don't delete yet.
	Game_object *obj;		// Remove objs. inside.
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		Shape_info& info = gwin->get_info(obj);
		Tile_coord p(-1, -1, -1);
		for (int i = 1; p.tx < 0 && i < 8; i++)
			p = Game_object::find_unblocked_tile(pos,
					i, info.get_3d_height());
		if (p.tx == -1)
			obj->remove_this();
		else
			obj->move(p.tx, p.ty, p.tz);
		}
	delete this;
	return 0;
	}

/*
 *	Set a flag on this and all contents.
 */

void Container_game_object::set_flag_recursively
	(
	int flag
	)
	{
	set_flag(flag);
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		obj->set_flag_recursively(flag);
	}

/*
 *	Write out container and its members.
 */

void Container_game_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	uint8 *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
	Game_object *first = objects.get_first(); // Guessing: +++++
	unsigned short tword = first ? first->get_prev()->get_shapenum() 
									: 0;
	Write2(ptr, tword);
	*ptr++ = 0;			// Unknown.
	*ptr++ = get_quality();
	int npc = get_live_npc_num();	// If body, get source.
	int quant = (npc >= 0 && npc <= 127) ? (npc + 0x80) : 0;
	*ptr++ = quant&0xff;		// "Quantity".
	*ptr++ = (get_lift()&15)<<4;
	*ptr++ = resistance;		// Resistance.
					// Flags:  B0=invis. B3=okay_to_take.
	*ptr++ = get_flag((Obj_flags::invisible) != 0) +
		 ((get_flag(Obj_flags::okay_to_take) != 0) << 3);
	out.write((char*)buf, sizeof(buf));
	write_contents(out);		// Write what's contained within.
	}

/*
 *	Write contents (if there is any).
 */

void Container_game_object::write_contents
	(
	ostream& out
	)
	{
	if (!objects.is_empty())	// Now write out what's inside.
		{
		Game_object *obj;
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			obj->write_ireg(out);
		out.put(0x01);		// A 01 terminates the list.
		}
	}

