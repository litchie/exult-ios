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

#include <cstdio>			/* Debugging.			*/
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "utils.h"
#include "schedule.h"
#include "mouse.h"
#include "barge.h"
#include "ucmachine.h"
#include "gamewin.h"
#include "objs.h"
#include "delobjs.h"
#include "animate.h"
#include "vec.h"
#include "SDL.h"
#include "tqueue.h"
#include "Gump.h"
#include "Text_gump.h"
#include "useval.h"
#include "game.h"
#include <iomanip>
#include "ucsched.h"

#ifdef XWIN
#include <signal.h>
#endif
#if USECODE_DEBUGGER
#include <algorithm>       // STL function things
#endif

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
 *	Add a possible 'answer'.
 */

Answers::Answers()
{}

void Answers::clear(void)
{
	answers.clear();
}

void Answers::add_answer
	(
	const char *str
	)
	{
	_remove_answer(str);
	string	s(str);
	answers.push_back(s);
	}

/*
 *	Add an answer to the list.
 */

void Answers::add_answer
	(
	Usecode_value& val		// Array or string.
	)
	{
	const char *str;
	int size = val.get_array_size();
	if (size)			// An array?
		{
		for (int i = 0; i < size; i++)
			add_answer(val.get_elem(i));
		}
	else if ((str = val.get_str_value()) != 0)
		add_answer(str);
	}

/*
 *	Remove an answer from the list.
 */

void Answers::_remove_answer(const char *str)
{
	vector<string>::iterator it;

	for(it=answers.begin();
		it!=answers.end();
		++it)
		{
	//	cerr << "'" << *it << "' ~ '" << str << "'"<<endl;
		if(*it==str)
			break;
		}
	if(it!=answers.end())
		answers.erase(it);
}

void Answers::remove_answer
	(
	Usecode_value& val		// String or array of strings
	)
	{
#if 0
	const char *str = val.get_str_value();
	if (!str)
		return;
	_remove_answer(str);
#else
	const char *str;
	if (val.is_array()) {
		int size = val.get_array_size();
		for (int i=0; i < size; i++) {
			str = val.get_elem(i).get_str_value();
			if (str) _remove_answer(str);
		}
	} else {
		str = val.get_str_value();
		_remove_answer(str);
	}
#endif
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
	else if (gwin->is_npc_text_pending())
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
			gwin->show_npc_message(str);
			break;
			}
		*eol = 0;
		gwin->show_npc_message(str);
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
	Usecode_value& arg2		// Frame.
	)
	{
	show_pending_text();
	Actor *npc = (Actor *) get_item(arg1);
	if (!npc)
		return;
	int shape = npc->get_face_shapenum();
	int frame = arg2.get_int_value();
	gwin->remove_text_effects();
	gwin->end_gump_mode();
//	gwin->set_all_dirty();
	gwin->paint();
	gwin->show_face(shape, frame);
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
	gwin->remove_face(shape);
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
	Rectangle rect = gwin->get_shape_rect(item);
					// Get chunk it's in.
	Chunk_object_list *chunk = gwin->get_objects(item);
	chunk->remove(item);		// Remove and add to update cache.
	item->set_shape(shape);
	chunk->add(item);
	rect = gwin->get_shape_rect(item).add(rect);
	rect.enlarge(8);
	rect = gwin->clip_to_win(rect);
	if (light_changed)
		gwin->paint();		// Complete repaint refigures lights.
	else
		gwin->paint(rect);	// Not sure...
	gwin->show();			// Not sure if this is needed.
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
		item->set_frame(frame);
	if (item->get_owner())		// Inside a container?
		{
		Gump *gump = gwin->find_gump(item);
		if (gump)
			{
			item->set_frame(frame);
			gump->paint(gwin);
			}
		}
	else
		{			// Figure area to repaint.
		gwin->add_dirty(item);
		item->set_frame(frame);
		gwin->add_dirty(item);
		}
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
	obj->remove_this(1);		// Remove from world or container, but
					//   don't delete.
	obj->set_invalid();		// Set to invalid chunk.
	removed->insert(obj);		// Add to pool instead.
	gwin->paint();
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
		gwin->add_text(str, box.x, box.y);
		gwin->show();		// Not sure.+++++testing.
		}
	}

/*
 *	Return an array of nearby objects.
 */

Usecode_value Usecode_machine::find_nearby
	(
	Usecode_value& objval,		// Find them near this.
	Usecode_value& shapeval,	// Shape to find, or -1 for any,
					//  -359 for any npc.
	Usecode_value& distval,		// Distance in tiles?
	Usecode_value& mval		// Some kind of mask?  Guessing:
					//   4 == party members only.
					//   8 == non-party NPC's only.
					//  16 == something with eggs???
					//  32 == monsters? invisible?
	)
	{
	GOVector vec;			// Gets list.
					// It might be (tx, ty, tz).
	int arraysize = objval.get_array_size();
	if (arraysize >= 3 && objval.get_elem(0).get_int_value() < num_tiles)
		{
					// Qual is 4th if there.
		int qual = arraysize == 5 ? objval.get_elem(3).get_int_value()
							: -359;
					// Frame is 5th if there.
		int frnum = arraysize == 5 ? objval.get_elem(4).get_int_value()
							: -359;
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
	for (GOVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
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
	GOVector vec;			// Gets list.
	obj = obj->get_outermost();	// Might be inside something.
	int dist = distval.get_int_value();
	int shnum = shapeval.get_int_value();
					// Kludge for Test of Courage:
	if (cur_function->id == 0x70a && shnum == 0x9a && dist == 0)
		dist = 16;		// Mage may have wandered.
	obj->find_nearby(vec, shnum, dist, 0);
	Game_object *closest = 0;
	unsigned long bestdist = 100000;// Distance-squared in tiles.
	int x1, y1, z1;
	obj->get_abs_tile(x1, y1, z1);
	for (GOVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
		{
		Game_object *each = *it;
		int x2, y2, z2;
		each->get_abs_tile(x2, y2, z2);
		int dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
		unsigned long dist = dx*dx + dy*dy + dz*dz;
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
	Usecode_value& shapeval,	// Object shape to count (-359=any).
	Usecode_value& qualval,		// Quality (-359=any).
	Usecode_value& frameval		// Frame (-359=any).
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
	Usecode_value& shapeval,	// Object shape to get or -359 for any.
	Usecode_value& qualval,		// Quality (-359=any).
	Usecode_value& frameval		// Frame (-359=any).
	)
	{
	Game_object *obj = get_item(objval);
	if (!obj)
		return Usecode_value((Game_object*) NULL);
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	int qual = qualval.get_int_value();
	GOVector vec;			// Gets list.
	obj->get_objects(vec, shapenum, qual, framenum);

//	cout << "Container objects found:  " << cnt << << endl;
	Usecode_value within(vec.size(), 0);	// Create return array.
	int i = 0;
	for (GOVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
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
 *	Output:	None
 */

void Usecode_machine::add_cont_items
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
	if (obj) obj->add_quantity(quantity, shapenum, quality, framenum);
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
	int tx = gwin->get_scrolltx() + x/tilesize;
	int ty = gwin->get_scrollty() + y/tilesize;
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
	cout << hex << "[0x" << setfill((char)0x30) << setw(2)
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
	USECODE_INTRINSIC_PTR(get_random),	// 0
	USECODE_INTRINSIC_PTR(execute_usecode_array), // 1
	USECODE_INTRINSIC_PTR(delayed_execute_usecode_array), // 2
	USECODE_INTRINSIC_PTR(show_npc_face), // 3
	USECODE_INTRINSIC_PTR(remove_npc_face), // 4
	USECODE_INTRINSIC_PTR(add_answer), // 5
	USECODE_INTRINSIC_PTR(remove_answer), // 6
	USECODE_INTRINSIC_PTR(push_answers), // 7
	USECODE_INTRINSIC_PTR(pop_answers), // 8
	USECODE_INTRINSIC_PTR(clear_answers), // 9 +++++Testing. WJP
	USECODE_INTRINSIC_PTR(select_from_menu), // 0x0a
	USECODE_INTRINSIC_PTR(select_from_menu2), // 0x0b
	USECODE_INTRINSIC_PTR(input_numeric_value), // 0xc
	USECODE_INTRINSIC_PTR(set_item_shape), // 0xd
	USECODE_INTRINSIC_PTR(find_nearest), // 0xe
	USECODE_INTRINSIC_PTR(play_sound_effect), // 0xf - Sound effect
	USECODE_INTRINSIC_PTR(die_roll), // 0x10
	USECODE_INTRINSIC_PTR(get_item_shape), // 0x11
	USECODE_INTRINSIC_PTR(get_item_frame), // 0x12
	USECODE_INTRINSIC_PTR(set_item_frame), // 0x13
	USECODE_INTRINSIC_PTR(get_item_quality), // 0x14
	USECODE_INTRINSIC_PTR(set_item_quality), // 0x15
	USECODE_INTRINSIC_PTR(get_item_quantity), // 0x16
	USECODE_INTRINSIC_PTR(set_item_quantity), // 0x17
	USECODE_INTRINSIC_PTR(get_object_position), // 0x18
	USECODE_INTRINSIC_PTR(get_distance), // 0x19
	USECODE_INTRINSIC_PTR(find_direction), // 0x1a
	USECODE_INTRINSIC_PTR(get_npc_object), // 0x1b
	USECODE_INTRINSIC_PTR(get_schedule_type), // 0x1c
	USECODE_INTRINSIC_PTR(set_schedule_type), // 0x1d
	USECODE_INTRINSIC_PTR(add_to_party), // 0x1e
	USECODE_INTRINSIC_PTR(remove_from_party), // 0x1f
	USECODE_INTRINSIC_PTR(get_npc_prop), // 0x20
	USECODE_INTRINSIC_PTR(set_npc_prop), // 0x21
	USECODE_INTRINSIC_PTR(get_avatar_ref), // 0x22
	USECODE_INTRINSIC_PTR(get_party_list), // 0x23
	USECODE_INTRINSIC_PTR(create_new_object), // 0x24
	USECODE_INTRINSIC_PTR(set_last_created), // 0x25 
	USECODE_INTRINSIC_PTR(update_last_created), // 0x26
	USECODE_INTRINSIC_PTR(get_npc_name), // 0x27
	USECODE_INTRINSIC_PTR(count_objects), // 0x28
	USECODE_INTRINSIC_PTR(find_in_owner), // 0x29
	USECODE_INTRINSIC_PTR(get_cont_items), // 0x2a
	USECODE_INTRINSIC_PTR(remove_party_items), // 0x2b
	USECODE_INTRINSIC_PTR(add_party_items), // 0x2c
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0x2d UNUSED. - add_cont_items??? - GUESS
	USECODE_INTRINSIC_PTR(play_music), // 0x2e
	USECODE_INTRINSIC_PTR(npc_nearby), // 0x2f
	USECODE_INTRINSIC_PTR(find_nearby_avatar), // 0x30
	USECODE_INTRINSIC_PTR(is_npc),  // 0x31
	USECODE_INTRINSIC_PTR(display_runes), // 0x32
	USECODE_INTRINSIC_PTR(click_on_item), // 0x33
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0x34 UNUSED
	USECODE_INTRINSIC_PTR(find_nearby), // 0x35
	USECODE_INTRINSIC_PTR(give_last_created), // 0x36
	USECODE_INTRINSIC_PTR(is_dead), // 0x37
	USECODE_INTRINSIC_PTR(game_hour), // 0x38
	USECODE_INTRINSIC_PTR(game_minute), // 0x39
	USECODE_INTRINSIC_PTR(get_npc_number),	// 0x3a
	USECODE_INTRINSIC_PTR(part_of_day),	// 0x3b
	USECODE_INTRINSIC_PTR(get_alignment),	// 0x3c
	USECODE_INTRINSIC_PTR(set_alignment),	// 0x3d
	USECODE_INTRINSIC_PTR(move_object),	// 0x3e
	USECODE_INTRINSIC_PTR(remove_npc),	// 0x3f
	USECODE_INTRINSIC_PTR(item_say),	// 0x40
	USECODE_INTRINSIC_PTR(projectile_effect),	// 0x41
	USECODE_INTRINSIC_PTR(get_lift),	// 0x42
	USECODE_INTRINSIC_PTR(set_lift),	// 0x43
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x44++++Get_something() (0-3)
	// 3==can't do magic here?         GetWeather (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x45++++SetWeather(0-3) (ucdump.c)
	USECODE_INTRINSIC_PTR(sit_down),// 0x46
	USECODE_INTRINSIC_PTR(summon),	// 0x47     SummonCreature (ucdump.c)
	USECODE_INTRINSIC_PTR(display_map),	// 0x48
	USECODE_INTRINSIC_PTR(kill_npc),// 0x49
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4a ??(strength, egg-quality)
	USECODE_INTRINSIC_PTR(set_attack_mode),	// 0x4b
	USECODE_INTRINSIC_PTR(set_opponent),	// 0x4c
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4d     CloneNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4e UNUSED
	USECODE_INTRINSIC_PTR(display_area), // 0x4f// ShowCrystalBall(ucdump)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x50     ShowWizardEye (ucdump.c)
	USECODE_INTRINSIC_PTR(resurrect),// 0x51     ResurrectNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(add_spell),// 0x52     AddSpellToBook (ucdump.c)
	USECODE_INTRINSIC_PTR(sprite_effect),// 0x53 ExecuteSprite (ucdump.c)
	USECODE_INTRINSIC_PTR(explode),	// 0x54
	USECODE_INTRINSIC_PTR(book_mode),// 0x55
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x56 ++++Something to do with time.
                           // StopTime (ucdump.c)
	USECODE_INTRINSIC_PTR(cause_light),	// 0x57 CauseLight (ucdump.c)
	USECODE_INTRINSIC_PTR(get_barge),// 0x58
	USECODE_INTRINSIC_PTR(earthquake),	// 0x59
	USECODE_INTRINSIC_PTR(is_pc_female),	// 0x5a
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x5b     Armageddon (ucdump.c)
	USECODE_INTRINSIC_PTR(halt_scheduled),	// 0x5c
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x5d  +++++CauseBlackout (ucdump.c)
	USECODE_INTRINSIC_PTR(get_array_size),	// 0x5e
	USECODE_INTRINSIC_PTR(mark_virtue_stone),	// 0x5f
	USECODE_INTRINSIC_PTR(recall_virtue_stone),	// 0x60
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x61
	USECODE_INTRINSIC_PTR(is_pc_inside),	// 0x62
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x63     SetOrreryState (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x64     UNUSED
	USECODE_INTRINSIC_PTR(get_timer),	// 0x65
	USECODE_INTRINSIC_PTR(set_timer),	// 0x66
	USECODE_INTRINSIC_PTR(wearing_fellowship),	// 0x67
	USECODE_INTRINSIC_PTR(mouse_exists),	// 0x68
	USECODE_INTRINSIC_PTR(get_speech_track), // 0x69
	USECODE_INTRINSIC_PTR(flash_mouse),	// 0x6a
	USECODE_INTRINSIC_PTR(get_item_frame_rot),	// 0x6b Guessing
	USECODE_INTRINSIC_PTR(set_item_frame),	// 0x6c Guessing
	USECODE_INTRINSIC_PTR(okay_to_fly),	// 0x6d
	USECODE_INTRINSIC_PTR(get_container),	// 0x6e
	USECODE_INTRINSIC_PTR(remove_item),	// 0x6f
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x70
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x71
	USECODE_INTRINSIC_PTR(is_readied),	// 0x72
	USECODE_INTRINSIC_PTR(restart_game),	// 0x73
	USECODE_INTRINSIC_PTR(start_speech),	// 0x74
	USECODE_INTRINSIC_PTR(run_endgame),	// 0x75 StartEndGame (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x76     FireCannon (ucdump.c)
	USECODE_INTRINSIC_PTR(nap_time),	// 0x77
	USECODE_INTRINSIC_PTR(advance_time),	// 0x78
	USECODE_INTRINSIC_PTR(in_usecode),	// 0x79
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7a
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7b ++++Another sprite animation?
	USECODE_INTRINSIC_PTR(attack_avatar),	// 0x7c
	USECODE_INTRINSIC_PTR(path_run_usecode),	// 0x7d
	USECODE_INTRINSIC_PTR(close_gumps),	// 0x7e
	USECODE_INTRINSIC_PTR(item_say),	// 0x7f ItemSay in gump.
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x80 ++++Open_gump(item)???
	USECODE_INTRINSIC_PTR(in_gump_mode),	// 0x81
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x82
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x83
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	USECODE_INTRINSIC_PTR(is_not_blocked),	// 0x85
	USECODE_INTRINSIC_PTR(play_sound_effect),	// 0x86  +++++A sound??  Animation??
	USECODE_INTRINSIC_PTR(direction_from),	// 0x87
	USECODE_INTRINSIC_PTR(get_item_flag),	// 0x88
	USECODE_INTRINSIC_PTR(set_item_flag),	// 0x89
	USECODE_INTRINSIC_PTR(clear_item_flag),	// 0x8a
	USECODE_INTRINSIC_PTR(run_usecode),	// 0x8b 
	USECODE_INTRINSIC_PTR(fade_palette),	// 0x8c 
	USECODE_INTRINSIC_PTR(get_party_list2),	// 0x8d
	USECODE_INTRINSIC_PTR(in_combat),	// 0x8e
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x8f +++++Play speech/music?? Only
		//  called right before endgame.
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x90
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x91
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x92
	USECODE_INTRINSIC_PTR(get_dead_party),	// 0x93
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x94    SetupOrrery (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x95
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x96
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x97
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x98
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x99
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9a
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9b
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9c
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9d
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9e
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x9f
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaa
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xab
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xac
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xad
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xae
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xba
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbe
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xca
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xce
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xda
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xde
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xea
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xeb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xec
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xed
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xee
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xef
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xfa
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xfb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xfc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xfd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xfe
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xff
	};

// Serpent Isle Intrinsic Function Tablee
// It's different to the Black Gate one.
struct Usecode_machine::IntrinsicTableEntry
	  Usecode_machine::serpent_table[]=
	{
	USECODE_INTRINSIC_PTR(get_random),	// 0
	USECODE_INTRINSIC_PTR(execute_usecode_array), // 1
	USECODE_INTRINSIC_PTR(delayed_execute_usecode_array), // 2

	USECODE_INTRINSIC_PTR(UNKNOWN), // 3
	USECODE_INTRINSIC_PTR(UNKNOWN), // 4
	
	USECODE_INTRINSIC_PTR(show_npc_face), // 5
	USECODE_INTRINSIC_PTR(remove_npc_face), // 6

	USECODE_INTRINSIC_PTR(UNKNOWN), // 7
	USECODE_INTRINSIC_PTR(UNKNOWN), // 8
	USECODE_INTRINSIC_PTR(UNKNOWN), // 9
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0xa
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0xb

	USECODE_INTRINSIC_PTR(add_answer), // 0xc
	USECODE_INTRINSIC_PTR(remove_answer), // 0xd
	USECODE_INTRINSIC_PTR(push_answers), // 0xe
	USECODE_INTRINSIC_PTR(pop_answers), // 0xf
	USECODE_INTRINSIC_PTR(clear_answers), // 0x10

	USECODE_INTRINSIC_PTR(select_from_menu), // 0x11
	USECODE_INTRINSIC_PTR(select_from_menu2), // 0x12
	USECODE_INTRINSIC_PTR(input_numeric_value), // 0x13

	USECODE_INTRINSIC_PTR(set_item_shape), // 0x14

	USECODE_INTRINSIC_PTR(find_nearest), // 0x15
	USECODE_INTRINSIC_PTR(play_sound_effect), // 0x16
	
	USECODE_INTRINSIC_PTR(die_roll), // 0x17
	USECODE_INTRINSIC_PTR(get_item_shape), // appears to be get_item_shape for some odd reason
	
	USECODE_INTRINSIC_PTR(get_item_shape), // 0x19
	USECODE_INTRINSIC_PTR(get_item_frame), // 0x1a
	USECODE_INTRINSIC_PTR(set_item_frame), // 0x1b
	USECODE_INTRINSIC_PTR(get_item_quality), // 0x1c
	USECODE_INTRINSIC_PTR(set_item_quality), // 0x1d
	USECODE_INTRINSIC_PTR(get_item_quantity), // 0x1e
	USECODE_INTRINSIC_PTR(set_item_quantity), // 0x1f
	
	USECODE_INTRINSIC_PTR(get_object_position), // 0x20
	USECODE_INTRINSIC_PTR(get_distance), // 0x21
	USECODE_INTRINSIC_PTR(find_direction), // 0x22
	USECODE_INTRINSIC_PTR(get_npc_object), // 0x23
	USECODE_INTRINSIC_PTR(get_schedule_type), // 0x24
	USECODE_INTRINSIC_PTR(set_schedule_type), // 0x25
	USECODE_INTRINSIC_PTR(add_to_party), // 0x26
	USECODE_INTRINSIC_PTR(remove_from_party), // 0x27
	
	USECODE_INTRINSIC_PTR(get_npc_prop), // 0x28
	USECODE_INTRINSIC_PTR(set_npc_prop), // 0x29
	USECODE_INTRINSIC_PTR(get_avatar_ref), // 0x2a
	USECODE_INTRINSIC_PTR(get_party_list), // 0x2b
	
	USECODE_INTRINSIC_PTR(create_new_object), // 0x2c - Known
	USECODE_INTRINSIC_PTR(create_new_object), // 0x2d

	USECODE_INTRINSIC_PTR(set_last_created), // 0x2e 
	USECODE_INTRINSIC_PTR(update_last_created), // 0x2f - Known
	USECODE_INTRINSIC_PTR(get_npc_name), // 0x30
	USECODE_INTRINSIC_PTR(count_objects), // 0x31
	USECODE_INTRINSIC_PTR(find_in_owner), // 0x32
	USECODE_INTRINSIC_PTR(get_cont_items), // 0x33
	USECODE_INTRINSIC_PTR(remove_party_items), // 0x34
	USECODE_INTRINSIC_PTR(add_party_items), // 0x35
	USECODE_INTRINSIC_PTR(add_cont_items), // 0x36

	// Packing
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0x37
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0x38
	// End pack

	USECODE_INTRINSIC_PTR(play_music), // 0x39 - Known

	// Si Pack
	USECODE_INTRINSIC_PTR(npc_nearby), // 0x3c


	USECODE_INTRINSIC_PTR(npc_nearby), // 0x3a
	USECODE_INTRINSIC_PTR(find_nearby_avatar), // 0x3b

	USECODE_INTRINSIC_PTR(is_npc),  // 0x3d - Known
	USECODE_INTRINSIC_PTR(display_runes), // 0x3e
	USECODE_INTRINSIC_PTR(click_on_item), // 0x3f
	USECODE_INTRINSIC_PTR(UNKNOWN), // 0x40 UNUSED

	USECODE_INTRINSIC_PTR(find_nearby), // 0x41 - Known
	
	USECODE_INTRINSIC_PTR(give_last_created), // 0x42
	USECODE_INTRINSIC_PTR(is_dead), // 0x43
	USECODE_INTRINSIC_PTR(game_hour), // 0x44
	USECODE_INTRINSIC_PTR(game_minute), // 0x45
	
	USECODE_INTRINSIC_PTR(get_npc_number),	// 0x46

	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x47

	USECODE_INTRINSIC_PTR(part_of_day),	// 0x48
	USECODE_INTRINSIC_PTR(get_alignment),	// 0x49
	USECODE_INTRINSIC_PTR(set_alignment),	// 0x4a
	USECODE_INTRINSIC_PTR(move_object),	// 0x4b
	USECODE_INTRINSIC_PTR(remove_npc),	// 0x4c

	
	USECODE_INTRINSIC_PTR(item_say),	// 0x4d
	// Si Pack
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x51 Gets an Array??

	USECODE_INTRINSIC_PTR(projectile_effect),	// 0x4e
	USECODE_INTRINSIC_PTR(get_lift),	// 0x4f
	USECODE_INTRINSIC_PTR(set_lift),	// 0x50

	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x52++++Get_something() (0-3)
	// 3==can't do magic here?         GetWeather (ucdump.c)
	
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x53, set Weather

	USECODE_INTRINSIC_PTR(sit_down),// 0x54 - Known

	// Packing
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x55
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x56
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x57
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x58
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x59
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x5a
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x5b
	// End pack


	USECODE_INTRINSIC_PTR(summon),	// 0x5c     SummonCreature (ucdump.c)
	USECODE_INTRINSIC_PTR(display_map),	// 0x5d
	USECODE_INTRINSIC_PTR(kill_npc),// 0x5e
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x5f
	USECODE_INTRINSIC_PTR(set_attack_mode),	// 0x60
	USECODE_INTRINSIC_PTR(set_opponent),	// 0x61
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x62     CloneNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x63 UNUSED
	USECODE_INTRINSIC_PTR(display_area),	// 0x64 ++++called when you dbl-click
                         	// on FoV gem. (gift from LB) display area???
                                // ShowCrystalBall  (ucdump.c)

	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x65     ShowWizardEye (ucdump.c)
	USECODE_INTRINSIC_PTR(resurrect),// 0x66     ResurrectNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(add_spell),// 0x67     AddSpellToBook (ucdump.c)
	USECODE_INTRINSIC_PTR(sprite_effect),// 0x68 ExecuteSprite (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x69  ++++Explode???



	USECODE_INTRINSIC_PTR(book_mode),// 0x6a - Known
	
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x6b ++++Something to do with time.
                           // StopTime (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x6c ++++?Light_source(time)?
                           //CauseLight (ucdump.c)
	USECODE_INTRINSIC_PTR(get_barge),// 0x6d
	USECODE_INTRINSIC_PTR(earthquake),	// 0x6e
	USECODE_INTRINSIC_PTR(is_pc_female),	// 0x6f - Known
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x70     Armageddon (ucdump.c)
	USECODE_INTRINSIC_PTR(halt_scheduled),	// 0x71
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x72  +++++CauseBlackout (ucdump.c)
	USECODE_INTRINSIC_PTR(get_array_size),	// 0x73
	
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x74  ++++mark(virtue-stone)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x75  ++++recall(virtue-stone)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x76
	USECODE_INTRINSIC_PTR(is_pc_inside),	// 0x77
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x78     SetOrreryState (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x79     UNUSED
	USECODE_INTRINSIC_PTR(get_timer),	// 0x7a
	USECODE_INTRINSIC_PTR(set_timer),	// 0x7b
	USECODE_INTRINSIC_PTR(wearing_fellowship),	// 0x7c
	USECODE_INTRINSIC_PTR(mouse_exists),	// 0x7d
	USECODE_INTRINSIC_PTR(get_speech_track), // 0x7e
	USECODE_INTRINSIC_PTR(flash_mouse),	// 0x7f
	USECODE_INTRINSIC_PTR(get_item_frame),	// 0x80 Guessing++++
	USECODE_INTRINSIC_PTR(set_item_frame),	// 0x81 Guessing++++
	USECODE_INTRINSIC_PTR(okay_to_fly),	// 0x82
	USECODE_INTRINSIC_PTR(get_container),	// 0x83



	USECODE_INTRINSIC_PTR(remove_item),	// 0x84 - Known

	// Packing!!!
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	// End Pack

	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x70
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x71
	USECODE_INTRINSIC_PTR(is_readied),	// 0x72
	USECODE_INTRINSIC_PTR(restart_game),	// 0x73
	USECODE_INTRINSIC_PTR(start_speech),	// 0x74
	USECODE_INTRINSIC_PTR(run_endgame),	// 0x75 StartEndGame (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x76     FireCannon (ucdump.c)
	USECODE_INTRINSIC_PTR(nap_time),	// 0x77
	USECODE_INTRINSIC_PTR(advance_time),	// 0x78
	USECODE_INTRINSIC_PTR(in_usecode),	// 0x79
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7a
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7b ++++Another sprite animation?
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7c

	// Packing
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	// End Pack

	USECODE_INTRINSIC_PTR(path_run_usecode),	// 0x94 - Known
	
	USECODE_INTRINSIC_PTR(close_gumps),	// 0x7e
	USECODE_INTRINSIC_PTR(item_say),	// 0x7f ItemSay in gump.
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x80 ++++Open_gump(item)???
	USECODE_INTRINSIC_PTR(in_gump_mode),	// 0x81
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x82
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x83
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84

	// Packing!!!
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x84
	// End Pack

	USECODE_INTRINSIC_PTR(is_not_blocked),	// 0x85
	
	USECODE_INTRINSIC_PTR(play_sound_effect),	// 0xA1 - Known
	
	USECODE_INTRINSIC_PTR(direction_from),	// 0xa2
	
	USECODE_INTRINSIC_PTR(get_item_flag),	// 0xA3 - Known
	USECODE_INTRINSIC_PTR(set_item_flag),	// 0xA4 - Known
	USECODE_INTRINSIC_PTR(clear_item_flag),	// 0xA5 - Known
	
	USECODE_INTRINSIC_PTR(run_usecode),	// 0xa6 
	USECODE_INTRINSIC_PTR(fade_palette),	// 0xa7 
	USECODE_INTRINSIC_PTR(get_party_list2),	// 0xa8
	USECODE_INTRINSIC_PTR(in_combat),	// 0xa9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaa +++++Play speech/music?? Only
		//  called right before endgame.
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xab
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xac
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xad
	USECODE_INTRINSIC_PTR(get_dead_party),	// 0xae
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaf    SetupOrrery (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xba
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbb
	USECODE_INTRINSIC_PTR(get_npc_id),	// 0xbc - Known
	USECODE_INTRINSIC_PTR(set_npc_id),	// 0xbd - Known
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xa9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaa
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xab
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xac
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xad
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xae
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xaf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xb9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xba
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbe
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xbf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xc9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xca
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xce
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xcf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xd9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xda
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdc
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdd
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xde
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xdf
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe7
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe8
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xe9
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xea
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xeb
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xec
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xed
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xee
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xef
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf0
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf1
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf2
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf3
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf4
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf5
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf6
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0xf7
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
	gwin->clear_text_pending();
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
	if (!answers.answers.size())
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
//	cout << "Choose: ";		// TESTING.
//	for (int i = 0; i < answers.num_answers; i++)
//		cout << ' ' << answers.answers[i] << '(' << i << ") ";
	gwin->show_avatar_choices(answers.answers);
	int x, y;			// Get click.
	int choice_num;
	do
		{
		char chr;		// Allow '1', '2', etc.
		int result=Get_click(x, y, Mouse::hand, &chr);
		if (result<=0)		// Ignore ESC, and keep going.
			choice_num = -1;
		else if (chr >= '1' && chr <= '9')
			choice_num = chr - '1';
		else
			choice_num = gwin->conversation_choice(x, y);
		}
					// Wait for valid choice.
	while (choice_num  < 0 || choice_num >= (int)answers.answers.size());
					// Store ->answer string.
	user_choice = answers.answers[choice_num].c_str();
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
	    last_created(0), removed(new Deleted_objects()), user_choice(0),
	    String(0), stack(new Usecode_value[1024])
	{
	_init_(file);
	}

Usecode_machine::Usecode_machine
	(
	Game_window *gw
	) : gwin(gw), call_depth(0), cur_function(0), book(0), caller_item(0),
	    last_created(0), removed(new Deleted_objects()), user_choice(0),
	    String(0), stack(new Usecode_value[1024])
	{
	ifstream file;                // Read in usecode.
        U7open(file, USECODE);
	_init_(file);
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

	// This shouldn't be here!!!!!!!!!!
	// read();				// Read saved data (might be absent).
	
	file.seekg(0, ios::end);
	int size = file.tellg();	// Get file size.
	file.seekg(0);
#if 0
					// A slot for funs. n/256.
	funs = new vector<Usecode_function*>[16];
#endif
					// Read in all the functions.
	while (file.tellg() < size)
		{
		Usecode_function *fun = new Usecode_function(file);
#if 0
					// Get slot.
		Vector *slot = (Vector *) funs->get(fun->id/0x100);
		if (!slot)
			funs->put(fun->id/0x100, (slot = new Vector(10)));
					// Store in slot.
		slot->put(fun->id%0x100, fun);
#endif
		funs[fun->id/0x100].put(fun->id%0x100, fun);
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
	delete removed;
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
int debug = 0;				// 2 for more stuff.
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
#if DEBUG
	if (debug >= 0)
		printf("Running usecode %04x with event %d\n", fun->id, event);
#endif
					// Save/set function.
	Usecode_function *save_fun = cur_function;
	cur_function = fun;
	unsigned char *ip = fun->code;	// Instruction pointer.
					// Where it ends.
	unsigned char *endp = ip + fun->len;
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
	Usecode_value *save_sp = sp;	// NOW, save TOS, last-created.
	int num_externs = Read2(ip);	// # of external refs. following.
	unsigned char *externals = ip;	// Save -> to them.
	ip += 2*num_externs;		// ->actual bytecode.
	int offset;			// Gets offset parm.
	int sval;			// Get value from top-of-stack.
	unsigned char *code = ip;	// Save for debugging.
	int set_ret_value = 0;		// 1 when return value is set.
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
			printf("SP = %d, IP = %04x, op = %02x\n", sp - stack,
						curip, opcode);
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
			if (set_ret_value || !answers.answers.size() || abort)
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
					sprintf(buf, "%ld%s", 
						v1.get_int_value(),
						v2.get_str_value());
					sum = Usecode_value(buf);
					}
				}
			else if (v1.get_type() == Usecode_value::string_type)
				{
				if (v2.get_type() == Usecode_value::int_type)
					{
					sprintf(buf, "%s%ld", 
							v1.get_str_value(),
							v2.get_int_value());
					sum = Usecode_value(buf);
					}
				else if (v2.get_type() == 
						Usecode_value::string_type)
					{
					sprintf(buf, "%s%s", 
							v1.get_str_value(),
							v2.get_str_value());
					sum = Usecode_value(buf);
					}
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
			sval = popi();
			pushi(popi() != sval);
			break;
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
#if 0
					// But 1st takes precedence.
			if (!set_ret_value)
				ret_value = pop();
			set_ret_value = 1;
			break;
#else	/* +++++Try this: */
			{
			Usecode_value r = pop();
					// But 1st takes precedence.
			if (!set_ret_value)
				ret_value = r;
			set_ret_value = 1;
			break;
			}
#endif
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
				sprintf(buf, "%ld",
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
		printf("RETurning from usecode %04x\n", fun->id);
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
					// Nothing going on?
	if (!call_depth && !Scheduled_usecode::get_count())
		removed->flush();	// Flush removed objects.
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
	if (call_depth && event == npc_proximity)
		return (0);
	Game_object *prev_item = caller_item;
	caller_item = obj;
	answers.clear();
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
