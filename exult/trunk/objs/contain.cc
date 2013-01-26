/*
 *  contain.cc - Container objects.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
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
#include "ucsched.h"
#include "cheat.h"
#include "ready.h"
#include "weaponinf.h"
#include "frflags.h"

#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "objserial.h"
#include "servemsg.h"
#endif

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::endl;
using std::rand;
using std::ostream;
#endif

/*
 *  Determines if a shape can be added/found inside a container.
 */
static inline bool Can_be_added(
    Container_game_object *cont,
    int shapenum
) {
	Shape_info &info = cont->get_info();
	return !(cont->get_shapenum() == shapenum   // Shape can't be inside itself.
	         || info.is_container_locked()   // Locked container.
	         || !info.is_shape_accepted(shapenum));  // Shape can't be inside.
}

/*
 *  Delete all contents.
 */

Container_game_object::~Container_game_object(
) {
}

/*
 *  Remove an object.  The object's (cx, cy) fields are set to invalid
 *  #'s (255, 255).
 */

void Container_game_object::remove(
    Game_object *obj
) {
	if (objects.is_empty())
		return;
	volume_used -= obj->get_volume();
	obj->set_owner(0);
	objects.remove(obj);
	obj->set_invalid();     // No longer part of world.
}

/*
 *  Add an object.
 *
 *  Output: 1, meaning object is completely contained in this.  Obj may
 *          be deleted in this case if combine==true.
 *      0 if not enough space, although obj's quantity may be
 *          reduced if combine==true.
 */

bool Container_game_object::add(
    Game_object *obj,
    bool dont_check,        // True to skip volume/recursion check.
    bool combine,           // True to try to combine obj.  MAY
    //   cause obj to be deleted.
    bool noset      // True to prevent actors from setting sched. weapon.
) {
	// Prevent dragging the avatar into a container.
	// Casting to void* to avoid including actors.h.
	if (obj == static_cast<void *>(gwin->get_main_actor()))
		return false;
	Shape_info &info = get_info();
	if (get_shapenum() == obj->get_shapenum()   // Shape can't be inside itself.
	        || (!dont_check && info.is_container_locked())) // Locked container.
		return false;
	if (!info.is_shape_accepted(obj->get_shapenum()))   // Shape can't be inside.
		return false;

	// Always check this. ALWAYS!
	Game_object *parent = this;
	do              // Watch for snake eating itself.
		if (obj == parent)
			return false;
	while ((parent = parent->get_owner()) != 0);

	if (combine) {          // Should we try to combine?
		Shape_info &info = obj->get_info();
		int quant = obj->get_quantity();
		// Combine, but don't add.
		int newquant = add_quantity(quant, obj->get_shapenum(),
		                            info.has_quality() ? obj->get_quality() : c_any_qual,
		                            obj->get_framenum(), true);
		if (newquant == 0) { // All added?
			obj->remove_this();
			return true;
		} else if (newquant < quant) // Partly successful.
			obj->modify_quantity(newquant - quant);
	}
	int objvol = obj->get_volume();
	if (!cheat.in_hack_mover() && !dont_check) {
		int maxvol = get_max_volume();
		// maxvol = 0 means infinite (ship's hold, hollow tree, etc...)
		if (maxvol > 0 && objvol + volume_used > maxvol)
			return false;   // Doesn't fit.
	}
	volume_used += objvol;
	obj->set_owner(this);       // Set us as the owner.
	objects.append(obj);        // Append to chain.
	// Guessing:
	if (get_flag(Obj_flags::okay_to_take))
		obj->set_flag(Obj_flags::okay_to_take);
	return true;
}

/*
 *  Change shape of a member.
 */

void Container_game_object::change_member_shape(
    Game_object *obj,
    int newshape
) {
	int oldvol = obj->get_volume();
	obj->set_shape(newshape);
	// Update total volume.
	volume_used += obj->get_volume() - oldvol;
}

/*
 *  Add a key (by quality) to the SI keyring.
 *
 *  Output: 1 if successful, else 0 (and this does need to be an int).
 */

static int Add2keyring(
    int qual,
    int framenum
) {
	if (framenum >= 21 && framenum <= 23)
		return 0;       // Fire, ice, blackrock in SI.
	Keyring *ring =
	    Game_window::get_instance()->get_usecode()->getKeyring();
	// Valid quality & not already there?
	if (qual != c_any_qual && !ring->checkkey(qual)) {
		ring->addkey(qual);
		return 1;
	}
	return 0;
}

/*
 *  Recursively add a quantity of an item to those existing in
 *  this container, and create new objects if necessary.
 *
 *  Output: Delta decremented # added.
 */

int Container_game_object::add_quantity(
    int delta,          // Quantity to add.
    int shapenum,           // Shape #.
    int qual,           // Quality, or c_any_qual for any.
    int framenum,           // Frame, or c_any_framenum for any.
    bool dontcreate,        // If true, don't create new objs.
    bool temporary      // If objects should be temporary
) {
	if (delta <= 0 || !Can_be_added(this, shapenum))
		return delta;

	int cant_add = 0;       // # we can't add due to weight.
	int maxweight = get_max_weight();// Check weight.
	if (maxweight) {
		maxweight *= 10;    // Work in .1 stones.
		int avail = maxweight - get_outermost()->get_weight();
		int objweight = Ireg_game_object::get_weight(shapenum, delta);
		if (objweight && objweight > avail) {
			// Limit what we can add.
			// Work in 1/100ths.
			int weight1 = (10 * objweight) / delta;
			cant_add = delta - (10 * avail) / (weight1 ? weight1 : 1);
			if (cant_add >= delta)
				return delta;   // Can't add any.
			delta -= cant_add;
		}
	}
	Shape_info &info = ShapeID::get_info(shapenum);
	bool has_quantity = info.has_quantity();    // Quantity-type shape?
	bool has_quantity_frame = has_quantity ? info.has_quantity_frames() : false;
	// Note:  quantity is ignored for
	//   figuring volume.
	Game_object *obj;
	if (!objects.is_empty()) {
		// First try existing items.
		Object_iterator next(objects);
		while (delta && (obj = next.get_next()) != 0) {
			if (has_quantity && obj->get_shapenum() == shapenum &&
			        (framenum == c_any_framenum || has_quantity_frame ||
			         obj->get_framenum() == framenum))
				delta = obj->modify_quantity(delta);
			// Adding key to SI keyring?
			else if (GAME_SI && shapenum == 641 &&
			         obj->get_shapenum() == 485 && delta == 1)
				delta -= Add2keyring(qual, framenum);
		}
		next.reset();           // Now try recursively.
		while ((obj = next.get_next()) != 0)
			delta = obj->add_quantity(
			            delta, shapenum, qual, framenum, true, temporary);
	}
	if (!delta || dontcreate)   // All added?
		return (delta + cant_add);
	else
		return cant_add + create_quantity(delta, shapenum, qual,
		                                  framenum == c_any_framenum ? 0 : framenum, temporary);
}

/*
 *  Recursively create a quantity of an item.  Assumes weight check has
 *  already been done.
 *
 *  Output: Delta decremented # added.
 */

int Container_game_object::create_quantity(
    int delta,          // Quantity to add.
    int shnum,          // Shape #.
    int qual,           // Quality, or c_any_qual for any.
    int frnum,          // Frame.
    bool temporary          // Create temporary quantity
) {
	if (!Can_be_added(this, shnum))
		return delta;
	// Usecode container?
	Shape_info &info = ShapeID::get_info(get_shapenum());
	if (info.get_ready_type() == ucont)
		return delta;
	Shape_info &shp_info = ShapeID::get_info(shnum);
	if (!shp_info.has_quality())    // Not a quality object?
		qual = c_any_qual;  // Then don't set it.
	while (delta) {         // Create them here first.
		Game_object *newobj = gmap->create_ireg_object(
		                          shp_info, shnum, frnum, 0, 0, 0);
		if (!add(newobj)) {
			delete newobj;
			break;
		}

		// Set temporary
		if (temporary) newobj->set_flag(Obj_flags::is_temporary);

		if (qual != c_any_qual) // Set desired quality.
			newobj->set_quality(qual);
		delta--;
		if (delta > 0)
			delta =  newobj->modify_quantity(delta);
	}
	if (!delta)         // All done?
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
 *  Recursively remove a quantity of an item from those existing in
 *  this container.
 *
 *  Output: Delta decremented by # removed.
 */

int Container_game_object::remove_quantity(
    int delta,          // Quantity to remove.
    int shapenum,           // Shape #.
    int qual,           // Quality, or c_any_qual for any.
    int framenum            // Frame, or c_any_framenum for any.
) {
	if (objects.is_empty() || !Can_be_added(this, shapenum))
		return delta;       // Empty.
	Game_object *obj = objects.get_first();
	Game_object *last = obj->get_prev();    // Save last.
	Game_object *next;
	while (obj && delta) {
		// Might be deleting obj.
		next = obj == last ? 0 : obj->get_next();
		bool del = false;   // Gets 'deleted' flag.
		if (obj->get_shapenum() == shapenum &&
		        (qual == c_any_qual || obj->get_quality() == qual) &&
		        (framenum == c_any_framenum ||
		         (obj->get_framenum() & 31) == framenum))
			delta = -obj->modify_quantity(-delta, &del);

		if (!del)       // Still there?
			// Do it recursively.
			delta = obj->remove_quantity(delta, shapenum,
			                             qual, framenum);
		obj = next;
	}
	return (delta);
}

/*
 *  Find and return a desired item.
 *
 *  Output: ->object if found, else 0.
 */

Game_object *Container_game_object::find_item(
    int shapenum,           // Shape #.
    int qual,           // Quality, or c_any_qual for any.
    int framenum            // Frame, or c_any_framenum for any.
) {
	if (objects.is_empty() || !Can_be_added(this, shapenum))
		return 0;       // Empty.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0) {
		if (obj->get_shapenum() == shapenum &&
		        (framenum == c_any_framenum ||
		         (obj->get_framenum() & 31) == framenum) &&
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
 *  Displays the object's gump.
 *  Returns true if the gump has been handled.
 */

bool Container_game_object::show_gump(
    int event
) {
	Shape_info &inf = get_info();
	int gump;
	if (cheat.in_map_editor())
		return true;    // Do nothing.
	else if (inf.has_object_flag(get_framenum(),
	                             inf.has_quality() ? get_quality() : -1, Frame_flags::force_usecode))
		// Run normal usecode fun.
		return false;
	else if ((gump = inf.get_gump_shape()) >= 0) {
		Gump_manager *gump_man = gumpman;
		gump_man->add_gump(this, gump);
		return true;
	}
	return false;
}

/*
 *  Run usecode when double-clicked.
 */

void Container_game_object::activate(
    int event
) {
	if (edit())
		return;         // Map-editing.
	if (!show_gump(event))
		// Try to run normal usecode fun.
		ucmachine->call_usecode(get_usecode(), this,
		                        static_cast<Usecode_machine::Usecode_events>(event));
}

/*
 *  Edit in ExultStudio.
 */

bool Container_game_object::edit(
) {
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&   // Talking to ExultStudio?
	        cheat.in_map_editor()) {
		editing = 0;
		Tile_coord t = get_tile();
		unsigned long addr = reinterpret_cast<unsigned long>(this);
		std::string name = get_name();
		if (Container_out(client_socket, addr, t.tx, t.ty, t.tz,
		                  get_shapenum(), get_framenum(), get_quality(), name,
		                  get_obj_hp(),
		                  get_flag(Obj_flags::invisible) != 0,
		                  get_flag(Obj_flags::okay_to_take) != 0) != -1) {
			cout << "Sent object data to ExultStudio" << endl;
			editing = this;
		} else
			cout << "Error sending object to ExultStudio" << endl;
		return true;
	}
#endif
	return false;
}

/*
 *  Message to update from ExultStudio.
 */

void Container_game_object::update_from_studio(
    unsigned char *data,
    int datalen
) {
#ifdef USE_EXULTSTUDIO
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame, quality;
	unsigned char res;
	bool invis, can_take;
	std::string name;
	if (!Container_in(data, datalen, addr, tx, ty, tz, shape, frame,
	                  quality, name, res, invis, can_take)) {
		cout << "Error decoding object" << endl;
		return;
	}
	Container_game_object *obj = reinterpret_cast<Container_game_object *>(addr);
	if (!editing || obj != editing) {
		cout << "Obj from ExultStudio is not being edited" << endl;
		return;
	}
//	editing = 0; // He may have chosen 'Apply', so still editing.
	if (invis)
		obj->set_flag(Obj_flags::invisible);
	else
		obj->clear_flag(Obj_flags::invisible);
	if (can_take)
		obj->set_flag(Obj_flags::okay_to_take);
	else
		obj->clear_flag(Obj_flags::okay_to_take);
	gwin->add_dirty(obj);
	obj->set_shape(shape, frame);
	gwin->add_dirty(obj);
	obj->set_quality(quality);
	obj->set_obj_hp(res);
	Container_game_object *owner = obj->get_owner();
	if (!owner) {
		// See if it moved -- but only if not inside something!
		Tile_coord oldt = obj->get_tile();
		if (oldt.tx != tx || oldt.ty != ty || oldt.tz != tz)
			obj->move(tx, ty, tz);
	}
	cout << "Object updated" << endl;
#endif
}

/*
 *  Get (total) weight.
 */

int Container_game_object::get_weight(
) {
	int wt = Ireg_game_object::get_weight();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		wt += obj->get_weight();
	return wt;
}

/*
 *  Drop another onto this.
 *
 *  Output: 0 to reject, 1 to accept.
 */

int Container_game_object::drop(
    Game_object *obj        // May be deleted if combined.
) {
	if (!get_owner())       // Only accept if inside another.
		return (0);
	return (add(obj, false, true)); // We'll take it, and try to combine.
}

/*
 *  Recursively count all objects of a given shape.
 */

int Container_game_object::count_objects(
    int shapenum,           // Shape#, or c_any_shapenum for any.
    int qual,           // Quality, or c_any_qual for any.
    int framenum            // Frame#, or c_any_framenum for any.
) {
	if (!Can_be_added(this, shapenum))
		return 0;
	int total = 0;
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0) {
		if ((shapenum == c_any_shapenum || obj->get_shapenum() == shapenum) &&
		        // Watch for reflection.
		        (framenum == c_any_framenum || (obj->get_framenum() & 31) == framenum) &&
		        (qual == c_any_qual || obj->get_quality() == qual)) {
			// Check quantity.
			int quant = obj->get_quantity();
			total += quant;
		}
		// Count recursively.
		total += obj->count_objects(shapenum, qual, framenum);
	}
	return (total);
}

/*
 *  Recursively get all objects of a given shape.
 */

int Container_game_object::get_objects(
    Game_object_vector &vec,    // Objects returned here.
    int shapenum,           // Shape#, or c_any_shapenum for any.
    int qual,           // Quality, or c_any_qual for any.
    int framenum            // Frame#, or c_any_framenum for any.
) {
	int vecsize = vec.size();
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0) {
		if ((shapenum == c_any_shapenum || obj->get_shapenum() == shapenum) &&
		        (qual == c_any_qual || obj->get_quality() == qual) &&
		        // Watch for reflection.
		        (framenum == c_any_framenum || (obj->get_framenum() & 31) == framenum))
			vec.push_back(obj);
		// Search recursively.
		obj->get_objects(vec, shapenum, qual, framenum);
	}
	return (vec.size() - vecsize);
}


/*
 *  Set a flag on this and all contents.
 */

void Container_game_object::set_flag_recursively(
    int flag
) {
	set_flag(flag);
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		obj->set_flag_recursively(flag);
}

/*
 *  Write out container and its members.
 */

void Container_game_object::write_ireg(
    DataSource *out
) {
	unsigned char buf[20];      // 12-byte entry.
	uint8 *ptr = write_common_ireg(12, buf);
	Game_object *first = objects.get_first(); // Guessing: +++++
	unsigned short tword = first ? first->get_prev()->get_shapenum()
	                       : 0;
	Write2(ptr, tword);
	*ptr++ = 0;         // Unknown.
	*ptr++ = get_quality();
	*ptr++ = 0;     // "Quantity".
	*ptr++ = (get_lift() & 15) << 4; // Lift
	*ptr++ = static_cast<unsigned char>(resistance);        // Resistance.
	// Flags:  B0=invis. B3=okay_to_take.
	*ptr++ = (get_flag(Obj_flags::invisible) != 0) +
	         ((get_flag(Obj_flags::okay_to_take) != 0) << 3);
	out->write(reinterpret_cast<char *>(buf), ptr - buf);
	write_contents(out);        // Write what's contained within.
	// Write scheduled usecode.
	Game_map::write_scheduled(out, this);
}

// Get size of IREG. Returns -1 if can't write to buffer
int Container_game_object::get_ireg_size() {
	// These shouldn't ever happen, but you never know
	if (gumpman->find_gump(this) || Usecode_script::find(this))
		return -1;

	int total_size = 8 + get_common_ireg_size();

	// Now what's inside.
	if (!objects.is_empty()) {
		Game_object *obj;
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0) {
			int size = obj->get_ireg_size();

			if (size < 0) return -1;

			total_size += size;
		}
		total_size += 1;
	}

	return total_size;
}

/*
 *  Write contents (if there is any).
 */

void Container_game_object::write_contents(
    DataSource *out
) {
	if (!objects.is_empty()) {  // Now write out what's inside.
		Game_object *obj;
		Object_iterator next(objects);
		while ((obj = next.get_next()) != 0)
			obj->write_ireg(out);
		out->write1(0x01);      // A 01 terminates the list.
	}
}


bool Container_game_object::extract_contents(Container_game_object *targ) {
	if (objects.is_empty())
		return true;

	bool status = true;

	Game_object *obj;

	while ((obj = objects.get_first())) {
		remove(obj);

		if (targ) {
			targ->add(obj, 1); // add without checking volume
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

void Container_game_object::delete_contents() {
	if (objects.is_empty())
		return;

	Game_object *obj;
	while ((obj = objects.get_first())) {
		remove(obj);

		obj->delete_contents(); // recurse into contained containers
		obj->remove_this(0);
	}
}

void Container_game_object::remove_this(int nodel) {
	// Needs to be saved, as it is invalidated below but needed
	// shortly after.
	Container_game_object *safe_owner = Container_game_object::get_owner();
	// Special case to avoid recursion.
	if (safe_owner) {
		// First remove from owner.
		Ireg_game_object::remove_this(1);
		if (nodel)      // Not deleting?  Then done.
			return;
	}
	if (!nodel)
		extract_contents(safe_owner);

	Ireg_game_object::remove_this(nodel);
}

/*
 *  Find ammo used by weapon.
 *
 *  Output: ->object if found. Additionally, is_readied is set to
 *  true if the ammo is readied.
 */

Game_object *Container_game_object::find_weapon_ammo(
    int weapon,         // Weapon shape.
    int needed,
    bool recursive
) {
	if (weapon < 0 || !Can_be_added(this, weapon))
		return 0;
	Weapon_info *winf = ShapeID::get_info(weapon).get_weapon_info();
	if (!winf)
		return 0;
	int family = winf->get_ammo_consumed();
	if (family >= 0)
		return 0;

	Game_object_vector vec;     // Get list of all possessions.
	vec.reserve(50);
	get_objects(vec, c_any_shapenum, c_any_qual, c_any_framenum);
	for (Game_object_vector::const_iterator it = vec.begin();
	        it != vec.end(); ++it) {
		Game_object *obj = *it;
		if (obj->get_shapenum() != weapon)
			continue;
		Shape_info &inf = obj->get_info();
		if (family == -2) {
			if (!inf.has_quality() || obj->get_quality() >= needed)
				return obj;
		}
		// Family -1 and family -3.
		else if (obj->get_quantity() >= needed)
			return obj;
	}
	return 0;
}
