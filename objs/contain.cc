/*
 *	contain.cc - Container objects.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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

#include "contain.h"
#include "gamewin.h"
#include "gamemap.h"
#include "objiter.h"
#include "game.h"
#include "ucmachine.h"
#include "keyring.h"
#include "utils.h"
#include "Gump_manager.h"
#include "databuf.h"

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
 *	Output:	1, meaning object is completely contained in this.  Obj may
 *			be deleted in this case if combine==true.
 *		0 if not enough space, although obj's quantity may be
 *			reduced if combine==true.
 */

bool Container_game_object::add
	(
	Game_object *obj,
	bool dont_check,		// 1 to skip volume/recursion check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
	{
	if (obj->get_shapenum() == get_shapenum() && !dont_check)
		return false;		// Can't put a bag in a bag.

	// ugly hack for SI urn (shouldn't be a container)
	if (Game::get_game_type() == SERPENT_ISLE && get_shapenum() == 914) {
		return false;
	}

	// Always check this. ALWAYS!
	Game_object *parent = this;
	do				// Watch for snake eating itself.
		if (obj == parent)
			return false;
	while ((parent = parent->get_owner()) != 0);

	if (combine)			// Should we try to combine?
		{
		Game_window *gwin = Game_window::get_game_window();
		Shape_info& info = gwin->get_info(obj->get_shapenum());
		int quant = obj->get_quantity();
					// Combine, but don't add.
		int newquant = add_quantity(quant, obj->get_shapenum(),
			info.has_quality() ? obj->get_quality() : c_any_qual,
						obj->get_framenum(), true);
		if (newquant == 0)	// All added?
			{
			obj->remove_this();
			return true;
			}
		else if (newquant < quant)	// Partly successful.
			obj->modify_quantity(newquant - quant);
		}
	int objvol = obj->get_volume();
	if (!dont_check)
		{
		int maxvol = get_max_volume();
		// maxvol = 0 means infinite (ship's hold, hollow tree, etc...)
		if (maxvol > 0 && objvol + volume_used > maxvol)
			return false;	// Doesn't fit.
		}
	volume_used += objvol;
	obj->set_owner(this);		// Set us as the owner.
	objects.append(obj);		// Append to chain.
					// Guessing:
	if (get_flag(Obj_flags::okay_to_take))
		obj->set_flag(Obj_flags::okay_to_take);
	return true;
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
 *	Add a key (by quality) to the SI keyring.
 *
 *	Output:	1 if successful, else 0 (and this does need to be an int).
 */

static int Add2keyring
	(
	int qual
	)
	{
	Keyring *ring = 
		Game_window::get_game_window()->get_usecode()->getKeyring();
					// Valid quality & not already there?
	if (qual != c_any_qual && !ring->checkkey(qual))
		{
		ring->addkey(qual);
		return 1;
		}
	return 0;
	}

/*
 *	Get information about whether a shape is a 'quantity' shape, and
 *	whether we need to match its framenum to combine it with another.
 *
 *	Output:	True if a 'quantity' shape.
 */

static bool Get_combine_info
	(
	int shapenum,
	int framenum,
	bool& quantity_frame		// Rets. true if frame depends on quan.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_info(shapenum);
	quantity_frame = false;
	if (!info.has_quantity())
		return false;
	switch (shapenum)		// Which shapes have frames depending
		{			//   on their quantity?
	case 644:			// Coins.
	case 627:			// Lockpicks.
	case 581:			// Ammunition.
	case 554:			// Burst arrows.
	case 556:			// Magic arrows.
	case 558:			// Lucky arrows.
	case 560:			// Love arrows.
	case 568:			// Tseramed arrows.
	case 722:			// Arrows.
	case 723:			// Bolts.
	case 417:			// Magic bolts.
	case 948:			// Bolts.  (or SI filari).
		quantity_frame = true;
		break;
	case 951:			// Monetari/guilders:
	case 952:
		if (GAME_SI)
			quantity_frame = true;
		break;
	default:
		break;
		}
	return true;
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
	if (delta <= 0)
		return delta;

	int cant_add = 0;		// # we can't add due to weight.
	int maxweight = get_max_weight();// Check weight.
	if (maxweight)
		{	
		maxweight *= 10;	// Work in .1 stones.
		int avail = maxweight - get_outermost()->get_weight();
		int objweight = Ireg_game_object::get_weight(shapenum, delta);
		if (objweight && objweight > avail)
			{		// Limit what we can add.
					// Work in 1/100ths.
			int weight1 = (10*objweight)/delta;
			cant_add = delta - (10*avail)/(weight1 ? weight1 : 1);
			if (cant_add >= delta)
				return delta;	// Can't add any.
			delta -= cant_add;
			}
		}
	bool has_quantity_frame;	// Quantity-type shape?
	bool has_quantity = Get_combine_info(
				shapenum, framenum, has_quantity_frame);
					// Note:  quantity is ignored for
					//   figuring volume.
	Game_object *obj;
	if (!objects.is_empty())
		{			// First try existing items.
		Object_iterator next(objects);
		while (delta && (obj = next.get_next()) != 0)
			{
			if (has_quantity && obj->get_shapenum() == shapenum &&
		    	 (framenum == c_any_framenum || has_quantity_frame ||
					obj->get_framenum() == framenum))

				delta = obj->modify_quantity(delta);
					// Adding key to SI keyring?
			else if (GAME_SI && shapenum == 641 &&
				 obj->get_shapenum() == 485 && delta == 1)
				delta -= Add2keyring(qual);
			}
		next.reset();			// Now try recursively.
		while ((obj = next.get_next()) != 0)
			delta = obj->add_quantity(
					delta, shapenum, qual, framenum, 1);
		}
	if (!delta || dontcreate)	// All added?
		return (delta + cant_add);
	else
		return cant_add + create_quantity(delta, shapenum, qual,
				framenum == c_any_framenum ? 0 : framenum);
}

/*
 *	Recursively create a quantity of an item.  Assumes weight check has
 *	already been done.
 *
 *	Output:	Delta decremented # added.
 */

int Container_game_object::create_quantity
	(
	int delta,			// Quantity to add.
	int shnum,			// Shape #.
	int qual,			// Quality, or c_any_qual for any.
	int frnum,			// Frame.
	bool temporary			// Create temporary quantity
	)
	{
					// Usecode container?
	if (get_shapenum() == 486 && Game::get_game_type() == SERPENT_ISLE)
		return delta;
	Shape_info& shp_info=Game_window::get_game_window()->get_info(
								shnum);
	if (!shp_info.has_quality())	// Not a quality object?
		qual = c_any_qual;	// Then don't set it.
	while (delta)			// Create them here first.
		{
		Game_object *newobj = Game_window::get_game_window()->
			create_ireg_object(shp_info, shnum, frnum,0,0,0);

		if (!add(newobj))
			{
			delete newobj;
			break;
			}

		// Set temporary
		if (temporary) newobj->set_flag (Obj_flags::is_temporary);

		if (qual != c_any_qual)	// Set desired quality.
			newobj->set_quality(qual);
		delta--;
		if (delta > 0)
			delta =  newobj->modify_quantity(delta);
		}
	if (!delta)			// All done?
		return (0);
					// Now try those below.
	Game_object *obj;
	if (objects.is_empty())
		return (delta);
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		delta = obj->create_quantity(delta, shnum, qual, frnum);
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
	Game_object *last = obj->get_prev();	// Save last.
	Game_object *next;
	while (obj && delta)
		{
					// Might be deleting obj.
		next = obj == last ? 0 : obj->get_next();
		bool del = false;	// Gets 'deleted' flag.
		if (obj->get_shapenum() == shapenum &&
		    (qual == c_any_qual || obj->get_quality() == qual) &&
		    (framenum == c_any_framenum || 
					obj->get_framenum() == framenum))
			delta = -obj->modify_quantity(-delta, &del);

		if (!del)		// Still there?
					// Do it recursively.
			delta = obj->remove_quantity(delta, shapenum, 
							qual, framenum);
		obj = next;
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
	int qual,			// Quality, or c_any_qual for any.
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
		    (framenum == c_any_framenum || 
					obj->get_framenum() == framenum) &&
		    (qual == c_any_qual || obj->get_quality() == qual))
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
	if (edit())
		return;			// Map-editing.
	int shnum = get_shapenum();
	Gump_manager *gump_man = Game_window::get_game_window()->get_gump_man();

	if (Game::get_game_type() == BLACK_GATE)  switch(shnum)	// Watch for gumps.
	{
		case 405:			// Ship's hold
		gump_man->add_gump(this, game->get_shape("gumps/shipshold"));
		return;

		case 406:			// Nightstand.
		case 407:			// Desk.
		case 283:
		case 203:
		case 416:			// Chest of drawers.
		case 679:
		gump_man->add_gump(this, game->get_shape("gumps/drawer"));
		return;

		case 400:			// Bodies.
		case 414:
		case 762:
		case 778:
		case 892:
		case 507: 			// Bones
		gump_man->add_gump(this, game->get_shape("gumps/body"));
		return;

		case 800:			// Chest.
		gump_man->add_gump(this, game->get_shape("gumps/chest"));
		return;

		case 801:			// Backpack.
		gump_man->add_gump(this, game->get_shape("gumps/backpack"));
		return;

		case 799:			// Unsealed box
		gump_man->add_gump(this, game->get_shape("gumps/box"));
		return;

		case 802:			// Bag.
		gump_man->add_gump(this, game->get_shape("gumps/bag"));
		return;

		case 803:			// Basket.
		gump_man->add_gump(this, game->get_shape("gumps/basket"));
		return;
	
		case 804:			// Crate.
		gump_man->add_gump(this, game->get_shape("gumps/crate"));
		return;

		case 819:			// Barrel.
		gump_man->add_gump(this, game->get_shape("gumps/barrel"));
		return;
	}
	else if (Game::get_game_type() == SERPENT_ISLE) switch(shnum)	// Watch for gumps.
	{
		case 405:			// Ship's hold
		gump_man->add_gump(this, game->get_shape("gumps/shipshold"));
		return;

		case 406:			// Nightstand.
		case 407:			// Desk.
		case 283:
		case 416:			// Chest of drawers.
		case 679:
		gump_man->add_gump(this, game->get_shape("gumps/drawer"));
		return;

		case 400:			// Bodies.
		case 402:
		case 414:
		case 762:
		case 778:
		case 892:
		case 507: 			// Bones
		gump_man->add_gump(this, game->get_shape("gumps/body"));
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
		gump_man->add_gump(this, game->get_shape("gumps/chest"));
		return;

		case 801:			// Backpack.
		gump_man->add_gump(this, game->get_shape("gumps/backpack"));
		return;

		case 799:			// Unsealed box
		gump_man->add_gump(this, game->get_shape("gumps/box"));
		return;

		case 802:			// Bag.
		gump_man->add_gump(this, game->get_shape("gumps/bag"));
		return;

		case 803:			// Basket.
		gump_man->add_gump(this, game->get_shape("gumps/basket"));
		return;
	
		case 804:			// Crate.
		gump_man->add_gump(this, game->get_shape("gumps/crate"));
		return;

		case 819:			// Barrel.
		gump_man->add_gump(this, game->get_shape("gumps/barrel"));
		return;

		case 297:			// Hollow Tree
		gump_man->add_gump(this, game->get_shape("gumps/tree"));
		return;

		case 555:			// Serpent Jawbone
		gump_man->add_gump(this, game->get_shape("gumps/jawbone"));
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
	int wt = Ireg_game_object::get_weight();
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
	Game_object *obj		// May be deleted if combined.
	)
	{
	if (!get_owner())		// Only accept if inside another.
		return (0);
	return (add(obj, false, true));	// We'll take it, and try to combine.
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
	Game_object_vector& vec,	// Objects returned here.
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
	DataSource *out
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
	int quant;
	if (Game::get_game_type() == SERPENT_ISLE)
		quant = npc + 0x80;
	else
		quant = (npc >= 0 && npc <= 127) ? (npc + 0x80) : 0;

	*ptr++ = quant&0xff;		// "Quantity".
	*ptr++ = (get_lift()&15)<<4;	// Lift 
	*ptr++ = (unsigned char)resistance;		// Resistance.
					// Flags:  B0=invis. B3=okay_to_take.
	*ptr++ = get_flag((Obj_flags::invisible) != 0) +
		 ((get_flag(Obj_flags::okay_to_take) != 0) << 3);
	out->write((char*)buf, sizeof(buf));
	write_contents(out);		// Write what's contained within.
					// Write scheduled usecode.
	Game_map::write_scheduled(out, this);	
	}

/*
 *	Write contents (if there is any).
 */

void Container_game_object::write_contents
	(
	DataSource *out
	)
	{
	if (!objects.is_empty())	// Now write out what's inside.
		{
		Game_object *obj;
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			obj->write_ireg(out);
		out->write1(0x01);		// A 01 terminates the list.
		}
	}


bool Container_game_object::extract_contents()
{
	if (objects.is_empty())
		return true;

	bool status = true;

	Container_game_object *owner = get_owner();
	Game_object *obj;

	while ((obj = objects.get_first())) {
		remove(obj);

		if (owner) {
			owner->add(obj,1); // add without checking volume
		} else {
			obj->set_invalid(); // set to invalid chunk so move() doesn't fail
			if ((get_cx() == 255) && (get_cy() == 255)) {
				obj->remove_this(0);
				status = false;
			} else {
				obj->move(get_tile());
			}
		}
	}

	return status;
}

void Container_game_object::delete_contents()
{
	if (objects.is_empty())
		return;

	Game_object *obj;
	while ((obj = objects.get_first())) {
		remove(obj);

		obj->delete_contents(); // recurse into contained containers
		obj->remove_this(0);
	}
}

void Container_game_object::remove_this(int nodel)
{
					// Special case to avoid recursion.
	if (Container_game_object::get_owner())
		{			// First remove from owner.
		Ireg_game_object::remove_this(1);
		if (nodel)		// Not deleting?  Then done.
			return;
		}
	if (!nodel)
		extract_contents();

	Ireg_game_object::remove_this(nodel);
}
