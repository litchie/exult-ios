/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	ucmachine.cc - Interpreter for usecode.
 **
 **	Written: 8/12/99 - JSF
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

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>			/* Debugging.			*/
#  include <fstream>
#  include <cstring>
#  include <cstdlib>
#endif

#include <iomanip>

#ifdef XWIN
#include <signal.h>
#endif
#if USECODE_DEBUGGER
#include <algorithm>       // STL function things
#endif

#include "Gump.h"
#include "Text_gump.h"
#include "animate.h"
#include "barge.h"
#include "chunks.h"
#include "conversation.h"
#include "game.h"
#include "gamewin.h"
#include "mouse.h"
#include "schedule.h"
#include "tqueue.h"
#include "ucmachine.h"
#include "ucsched.h"
#include "useval.h"
#include "utils.h"
#include "vec.h"

using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::exit;
using std::ios;
using std::dec;
using std::hex;
using std::memset;
using std::setfill;
using std::setw;
using std::size_t;
using std::snprintf;
using std::string;
using std::strcat;
using std::strchr;
using std::strcmp;
using std::strcpy;
using std::strlen;
using std::vector;


// External globals..

extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *key = 0);
extern	bool intrinsic_trace,usecode_trace,usecode_debugging;
extern void Wait_for_arrival(Actor *actor, Tile_coord dest);

#if USECODE_DEBUGGER
std::vector<int> intrinsic_breakpoints;

void	initialise_usecode_debugger(void)
{
	// Summon up the configuration file

	// List all the keys.

	// Render intrinsic names to numbers (unless already given as
	// a number (which might be hex. Convert from that.

	// push them all onto the list


}

#endif


/*
 *	Read in a function.
 */

Usecode_function::Usecode_function
	(
	istream& file
	)
	{
	id = Read2(file);
	len = Read2(file);
	code = new unsigned char[len];	// Allocate buffer & read it in.
	file.read((char*)code, len);
	}

/*
 *	Append a string.
 */

void Usecode_machine::append_string
	(
	const char *str
	)
	{
	if (!str)
		return;
					// Figure new length.
	int len = String ? strlen(String) : 0;
	len += strlen(str);
	char *newstr = new char[len + 1];
	if (String)
		{
		strcpy(newstr, String);
		delete [] String;
		String = strcat(newstr, str);
		}
	else
		String = strcpy(newstr, str);
	}

	inline void Usecode_machine::push(Usecode_value& val)	// Push/pop stack.
		{ *sp++ = val; }
	inline Usecode_value Usecode_machine::pop()		// +++++Watch for too many str. copies.
		{ 
		if (sp <= stack)
			stack_error(1);
		return *--sp; 
		}
	inline void Usecode_machine::pushref(Game_object *obj)
		{
		Usecode_value v(obj);
		push(v);
		} 
	inline void Usecode_machine::pushi(long val)		// Push/pop integers.
		{
		Usecode_value v(val);
		push(v);
		}
	inline int Usecode_machine::popi()
		{
		Usecode_value val = pop();
		return val.need_int_value();
		}
					// Push/pop strings.
	inline void Usecode_machine::pushs(char *s)
		{
		Usecode_value val(s);
		push(val);
		}
/*
 *	Get a game object from an "itemref", which might be the actual
 *	pointer, or might be -(npc number).
 *
 *	Output:	->game object.
 */

Game_object *Usecode_machine::get_item
	(
	Usecode_value& itemref
	)
	{
					// If array, take 1st element.
	Usecode_value& elemval = itemref.get_elem(0);

	if (elemval.is_ptr())
		return elemval.get_ptr_value();

	long val = elemval.get_int_value();
	if (!val)
		return NULL;
	Game_object *obj = NULL;
	if (val == -356)		// Avatar.
		return gwin->get_main_actor();
	if (val < 0 && val > -356)
		obj = gwin->get_npc(-val);
	else if (val >= 0 && val < gwin->get_num_npcs())
		obj = gwin->get_npc(val);
					// Special case:  palace guards.
	else if (val >= 0 && val < 0x400)		// Looks like a shape #?
		{
		if (!itemref.is_array() &&
 		    caller_item && val == caller_item->get_shapenum())
			obj = caller_item;
		else
			return 0;	// Can't be an object.
		}
	return obj;
	}

/*
 *	Check for an actor.
 */

Actor *Usecode_machine::as_actor
	(
	Game_object *obj
	)
	{
	if (!obj)
		return 0;
	return (dynamic_cast<Actor *> (obj));
	}

/*
 *	Check for an actor.
 */

Npc_actor *Usecode_machine::as_npcactor
	(
	Game_object *obj
	)
	{
	if (!obj)
		return 0;
	return (dynamic_cast<Npc_actor *> (obj));
	}

/*
 *	Get a position.
 */

Tile_coord Usecode_machine::get_position
	(
	Usecode_value& itemval
	)
	{
	Game_object *obj;		// An object?
	if ((itemval.get_array_size() == 1 || !itemval.get_array_size()) && 
						(obj = get_item(itemval)))
			return obj->get_outermost()->get_abs_tile_coord();
	else if (itemval.get_array_size() == 3)
					// An array of coords.?
		return Tile_coord(itemval.get_elem(0).get_int_value(),
				itemval.get_elem(1).get_int_value(),
				itemval.get_elem(2).get_int_value());
	else if (itemval.get_array_size() == 4)
					// Result of click_on_item() with
					//  array = (null, tx, ty, tz)?
		return Tile_coord(itemval.get_elem(1).get_int_value(),
				itemval.get_elem(2).get_int_value(),
				itemval.get_elem(3).get_int_value());
	else				// Else assume caller_item.
		return caller_item->get_abs_tile_coord();
	}

/*
 *	Make sure pending text has been seen.
 */

void Usecode_machine::show_pending_text
	(
	)
	{
	if (book)			// Book mode?
		{
		int x, y;
		while (book->show_next_page(gwin) && 
						Get_click(x, y, Mouse::hand))
			;
		gwin->paint();
		}
					// Normal conversation:
	else if (conv->is_npc_text_pending())
		click_to_continue();
	}

/*
 *	Show book or scroll text.
 */

void Usecode_machine::show_book
	(
	)
	{
	char *str = String;
	book->add_text(str);
	delete [] String;
	String = 0;
	}

/*
 *	Say the current string and empty it.
 */

void Usecode_machine::say_string
	(
	)
	{
	user_choice = 0;		// Clear user's response.
	if (!String)
		return;
	if (book)			// Displaying a book?
		{
		show_book();
		return;
		}
	show_pending_text();		// Make sure prev. text was seen.
	char *str = String;
	while (*str)			// Look for stopping points ("~~").
		{
		char *eol = str;
		while ((eol = strchr(str, '~')) != 0)
			break;
		if (!eol)		// Not found?
			{
			conv->show_npc_message(str);
			break;
			}
		*eol = 0;
		conv->show_npc_message(str);
		click_to_continue();
		str = eol + 1;
		if (*str == '~')
			str++;		// 2 in a row.
		}
	delete [] String;
	String = 0;
	}

/*
 *	Stack error.
 */

void Usecode_machine::stack_error
	(
	int under			// 1 if underflow.
	)
	{
	if (under)
		cerr << "Stack underflow." << endl;
	else
		cerr << "Stack overflow." << endl;
	exit(1);
	}

/*
 *	Show an NPC's face.
 */

void Usecode_machine::show_npc_face
	(
	Usecode_value& arg1,		// Shape (NPC #).
	Usecode_value& arg2,		// Frame.
	int slot			// 0, 1, or -1 to find free spot.
	)
	{
	show_pending_text();
	Actor *npc = (Actor *) get_item(arg1);
	if (!npc)
		return;
	int shape = npc->get_face_shapenum();
	int frame = arg2.get_int_value();
	gwin->remove_text_effects();
	if (gwin->get_mode() != Game_window::conversation)
		{
		gwin->end_gump_mode();
		gwin->paint();
		init_conversation();	// jsf-Added 4/20/01 for SI-Lydia.
		}
	conv->show_face(shape, frame, slot);
//	user_choice = 0;		// Seems like a good idea.
// Also seems to create a conversation bug in Test of Love :-(

	}

/*
 *	Remove an NPC's face.
 */

void Usecode_machine::remove_npc_face
	(
	Usecode_value& arg1		// Shape (NPC #).
	)
	{
	show_pending_text();
	int shape = -arg1.get_int_value();
	conv->remove_face(shape);
	}

/*
 *	Set an item's shape.
 */

void Usecode_machine::set_item_shape
	(
	Usecode_value& item_arg,
	Usecode_value& shape_arg
	)
	{
	int shape = shape_arg.get_int_value();
	Game_object *item = get_item(item_arg);
	if (!item)
		return;
					// See if light turned on/off.
	int light_changed = gwin->get_info(item).is_light_source() !=
			    gwin->get_info(shape).is_light_source();
	if (item->get_owner())		// Inside something?
		{
		item->get_owner()->change_member_shape(item, shape);
		if (light_changed)	// Maybe we should repaint all.
			gwin->paint();	// Repaint finds all lights.
		else
			{
			Gump *gump = gwin->find_gump(item);
			if (gump)
				gump->paint(gwin);
			}
		return;
		}
					// Figure area to repaint.
//	Rectangle rect = gwin->get_shape_rect(item);
	gwin->add_dirty(item);
					// Get chunk it's in.
	Chunk_object_list *chunk = gwin->get_objects(item);
	chunk->remove(item);		// Remove and add to update cache.
	item->set_shape(shape);
	chunk->add(item);
	gwin->add_dirty(item);
//	rect = gwin->get_shape_rect(item).add(rect);
//	rect.enlarge(8);
//	rect = gwin->clip_to_win(rect);
	if (light_changed)
		gwin->paint();		// Complete repaint refigures lights.
//	else
//		gwin->paint(rect);	// Not sure...
//	gwin->show();			// Not sure if this is needed.
	}

/*
 *	Set an item's frame.
 *	+++++Modified to just set_dirty on Nov22,2000
 */

void Usecode_machine::set_item_frame
	(
	Usecode_value& item_arg,
	Usecode_value& frame_arg
	)
	{
	Game_object *item = get_item(item_arg);
	if (!item)
		return;
	int frame = frame_arg.get_int_value();
	if (frame == item->get_framenum())
		return;			// Already set to that.
	// cout << "Set_item_frame: " << item->get_shapenum() 
	//				<< ", " << frame << endl;
					// (Don't mess up rotated frames.)
	if ((frame&0xf) < gwin->get_shape_num_frames(item->get_shapenum()))
		{
		if (item->get_owner())	// Inside a container?
			{
			item->set_frame(frame);
			Gump *gump = gwin->find_gump(item);
			if (gump)
				gwin->set_all_dirty();
			}
		else
			{		// Figure area to repaint.
			gwin->add_dirty(item);
			item->set_frame(frame);
			gwin->add_dirty(item);
			}
		}
	gwin->set_painted();		// Make sure paint gets done.
//	gwin->show();
	}

/*
 *	Remove an item from the world.
 */

void Usecode_machine::remove_item
	(
	Game_object *obj
	)
	{
	if (!obj)
		return;
	if (obj == last_created)
		last_created = 0;
	if (obj->get_owner())		// Inside a container?
		{			// Paint gump if open.
		Gump *gump = gwin->find_gump(obj);
		if (gump)
			gump->paint(gwin);
		}
	else
		gwin->add_dirty(obj);
	gwin->delete_object(obj);
	}

#define PARTY_MAX (sizeof(party)/sizeof(party[0]))

/*
 *	Is an NPC in the party?
 */

int Usecode_machine::npc_in_party
	(
	Game_object *npc
	)
	{
	return (npc && npc->get_party_id() >= 0);
	}

/*
 *	Return an array containing the party.
 */

Usecode_value Usecode_machine::get_party
	(
	)
	{
	Usecode_value arr(1 + party_count, 0);
					// Add avatar.
	Usecode_value aval(gwin->get_main_actor());
	arr.put_elem(0, aval);	
	int num_added = 1;
	for (int i = 0; i < party_count; i++)
		{
		Game_object *obj = gwin->get_npc(party[i]);
		if (!obj)
			continue;
		Usecode_value val(obj);
		arr.put_elem(num_added++, val);
		}
	// cout << "Party:  "; arr.print(cout); cout << endl;
	return arr;
	}

/*
 *	Recursively look for a barge that an object is a part of, or on.
 *
 *	Output:	->barge if found, else 0.
 */

Barge_object *Get_barge
	(
	Game_object *obj
	)
	{
	int barge_shape = 961;
	Game_object *found = obj->find_closest(&barge_shape, 1);
	if (found)
		return dynamic_cast<Barge_object *> (found);
	else
		return 0;
	}

/*
 *	Put text near an item.
 */

void Usecode_machine::item_say
	(
	Usecode_value& objval,
	Usecode_value& strval
	)
	{
	Game_object *obj = get_item(objval);
	const char *str = strval.get_str_value();
	if (obj && str && *str)
		{
					// See if it's in a gump.
		Gump *gump = gwin->find_gump(obj);
		Rectangle box;
		if (gump)
			box = gump->get_shape_rect(obj);
		else
			box = gwin->get_shape_rect(obj->get_outermost());
		gwin->add_text(str, box.x, box.y, obj);
//		gwin->show();		// Not sure.+++++testing.
		}
	}

/*
 *	Activate all cached-in usecode eggs near a given spot.
 */

void Usecode_machine::activate_cached
	(
	Tile_coord pos
	)
	{
	if (Game::get_game_type() != BLACK_GATE)
		return;			// ++++Since we're not sure about it.
	const int dist = 16;
	Egg_vector vec;			// Find all usecode eggs.
	Game_object::find_nearby(vec, pos, 275, dist, 16, c_any_qual, 7);
	for (Egg_vector::const_iterator it = vec.begin(); it != vec.end(); 
									++it)
		{
		Egg_object *egg = *it;
		if (egg->get_criteria() == Egg_object::cached_in)
			egg->activate(this);
		}
	}

/*
 *	Return an array of nearby objects.
 */

Usecode_value Usecode_machine::find_nearby
	(
	Usecode_value& objval,		// Find them near this.
	Usecode_value& shapeval,	// Shape to find, or -1 for any,
					//  c_any_shapenum for any npc.
	Usecode_value& distval,		// Distance in tiles?
	Usecode_value& mval		// Some kind of mask?  Guessing:
					//   4 == party members only.
					//   8 == non-party NPC's only.
					//  16 == something with eggs???
					//  32 == monsters? invisible?
	)
	{
	Game_object_vector vec;			// Gets list.
					// It might be (tx, ty, tz).
	int arraysize = objval.get_array_size();
	if (arraysize >= 3 && objval.get_elem(0).get_int_value() < c_num_tiles)
		{
					// Qual is 4th if there.
		int qual = arraysize == 5 ? objval.get_elem(3).get_int_value()
							: c_any_qual;
					// Frame is 5th if there.
		int frnum = arraysize == 5 ? objval.get_elem(4).get_int_value()
							: c_any_framenum;
		Game_object::find_nearby(vec,
			Tile_coord(objval.get_elem(0).get_int_value(),
				   objval.get_elem(1).get_int_value(),
				   objval.get_elem(2).get_int_value()),
			shapeval.get_int_value(),
			distval.get_int_value(), mval.get_int_value(), 
			qual, frnum);
		}
	else
		{
		Game_object *obj = get_item(objval);
		if (!obj)
			return Usecode_value(0, 0);
		obj = obj->get_outermost();	// Might be inside something.
		obj->find_nearby(vec, shapeval.get_int_value(),
			distval.get_int_value(), mval.get_int_value());
		}
	Usecode_value nearby(vec.size(), 0);	// Create return array.
	int i = 0;
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *each = *it;
		Usecode_value val(each);
		nearby.put_elem(i++, val);
		}
	return (nearby);
	}

/*
 *	Return object of given shape nearest given obj.
 */

Usecode_value Usecode_machine::find_nearest
	(
	Usecode_value& objval,		// Find near this.
	Usecode_value& shapeval,	// Shape to find
	Usecode_value& distval		// Guessing it's distance.
	)
	{
	Game_object *obj = get_item(objval);
	if (!obj)
		return Usecode_value((Game_object*) NULL);
	Game_object_vector vec;			// Gets list.
	obj = obj->get_outermost();	// Might be inside something.
	int dist = distval.get_int_value();
	int shnum = shapeval.get_int_value();
					// Kludge for Test of Courage:
	if (cur_function->id == 0x70a && shnum == 0x9a && dist == 0)
		dist = 16;		// Mage may have wandered.
	obj->find_nearby(vec, shnum, dist, 0);
	Game_object *closest = 0;
	uint32 bestdist = 100000;// Distance-squared in tiles.
	int x1, y1, z1;
	obj->get_abs_tile(x1, y1, z1);
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *each = *it;
		int x2, y2, z2;
		each->get_abs_tile(x2, y2, z2);
		int dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
		uint32 dist = dx*dx + dy*dy + dz*dz;
		if (dist < bestdist)
			{
			bestdist = dist;
			closest = each;
			}
		}
	return Usecode_value(closest);
	}

/*
 *	Find the angle (0-7) from one object to another.
 */

Usecode_value Usecode_machine::find_direction
	(
	Usecode_value& from,
	Usecode_value& to
	)
	{
	unsigned angle;			// Gets angle 0-7 (north - northwest)
	Tile_coord t1 = get_position(from);
	Tile_coord t2 = get_position(to);
					// Treat as cartesian coords.
	angle = (int) Get_direction(t1.ty - t2.ty, t2.tx - t1.tx);
	return Usecode_value(angle);
	}

/*
 *	Count objects of a given shape in a container, or in the whole party.
 */

Usecode_value Usecode_machine::count_objects
	(
	Usecode_value& objval,		// The container, or -357 for party.
	Usecode_value& shapeval,	// Object shape to count (c_any_shapenum=any).
	Usecode_value& qualval,		// Quality (c_any_qual=any).
	Usecode_value& frameval		// Frame (c_any_framenum=any).
	)
	{
	long oval = objval.get_int_value();
	int shapenum = shapeval.get_int_value();
	int qualnum = qualval.get_int_value();
	int framenum = frameval.get_int_value();
	if (oval != -357)
		{
		Game_object *obj = get_item(objval);
		return (!obj ? 0 : obj->count_objects(
					shapenum, qualnum, framenum));
		}
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	int total = 0;
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (obj)
			total += obj->count_objects(shapenum, qualnum, 
								framenum);
		}
	return (total);
	}

/*
 *	Get objects of a given shape in a container.
 */

Usecode_value Usecode_machine::get_objects
	(
	Usecode_value& objval,		// The container.
	Usecode_value& shapeval,	// Object shape to get or c_any_shapenum for any.
	Usecode_value& qualval,		// Quality (c_any_qual=any).
	Usecode_value& frameval		// Frame (c_any_framenum=any).
	)
	{
	Game_object *obj = get_item(objval);
	if (!obj)
		return Usecode_value((Game_object*) NULL);
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	int qual = qualval.get_int_value();
	Game_object_vector vec;			// Gets list.
	obj->get_objects(vec, shapenum, qual, framenum);

//	cout << "Container objects found:  " << cnt << << endl;
	Usecode_value within(vec.size(), 0);	// Create return array.
	int i = 0;
	for (Game_object_vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *each = *it;
		Usecode_value val(each);
		within.put_elem(i++, val);
		}
	return (within);
	}

/*
 *	Remove a quantity of an item from the party.
 *
 *	Output:	1 if successful, else 0.
 */

int Usecode_machine::remove_party_items
	(
	Usecode_value& quantval,	// Quantity to remove.
	Usecode_value& shapeval,	// Shape.
	Usecode_value& qualval,		// Quality??
	Usecode_value& frameval,	// Frame.
	Usecode_value& flagval		// Flag??
	)
	{
	int quantity = quantval.need_int_value();
	if (quantity == -359 && Game::get_game_type() == SERPENT_ISLE)
		quantity = 1;		// Guessing.  Got to remove something.
	Usecode_value all(-357);	// See if they exist.
	Usecode_value avail = count_objects(all, shapeval, qualval, frameval);
	if (avail.get_int_value() < quantity)
		return 0;
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	int quality = qualval.get_int_value();
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	for (int i = 0; i < cnt && quantity > 0; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (obj)
			quantity = obj->remove_quantity(quantity, shapenum,
							quality, framenum);
		}
	return (quantity == 0);
	}

/*
 *	Add a quantity of an item to the party.
 *
 *	Output:	List of members who got objects.
 */

Usecode_value Usecode_machine::add_party_items
	(
	Usecode_value& quantval,	// Quantity to add.
	Usecode_value& shapeval,	// Shape.
	Usecode_value& qualval,		// Quality.
	Usecode_value& frameval,	// Frame.
	Usecode_value& flagval		// Flag??
	)
	{
	int quantity = quantval.get_int_value();
					// ++++++First see if there's room.
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	unsigned int quality = (unsigned int) qualval.get_int_value();
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	Usecode_value result((Game_object*) NULL);  // +++++ Itemref or integer?
	for (int i = 0; i < cnt && quantity > 0; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (!obj)
			continue;
		int prev = quantity;
		quantity = obj->add_quantity(quantity, shapenum,
							quality, framenum);
		if (quantity < prev)
			{		// Added to this NPC.
			if (!result.is_array())
				result = Usecode_value(1, &party.get_elem(i));
			else
				result.concat(party.get_elem(i));
			}
		}
	return result;
	}

/*
 *	Add a quantity of an item to a container
 *
 *	Output:	Num created
 */

Usecode_value Usecode_machine::add_cont_items
	(
	Usecode_value& container,	// What do we add to
	Usecode_value& quantval,	// Quantity to add.
	Usecode_value& shapeval,	// Shape.
	Usecode_value& qualval,		// Quality.
	Usecode_value& frameval,	// Frame.
	Usecode_value& flagval		// Flag??
	)
	{
	int quantity = quantval.get_int_value();
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	unsigned int quality = (unsigned int) qualval.get_int_value();

	Game_object *obj = get_item(container);
	if (obj) return Usecode_value (obj->add_quantity(quantity, shapenum, quality, framenum));
	return Usecode_value(0);
	}

/*
 *	Remove a quantity of an item to a container
 *
 *	Output:	Num removed
 */

Usecode_value Usecode_machine::remove_cont_items
	(
	Usecode_value& container,	// What do we add to
	Usecode_value& quantval,	// Quantity to add.
	Usecode_value& shapeval,	// Shape.
	Usecode_value& qualval,		// Quality.
	Usecode_value& frameval,	// Frame.
	Usecode_value& flagval		// Flag??
	)
	{
	int quantity = quantval.get_int_value();
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	unsigned int quality = (unsigned int) qualval.get_int_value();

	Game_object *obj = get_item(container);
	if (obj) return Usecode_value (quantity - obj->remove_quantity(quantity, shapenum, quality, framenum));
	return Usecode_value(0);
	}

/*
 *	Have the user choose an object/spot with the mouse.
 *
 *	Output:	4-elem array:  (Ref. to item, or 0; tx, ty, tz)
 */

Usecode_value Usecode_machine::click_on_item
	(
	)
	{
	int x, y;
	if (!Get_click(x, y, Mouse::greenselect))
		return Usecode_value(0);
					// Get abs. tile coords. clicked on.
	int tx = gwin->get_scrolltx() + x/c_tilesize;
	int ty = gwin->get_scrollty() + y/c_tilesize;
	int tz = 0;
					// Look for obj. in open gump.
	Gump *gump = gwin->find_gump(x, y);
	Game_object *obj;
	if (gump)
		obj = gump->find_object(x, y);
	else				// Search rest of world.
		{
		obj = gwin->find_object(x, y);
		if (obj)		// Found object?  Use its coords.
			obj->get_abs_tile(tx, ty, tz);
		}
	Usecode_value oval(obj);	// Ret. array with obj as 1st elem.
	Usecode_value ret(4, &oval);
	Usecode_value xval(tx), yval(ty), zval(tz);
	ret.put_elem(1, xval);
	ret.put_elem(2, yval);
	ret.put_elem(3, zval);
	return (ret);
	}

/*
 *	Find an unblocked tile within a distance of 3 from a given point.
 *
 *	Output:	Returns tile, or original if not found.
 */

static Tile_coord Find_unblocked
	(
	Tile_coord dest			// Where to go.
	)
	{
	Tile_coord start = dest;
	dest.tx = -1;			// Look outwards.
	for (int i = 0; dest.tx == -1 && i < 3; i++)
		dest = Game_object::find_unblocked_tile(start, i);
	if (dest.tx == -1)
		return start;
	else
		return dest;
	}


/*
 *	Have an NPC walk somewhere and then execute usecode.
 *
 *	Output:	1 if successful, else 0.
 */

int Usecode_machine::path_run_usecode
	(
	Usecode_value& npcval,		// # or ref.
	Usecode_value& locval,		// Where to walk to.
	Usecode_value& useval,		// Usecode #.
	Usecode_value& itemval,		// Use as itemref in Usecode fun.
	Usecode_value& eventval,	// Eventid.
	int find_free			// Not sure.  For SI.  
	)
	{
	Actor *npc = as_actor(get_item(npcval));
	if (!npc)
		return 0;
	int sz = locval.get_array_size();
	if (sz != 3)			// Looks like tile coords.
		{	//++++++Not sure about this.
		cout << "0x7d Location not a 3-int array" << endl;
		return 0;
		}
					// Get source, dest.
	Tile_coord src = npc->get_abs_tile_coord();
	int dx = locval.get_elem(0).get_int_value();
	int dy = locval.get_elem(1).get_int_value();
	int dz = locval.get_elem(2).get_int_value();
	cout << endl << "Path_run_usecode:  first walk to (" << 
			dx << ", " << dy << ", " << dz << ")" << endl;
	Tile_coord dest(dx, dy, dz);
	if (find_free)
		{
		dest = Find_unblocked(dest);
		if (src != dest && !npc->walk_path_to_tile(dest))
			{		// Try again at npc's level.
			dest.tz = src.tz;
			dest = Find_unblocked(dest);
			if (src != dest && !npc->walk_path_to_tile(dest))
				{
				cout << "Failed to find path" << endl;
				return 0;
				}
			}
		}
	else if (src != dest &&		// Just try once.
	    !npc->walk_path_to_tile(dest))
		{			// Failed to find path.
		cout << "Failed to find path" << endl;
		return 0;
		}
	Wait_for_arrival(npc, dest);
	Game_object *obj = get_item(itemval);
	if (obj)
		{
		call_usecode(useval.get_int_value(), obj, 
				(Usecode_events) eventval.get_int_value());
		return 1;	// Success.
		}
	return 0;
	}

/*
 *	Report unhandled intrinsic.
 */

static void Usecode_Trace
	(
	const char *name,
	int intrinsic,
	int num_parms,
	Usecode_value parms[12]
	)
	{
	cout << hex << "    [0x" << setfill((char)0x30) << setw(2)
		<< intrinsic << "]: " << name << "(";
	for (int i = 0; i < num_parms; i++)
		{
		parms[i].print(cout);
		if(i!=num_parms-1)
			cout << ", ";
		}
	cout <<") = ";
	cout << dec;
	}

static	void	Usecode_TraceReturn(Usecode_value &v)
{
	v.print(cout);
	cout << dec << endl;
}

#if 0	/* Not used at the moment. */
static void Unhandled
	(
	int intrinsic,
	int num_parms,
	Usecode_value parms[12]
	)
	{
	Usecode_Trace("UNKNOWN",intrinsic,num_parms,parms);
	}
#endif

Usecode_value no_ret;

Usecode_value Usecode_machine::Execute_Intrinsic(UsecodeIntrinsicFn func,const char *name,int event,int intrinsic,int num_parms,Usecode_value parms[12])
{
#ifdef XWIN
#if USECODE_DEBUGGER
	if(usecode_debugging)
		{
		// Examine the list of intrinsics for function breakpoints.
		if(std::find(intrinsic_breakpoints.begin(),intrinsic_breakpoints.end(),intrinsic)!=intrinsic_breakpoints.end())
			{
			raise(SIGIO);	// Breakpoint
			}
		}
#endif
#endif
	if(intrinsic_trace)
		{
		Usecode_Trace(name,intrinsic,num_parms,parms);
		cout.flush();
		Usecode_value u=((*this).*func)(event,intrinsic,num_parms,parms);
		Usecode_TraceReturn(u);
		return (u);
		}
	else
		return ((*this).*func)(event,intrinsic,num_parms,parms);
}


typedef	Usecode_value (Usecode_machine::*UsecodeIntrinsicFn)(int event,int intrinsic,int num_parms,Usecode_value parms[12]);

// missing from mingw32 header files, so included manually
#ifndef __STRING
#if defined __STDC__ && __STDC__
#define __STRING(x) #x
#else
#define __STRING(x) "x"
#endif
#endif

#define	USECODE_INTRINSIC_PTR(NAME)	{ &Usecode_machine::UI_ ## NAME, __STRING(NAME) }

struct Usecode_machine::IntrinsicTableEntry
	  Usecode_machine::intrinsic_table[]=
	{
#include "bgintrinsics.h"
	};

// Serpent Isle Intrinsic Function Tablee
// It's different to the Black Gate one.
struct Usecode_machine::IntrinsicTableEntry
	  Usecode_machine::serpent_table[]=
	{
#include "siintrinsics.h"
	};


int	max_bundled_intrinsics=0xff;	// Index of the last intrinsic in this table
/*
 *	Call an intrinsic function.
 */

Usecode_value Usecode_machine::call_intrinsic
	(
	int event,			// Event type.
	int intrinsic,			// The ID.
	int num_parms			// # parms on stack.
	)
	{
	Usecode_value parms[12];	// Get parms.
	for (int i = 0; i < num_parms; i++)
		{
		Usecode_value val = pop();
		parms[i] = val;
		}
	if (intrinsic<=max_bundled_intrinsics)
		{
		struct Usecode_machine::IntrinsicTableEntry *table_entry;
		
		if (Game::get_game_type() == SERPENT_ISLE)
			table_entry = serpent_table+intrinsic;
		else
			table_entry = intrinsic_table+intrinsic;
		UsecodeIntrinsicFn func=(*table_entry).func;
		const char *name=(*table_entry).name;
		return Execute_Intrinsic(func,name,event,intrinsic,
							num_parms,parms);
		}
	return(no_ret);
	}

/*
 *	Wait for user to click.  (Mainly for testing).
 */

void Usecode_machine::click_to_continue
	(
	)
	{
	int xx, yy;
	char c;
	if (!gwin->is_palette_faded_out())// If black screen, skip!
		Get_click(xx, yy, Mouse::hand, &c);
	conv->clear_text_pending();
	user_choice = 0;		// Clear it.
	}

/*
 *	Set book/scroll to display.
 */

void Usecode_machine::set_book
	(
	Text_gump *b			// Book/scroll.
	)
	{
	delete book;
	book = b;
	}

/*
 *	Get user's choice from among the possible responses.
 *
 *	Output:	->user choice string.
 *		0 if no possible choices or user quit.
 */

const char *Usecode_machine::get_user_choice
	(
	)
	{
	if (!conv->get_num_answers())
		return (0);		// Shouldn't happen.
	if (!user_choice)		// May have already been done.
		get_user_choice_num();
	return (user_choice);
	}

/*
 *	Get user's choice from among the possible responses.
 *
 *	Output:	User choice is set, with choice # returned.
 *		-1 if no possible choices.
 */

int Usecode_machine::get_user_choice_num
	(
	)
	{
	user_choice = 0;
	conv->show_avatar_choices();
	int x, y;			// Get click.
	int choice_num;
	do
		{
		char chr;		// Allow '1', '2', etc.
		int result=Get_click(x, y, Mouse::hand, &chr);
		if (result<=0) {	// ESC pressed, select 'bye' if poss.
			choice_num = conv->locate_answer("bye");
		} else if (chr) {		// key pressed
			if (chr>='1' && chr <='0'+conv->get_num_answers()) {
				choice_num = chr - '1';
			} else
				choice_num = -1;	//invalid key
		} else
			choice_num = conv->conversation_choice(x, y);
		}
					// Wait for valid choice.
	while (choice_num  < 0 || choice_num >= conv->get_num_answers());

	conv->clear_avatar_choices();
					// Store ->answer string.
	user_choice = conv->get_answer(choice_num);
	return (choice_num);		// Return choice #.
	}

/*
 *	Create machine from a 'usecode' file.
 */

Usecode_machine::Usecode_machine
	(
	istream& file,
	Game_window *gw
	) : gwin(gw), call_depth(0), cur_function(0),
	    speech_track(-1), book(0),  caller_item(0),
	    last_created(0), user_choice(0),
	    String(0), stack(new Usecode_value[1024])
	{
	_init_(file);
	}

Usecode_machine::Usecode_machine
	(
	Game_window *gw
	) : gwin(gw), call_depth(0), cur_function(0), book(0), caller_item(0),
	    last_created(0), user_choice(0),
	    String(0), stack(new Usecode_value[1024])
	{
	ifstream file;                // Read in usecode.
        U7open(file, USECODE);
	_init_(file);
	file.close();
	}

void Usecode_machine::_init_
	(
	istream &file
	)
	{
	sp = stack;
					// Clear global flags.
	memset(gflags, 0, sizeof(gflags));
					// Clear timers.
	memset((char *) &timers[0], 0, sizeof(timers));
					// Clear party list.
	memset((char *) &party[0], 0, sizeof(party));
	party_count = 0;
	conv = new Conversation;
	read_usecode(file);
	}

/*
 *	Read in usecode functions.  These may override previously read
 *	functions.
 */

void Usecode_machine::read_usecode
	(
	istream &file
	)
	{
	file.seekg(0, ios::end);
	int size = file.tellg();	// Get file size.
	file.seekg(0);
					// Read in all the functions.
	while (file.tellg() < size)
		{
		Usecode_function *fun = new Usecode_function(file);
		Exult_vector<Usecode_function *> & vec = funs[fun->id/0x100];
		int i = fun->id%0x100;
		if (i < vec.size())
			delete vec[i];
		vec.put(i, fun);
		}
	}

/*
 *	Delete.
 */

Usecode_machine::~Usecode_machine
	(
	)
	{
	delete [] stack;
	delete [] String;
	delete conv;
//	int num_slots = funs->get_cnt();
	int num_slots = sizeof(funs)/sizeof(funs[0]);
	for (int i = 0; i < num_slots; i++)
		{
#if 0
		Vector *slot = (Vector *) funs->get(i);
		if (!slot)
			continue;
		int cnt = slot->get_cnt();
		for (int j = 0; j < cnt; j++)
			delete (Usecode_function *) slot->get(j);
		delete slot;
#endif
		vector<Usecode_function*>& slot = funs[i];
		int cnt = slot.size();
		for (int j = 0; j < cnt; j++)
			delete slot[j];
		}
//	delete funs;
	delete book;
	}

#if DEBUG
int debug = 2;				// 2 for more stuff.
static int ucbp_fun = -1, ucbp_ip = -1;	// Breakpoint.
void Setbreak(int fun, int ip)
	{ ucbp_fun = fun; ucbp_ip = ip; }
void Clearbreak()
	{ ucbp_fun = ucbp_ip = -1; }
#endif

/*
 *	Interpret a single usecode function.
 *
 *	Output:	0 if ABRT executed.
 */

int Usecode_machine::run
	(
	Usecode_function *fun,
	int event,			// Event (??) that caused this call.
	int stack_elems			// # elems. on stack at call.
	)
	{
	call_depth++;
	int abort = 0;			// Flag if ABRT executed.
	unsigned char *catch_ip = 0;	// IP for catching an ABRT.
					// Save/set function.
	Usecode_function *save_fun = cur_function;
	cur_function = fun;
	uint8 *ip = fun->code;	// Instruction pointer.
					// Where it ends.
	uint8 *endp = ip + fun->len;
	int data_len = Read2(ip);	// Get length of (text) data.
	char *data = (char *) ip;	// Save ->text.
	ip += data_len;			// Point past text.
	int num_args = Read2(ip);	// # of args. this function takes.
	while (num_args > stack_elems)	// Not enough args pushed?
		{			// Push 0's.
		pushi(0);
		stack_elems++;
		}
					// Local variables follow args.
	int num_locals = Read2(ip) + num_args;
					// Allocate locals.
	Usecode_value *locals = new Usecode_value[num_locals];
					// Store args.
	for (int i = 0; i < num_args; i++)
		{
		Usecode_value val = pop();
		locals[num_args - i - 1] = val;
		}
#if DEBUG
	if (debug >= 0)
		{
		cout << "Running usecode " << hex << setfill((char)0x30) 
			<< setw(4) << fun->id << dec << setfill(' ') <<
			" (";
		for (int i = 0; i < num_args; i++)
			{
			if (i)
				cout << ", ";
			locals[i].print(cout);
			}
		cout << ") with event " << event << endl;
		}
#endif
	Usecode_value *save_sp = sp;	// NOW, save TOS, last-created.
	int num_externs = Read2(ip);	// # of external refs. following.
	unsigned char *externals = ip;	// Save -> to them.
	ip += 2*num_externs;		// ->actual bytecode.
	int offset;			// Gets offset parm.
	int sval;			// Get value from top-of-stack.
	unsigned char *code = ip;	// Save for debugging.
	int set_ret_value = 0;		// >=1 when return value is set.
	Usecode_value ret_value;	// Gets return value.
	/*
	 *	Main loop.
	 */
	while (ip < endp)
		{
		int opcode = *ip++;
#if DEBUG
		if (usecode_trace)
			{
			int curip = ip - 1 - code;
//			printf("SP = %d, IP = %04x, op = %02x\n", sp - stack,
//						curip, opcode);
			cout << "SP = " << sp - stack << ", IP = " << hex << curip
				 << ", op = "<< opcode << dec << endl;
			if (ucbp_fun == fun->id && ucbp_ip == curip)
				cout << "At breakpoint" << endl;
			cout.flush();
			}
#endif
		switch (opcode)
			{
		case 0x04:		// CATCH - Catch 'ABRT'.
			offset = (short) Read2(ip);
			catch_ip = ip + offset;
					// ++++++Not sure if this is needed:
			if (set_ret_value || !conv->get_num_answers() || abort)
				{
				ip = catch_ip;
				user_choice = 0;
				}
			break;
		case 0x05:		// JNE.
			{
			offset = (short) Read2(ip);
			Usecode_value val = pop();
			if (val.is_false())
				ip += offset;
			break;
			}
		case 0x06:		// JMP.
			offset = (short) Read2(ip);
			ip += offset;
			break;
		case 0x07:		// Guessing CMPS.
			{		// Skip out if ABRT in effect.
				//+++++Still necessary?  Try with Dell:nothing.
				// ++++Trying set_ret_value:  7/24/00
			if (/* abort || */ set_ret_value || !get_user_choice())
				user_choice = "";
			int cnt = Read2(ip);	// # strings.
			offset = (short) Read2(ip);
			while (cnt--)
				{
				Usecode_value s = pop();
				const char *str = s.get_str_value();
				if (!user_choice||!*user_choice)
					{
					cnt=-1;
					break;
					}
				if (str && strcmp(str, user_choice) == 0)
					break;
				}
			if (cnt == -1)	// Jump if no match.
				ip += offset;
			}
			break;
		case 0x09:		// ADD.
			{
			char buf[300];
			Usecode_value v2 = pop();
			Usecode_value v1 = pop();
			Usecode_value sum(0);
			if (v1.get_type() == Usecode_value::int_type)
				{
				if (v2.get_type() == Usecode_value::int_type)
					sum = Usecode_value(v1.get_int_value()
							+ v2.get_int_value());
				else if (v2.get_type() == 
						Usecode_value::string_type)
					{
					snprintf(buf, 300, "%ld%s", 
						v1.get_int_value(),
						v2.get_str_value());
					sum = Usecode_value(buf);
					}
				}
			else if (v1.get_type() == Usecode_value::string_type)
				{
				if (v2.get_type() == Usecode_value::int_type)
					{
					snprintf(buf, 300, "%s%ld", 
							v1.get_str_value(),
							v2.get_int_value());
					sum = Usecode_value(buf);
					}
				else if (v2.get_type() == 
						Usecode_value::string_type)
					{
					snprintf(buf, 300, "%s%s", 
							v1.get_str_value(),
							v2.get_str_value());
					sum = Usecode_value(buf);
					}
				else
					sum = v1;
				}
			push(sum);
			break;
			}
		case 0x0a:		// SUB.
			sval = popi();
			pushi(popi() - sval);
			break;
		case 0x0b:		// DIV.
			sval = popi();
			pushi(popi()/sval);
			break;
		case 0x0c:		// MUL.
			pushi(popi()*popi());
			break;
		case 0x0d:		// MOD.
			sval = popi();
			pushi(popi() % sval);
			break;
		case 0x0e:		// AND.
			{
			Usecode_value v1 = pop();
			Usecode_value v2 = pop();
			int result = v1.is_true() && v2.is_true();
			pushi(result);
			break;
			}
		case 0x0f:		// OR.
			{
			Usecode_value v1 = pop();
			Usecode_value v2 = pop();
			int result = v1.is_true() || v2.is_true();
			pushi(result);
			break;
			}
		case 0x10:		// NOT.
			pushi(!pop().is_true());
			break;
		case 0x12:		// POP into a variable.
			{
			offset = Read2(ip);
					// Get value.
			Usecode_value val = pop();
			if (offset < 0 || offset >= num_locals)
				cerr << "Local #" << offset << 
							"out of range" << endl;
			else
				locals[offset] = val;
			}
			break;
		case 0x13:		// PUSH true.
			pushi(1);
			break;
		case 0x14:		// PUSH false.
			pushi(0);
			break;
		case 0x16:		// CMPGT.
			sval = popi();
			pushi(popi() > sval);	// Order?
			break;
		case 0x17:		// CMPL.
			sval = popi();
			pushi(popi() < sval);
			break;
		case 0x18:		// CMPGE.
			sval = popi();
			pushi(popi() >= sval);
			break;
		case 0x19:		// CMPLE.
			sval = popi();
			pushi(popi() <= sval);
			break;
		case 0x1a:		// CMPNE.
			{
			Usecode_value val1 = pop();
			Usecode_value val2 = pop();
			pushi(!(val1 == val2));
			break;
			}
		case 0x1c:		// ADDSI.
			offset = Read2(ip);
			append_string(data + offset);
			break;
		case 0x1d:		// PUSHS.
			offset = Read2(ip);
			pushs(data + offset);
			break;
		case 0x1e:		// ARRC.
			{		// Get # values to pop into array.
			offset = Read2(ip);
			Usecode_value arr(offset, 0);
			int to = 0;	// Store at this index.
			while (offset--)
				{
				Usecode_value val = pop();
				to += arr.add_values(to, val);
				}
			push(arr);
			}
			break;
		case 0x1f:		// PUSHI.
			{		// Might be negative.
			short ival = Read2(ip);
			pushi(ival);
			break;
			}
		case 0x21:		// PUSH.
			offset = Read2(ip);
			push(locals[offset]);
			break;
		case 0x22:		// CMPEQ.
			{
			Usecode_value val1 = pop();
			Usecode_value val2 = pop();
			pushi(val1 == val2);
			break;
			}
		case 0x24:		// CALL.
			offset = Read2(ip);
			if (!call_usecode_function(externals[2*offset] + 
					256*externals[2*offset + 1], event,
					sp - save_sp))
				{	// Catch ABRT.
				abort = 1;
				if (catch_ip)
					ip = catch_ip;
				else	// No catch?  I think we should exit.
					{
					sp = save_sp;
					ip = endp;
					}
				}
			break;
		case 0x25:		// RET.
					// Experimenting...
			show_pending_text();
			sp = save_sp;		// Restore stack.
			ip = endp;	// End the loop.
			break;
		case 0x26:		// AIDX.
			{
			sval = popi();	// Get index into array.
			sval--;		// It's 1 based.
					// Get # of local to index.
			offset = Read2(ip);
			if (sval < 0 || offset < 0 || offset >= num_locals)
				{
				cerr << "Local #" << offset << 
							"out of range" << endl;
				pushi(0);
				break;
				}
			Usecode_value& val = locals[offset];
			push(val.get_elem(sval));
			break;
			}
		case 0x2c:		// Another kind of return?
					// Experimenting...
			show_pending_text();
			sp = save_sp;		// Restore stack.
			ip = endp;	// End the loop.
			break;
		case 0x2d:		// Set return value (SETR).
			{
			Usecode_value r = pop();
					// But 1st takes precedence.
			if (!set_ret_value)
				ret_value = r;
#if 0	/* ++++Looks like BG does too.  Need to for gangplank test. */
					// Looks like SI rets. here.
			if (Game::get_game_type() == SERPENT_ISLE ||
					// Fix infinite loop (0x944) bug.
				 set_ret_value > 40)
#endif
				{
				sp = save_sp;	// Restore stack, force ret.
				push(ret_value);
				ip = endp;
				}
			set_ret_value++;
			break;
			}
		case 0x2e:		// Looks like a loop.
			if (*ip++ != 2)
				cout << "2nd byte in loop isn't a 2!"<<endl;
					// FALL THROUGH.
		case 0x02:		// 2nd byte of loop.
			{
					// Counter (1-based).
			int local1 = Read2(ip);
					// Total count.
			int local2 = Read2(ip);
					// Current value of loop var.
			int local3 = Read2(ip);
					// Array of values to loop over.
			int local4 = Read2(ip);
					// Get offset to end of loop.
			offset = (short) Read2(ip);
					// Get array to loop over.
			Usecode_value& arr = locals[local4];
			if (opcode == 0x2e)
				{	// Initialize loop.
				int cnt = arr.is_array() ?
					arr.get_array_size() : 1;
				locals[local2] = Usecode_value(cnt);
				locals[local1] = Usecode_value(0);
				}
			int next = locals[local1].get_int_value();
					// End of loop?
			if (next >= locals[local2].get_int_value())
				ip += offset;
			else		// Get next element.
				{
				locals[local3] = arr.is_array() ?
					arr.get_elem(next) : arr;
				locals[local1] = Usecode_value(next + 1);
				}
			break;
			}
		case 0x2f:		// ADDSV.
			{
			offset = Read2(ip);
			const char *str = locals[offset].get_str_value();
			if (str)
				append_string(str);
			else		// Convert integer.
				{
				char buf[20];
				snprintf(buf, 20, "%ld",
					locals[offset].get_int_value());
				append_string(buf);
				}
			break;
			}
		case 0x30:		// IN.  Is a val. in an array?
			{
			Usecode_value arr = pop();
					// If an array, use 1st elem.
			Usecode_value val = pop().get_elem(0);
			pushi(arr.find_elem(val) >= 0);
			break;
			}
		case 0x31:		// Unknown.
			ip += 4;
			break;
		case 0x32:		// RTS: Push return value & ret. 
					//   from function.
			sp = save_sp;	// Restore stack.
			push(ret_value);
			ip = endp;
			break;
		case 0x33:		// SAY.
			say_string();
			break;
		case 0x38:		// CALLIS.
			{
			offset = Read2(ip);
			sval = *ip++;	// # of parameters.
			Usecode_value ival = call_intrinsic(event,
							offset, sval);
			push(ival);
			}
			break;
		case 0x39:		// CALLI.
			offset = Read2(ip);
			sval = *ip++;	// # of parameters.
			call_intrinsic(event, offset, sval);
			break;
		case 0x3e:		// PUSH ITEMREF.
			pushref(caller_item);
			break;
		case 0x3f:		// ABRT - Really like a 'throw'.
			show_pending_text();
			ip = endp;
			sp = save_sp;	// Restore stack.
			abort = 1;
			break;
		case 0x40:		// CATCH jmps here.
			abort = 0;	// ++++Experiment 7/26/00.
			catch_ip = 0;
			break;
		case 0x42:		// PUSHF.
			offset = Read2(ip);
			pushi(gflags[offset]);
			break;
		case 0x43:		// POPF.
			offset = Read2(ip);
			gflags[offset] = (unsigned char) popi();
			break;
		case 0x44:		// PUSHB.
			pushi(*ip++);
			break;
		case 0x45:		// Unknown.
			ip++;
			break;
		case 0x46:		// Set array element.
			{
					// Get # of local array.
			offset = Read2(ip);
			Usecode_value& arr = locals[offset];
			short index = popi();
			index--;	// It's 1-based.
			Usecode_value val = pop();
			int size = arr.get_array_size();
			if (index >= 0 && 
			    (index < size || arr.resize(index + 1)))
				arr.put_elem(index, val);
			break;
			}
		case 0x47:		// CALLE.  Stack has caller_item.
			{
			Game_object *prev_item = caller_item;
			Usecode_value ival = pop();
			caller_item = get_item(ival);
			push(ival);
			offset = Read2(ip);
			if (!call_usecode_function(offset, event, 
								sp - save_sp))
				{	// Catch ABRT.
				abort = 1;
				if (catch_ip)
					ip = catch_ip;
				else	// No catch?  I think we should exit.
					{
					sp = save_sp;
					ip = endp;
					}
				}
			caller_item = prev_item;
			break;
			}
		case 0x48:		// PUSH EVENTID.
			pushi(event);
			break;
		case 0x4a:		// ARRA.
			{
			Usecode_value val = pop();
			Usecode_value arr = pop();
			push(arr.concat(val));
			break;
			}
		case 0x4b:		// POP EVENTID.
			event = popi();
			break;
		default:
			cout << "Opcode " << opcode << " not known." << endl;
			break;
			}
		}
	delete [] locals;
#if DEBUG
	if (debug >= 1)
		{
		cout << "RETurning ";
		if (set_ret_value)
			{
			cout << "(";
			ret_value.print(cout);
			cout << ") ";
			}
		cout << "from usecode " << hex << setw(4) << 
			setfill((char)0x30) << fun->id << dec << setfill(' ')
			<< endl;
		}
#endif
	cout.flush();
	cur_function = save_fun;
	call_depth--;
	return (abort == 0);		// Return 0 if ABRT.
	}

/*
 *	Call a usecode function.
 *
 *	Output:	0 if ABRT executed.
 *		-1 if function not found.
 *		1 if okay.
 */

int Usecode_machine::call_usecode_function
	(
	int id,
	int event,			// Event (?) that caused this call.
	int stack_elems			// # elems. on stack at call.
	)
	{
					// Look up in table.
	vector<Usecode_function*>& slot = funs[id/0x100];
	size_t index = id%0x100;
	Usecode_function *fun = index < slot.size() ? slot[index] : 0;
	if (!fun)
		{
		cout << "Usecode " << id << " not found."<<endl;
		return (-1);
		}
					// Do it.  Rets. 0 if aborted.
	return run(fun, event, stack_elems);
	}

/*
 *	This is the main entry for outside callers.
 *
 *	Output:	-1 if not found.
 *		0 if can't execute now or if aborted.
 *		1 otherwise.
 */

int Usecode_machine::call_usecode
	(
	int id, 			// Function #.
	Game_object *obj,		// Item ref.
	Usecode_events event
	)
	{
					// Avoid these when already execing.
	if (call_depth && event == npc_proximity && Game::get_game_type() ==
								BLACK_GATE)
		return (0);
	Game_object *prev_item = caller_item;
	caller_item = obj;
	conv->clear_answers();
	int ret = call_usecode_function(id, event, 0);
	set_book(0);
	caller_item = prev_item;
	return ret;
	}

/*
 *	Write out global data to 'gamedat/usecode.dat'.
 *
 *	Output:	0 if error.
 */

void Usecode_machine::write
	(
	)
	{
	ofstream out;
	U7open(out, FLAGINIT);	// Write global flags.
	out.write((char*)gflags, sizeof(gflags));
	out.close();
	U7open(out, USEDAT);
	Write2(out, party_count);	// Write party.
	for (size_t i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		Write2(out, party[i]);
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		Write4(out, timers[t]);
					// +++++No longer needed.
	for (size_t i = 0; i < 8; i++)	// Virtue stones.
		{
		Write2(out, virtue_stones[i].tx);
		Write2(out, virtue_stones[i].ty);
		Write2(out, virtue_stones[i].tz);
		}
	out.flush();
	if( !out.good() )
		throw file_write_exception(USEDAT);
	}

/*
 *	Read in global data to 'gamedat/usecode.dat'.
 *
 *	Output:	0 if error.
 */

void Usecode_machine::read
	(
	)
	{
	ifstream in;
	try
	{
		U7open(in, FLAGINIT);	// Read global flags.
	}
	catch(...)
	{
		// +++++Eventually, remove this:
		U7open(in, "<STATIC>/flaginit.dat");
	}
	in.read((char*)gflags, sizeof(gflags));
	in.close();
	try
	{
		U7open(in, USEDAT);
	}
	catch(...)
	{
		return;		// Not an error if no saved game yet.
	}
	party_count = Read2(in);	// Read party.
	for (size_t i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		party[i] = Read2(in);
	link_party();
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		timers[t] = Read4(in);
	if (!in.good())
		throw file_read_exception(USEDAT);

					// +++++No longer needed:
	for (size_t i = 0; i < 8; i++)	// Virtue stones.
		{
		virtue_stones[i].tx = Read2(in);
		virtue_stones[i].ty = Read2(in);
		virtue_stones[i].tz = Read2(in);
		}
	if (!in.good())			// Failed.+++++Can remove this later.
		for (size_t i = 0; i < 8; i++)
			virtue_stones[i] = Tile_coord(0, 0, 0);
	}

/*
 *	In case NPC's were read after usecode, set party members' id's.
 */

void Usecode_machine::link_party
	(
	)
	{
	for (int i = 0; i < party_count; i++)
		{
		Actor *npc = gwin->get_npc(party[i]);
		if (npc)
			{
			npc->set_party_id(i);
			npc->set_schedule_type(Schedule::follow_avatar);
			}
		}
	}

void Usecode_machine::init_conversation()
{
	conv->init_faces();
}

int Usecode_machine::get_num_faces_on_screen() const
{
	 return conv->get_num_faces_on_screen();
}
