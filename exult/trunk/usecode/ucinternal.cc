/*
 *	ucinternal.cc - Interpreter for usecode.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
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
#include <algorithm>       // STL function things

#include "Gump.h"
#include "Gump_manager.h"
#include "Text_gump.h"
#include "Audio.h"
#include "animate.h"
#include "barge.h"
#include "chunks.h"
#include "conversation.h"
#include "exult.h"
#include "game.h"
#include "gamewin.h"
#include "keyring.h"
#include "mouse.h"
#include "schedule.h"
#include "tqueue.h"
#include "ucinternal.h"
#include "ucsched.h"
#include "useval.h"
#include "utils.h"
#include "vec.h"
#include "actors.h"
#include "egg.h"
#include "actions.h"

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

extern bool intrinsic_trace,usecode_debugging;
extern int usecode_trace;

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

void Usecode_internal::append_string
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

// Push/pop stack.
inline void Usecode_internal::push(Usecode_value& val)
{
	*sp++ = val;
}

inline Usecode_value Usecode_internal::pop()
{ 
	if (sp <= stack)
		{		// Happens in SI #0x939
			cerr << "Stack underflow" << endl;
			return Usecode_value(0);
		}
	return *--sp; 
}

inline void Usecode_internal::pushref(Game_object *obj)
{
	Usecode_value v(obj);
	push(v);
} 

inline void Usecode_internal::pushi(long val)		// Push/pop integers.
{
	Usecode_value v(val);
	push(v);
}

inline int Usecode_internal::popi()
{
	Usecode_value val = pop();
	return val.need_int_value();
}

// Push/pop strings.
inline void Usecode_internal::pushs(char *s)
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

Game_object *Usecode_internal::get_item
	(
	Usecode_value& itemref
	)
	{
					// If array, take 1st element.
	Usecode_value& elemval = itemref.get_elem0();

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
	else if (val >= 0 && val < gwin->get_num_npcs()) {
		obj = gwin->get_npc(val);
		CERR("Warning: interpreting positive integer as NPCnum");
					// Special case:  palace guards.
	} else if (val >= 0 && val < 0x400)		// Looks like a shape #?
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

Actor *Usecode_internal::as_actor
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

Npc_actor *Usecode_internal::as_npcactor
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

Tile_coord Usecode_internal::get_position
	(
	Usecode_value& itemval
	)
	{
	Game_object *obj;		// An object?
	if ((itemval.get_array_size() == 1 || !itemval.get_array_size()) && 
						(obj = get_item(itemval)))
			return obj->get_outermost()->get_tile();
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
		return caller_item->get_tile();
	}

/*
 *	Make sure pending text has been seen.
 */

void Usecode_internal::show_pending_text
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

void Usecode_internal::show_book
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

void Usecode_internal::say_string
	(
	)
	{
		//	user_choice = 0;		// Clear user's response.
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
		if (*str == '*')	// Just gets an extra click.
			{
			click_to_continue();
			str++;
			continue;
			}
		char *eol = strchr(str, '~');
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

void Usecode_internal::stack_error
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

void Usecode_internal::show_npc_face
	(
	Usecode_value& arg1,		// Shape (NPC #).
	Usecode_value& arg2,		// Frame.
	int slot			// 0, 1, or -1 to find free spot.
	)
	{
	show_pending_text();
	Actor *npc = as_actor(get_item(arg1));
	if (!npc)
		return;
	int shape = npc->get_face_shapenum();
	int frame = arg2.get_int_value();
	if (Game::get_game_type() == BLACK_GATE)
		{
		if (npc->get_npc_num() != -1) 
			npc->set_flag (Obj_flags::met);
		}
	else if (Game::get_game_type() == SERPENT_ISLE)
		{			// Special case: Nightmare Smith.
		if (npc->get_npc_num() == 296)
			shape = -1;
		}
	if (!conv->get_num_faces_on_screen())
		gwin->remove_text_effects();
	// Only non persitent
	if (gwin->get_gump_man()->showing_gumps(true))
		{
		gwin->get_gump_man()->close_all_gumps();
		gwin->set_all_dirty();
		init_conversation();	// jsf-Added 4/20/01 for SI-Lydia.
		}
	gwin->paint_dirty();
	conv->show_face(shape, frame, slot);
//	user_choice = 0;		// Seems like a good idea.
// Also seems to create a conversation bug in Test of Love :-(

	}

/*
 *	Remove an NPC's face.
 */

void Usecode_internal::remove_npc_face
	(
	Usecode_value& arg1		// Shape (NPC #).
	)
	{
	show_pending_text();
	Actor *npc = as_actor(get_item(arg1));
	if (!npc)
		return;
	int shape = npc->get_face_shapenum();
	conv->remove_face(shape);
	}

/*
 *	Set an item's shape.
 */

void Usecode_internal::set_item_shape
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
			Gump *gump = gwin->get_gump_man()->find_gump(item);
			if (gump)
				gump->paint(gwin);
			}
		return;
		}
					// Figure area to repaint.
//	Rectangle rect = gwin->get_shape_rect(item);
	gwin->add_dirty(item);
					// Get chunk it's in.
	Map_chunk *chunk = gwin->get_chunk(item);
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
 */

void Usecode_internal::set_item_frame
	(
	Game_object *item,
	int frame,
	int check_empty,		// If 1, don't set empty frame.
	int set_rotated			// Set 'rotated' bit to one in 'frame'.
	)
	{
	if (!item)
		return;
					// +++Added 9/16/2001:
	if (!set_rotated)		// Leave bit alone?
		frame = (item->get_framenum()&32)|(frame&31);
	if (frame == item->get_framenum())
		return;			// Already set to that.
					// Check for empty frame.
	ShapeID sid(item->get_shapenum(), frame, item->get_shapefile());
	Shape_frame *shape = sid.get_shape();
	if (!shape || (check_empty && shape->is_empty()))
		return;
	// cout << "Set_item_frame: " << item->get_shapenum() 
	//				<< ", " << frame << endl;
					// (Don't mess up rotated frames.)
	if ((frame&0xf) < item->get_num_frames())
		{
		if (item->get_owner())	// Inside a container?
			{
			item->set_frame(frame);
			Gump *gump = gwin->get_gump_man()->find_gump(item);
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
	}

/*
 *	Remove an item from the world.
 */

void Usecode_internal::remove_item
	(
	Game_object *obj
	)
	{
	if (!obj)
		return;
	if (!last_created.empty() && obj == last_created.back())
		last_created.pop_back();
	if (obj->get_owner())		// Inside a container?
		{			// Paint gump if open.
		Gump *gump = gwin->get_gump_man()->find_gump(obj);
		if (gump)
			gump->paint(gwin);
		}
	else
		gwin->add_dirty(obj);

	obj->remove_this();
	}

#define PARTY_MAX (sizeof(party)/sizeof(party[0]))

/*
 *	Is an NPC in the party?
 */

int Usecode_internal::npc_in_party
	(
	Game_object *npc
	)
	{
	return (npc && npc->get_party_id() >= 0);
	}

/*
 *	Return an array containing the party, with the Avatar first.
 */

Usecode_value Usecode_internal::get_party
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
 *	Put text near an item.
 */

void Usecode_internal::item_say
	(
	Usecode_value& objval,
	Usecode_value& strval
	)
	{
	Game_object *obj = get_item(objval);
	const char *str = strval.get_str_value();
	if (obj && str && *str)
		{
					// Added Nov01,01 to fix 'locate':
		gwin->remove_text_effect(obj);
		gwin->add_text(str, obj);
		}
	}

/*
 *	Activate all cached-in usecode eggs near a given spot.
 */

void Usecode_internal::activate_cached
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
 *	For sorting up-to-down, right-to-left, and near-to-far:
 */
class Object_reverse_sorter
	{
public:
	bool operator()(const Game_object *o1, const Game_object *o2)
		{
		Tile_coord t1 = o1->get_tile(),
			   t2 = o2->get_tile();
		if (t1.ty > t2.ty)
			return true;
		else if (t1.ty == t2.ty)
			{
			if (t1.tx > t2.tx)
				return true;
			else 
				return t1.tx == t2.tx && t1.tz > t2.tz;
			}
		else
			return false;
		}
	};

/*
 *	Return an array of nearby objects.
 */

Usecode_value Usecode_internal::find_nearby
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

	int shapenum;

	if (shapeval.is_array()) {
		// fixes 'lightning whip sacrifice' in Silver Seed
		shapenum = shapeval.get_elem(0).get_int_value();
		if (shapeval.get_array_size() > 1)
			cerr << "Calling find_nearby with an array > 1 !!!!"
				 << endl;
	} else
		shapenum = shapeval.get_int_value();

		
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
			shapenum,
			distval.get_int_value(), mval.get_int_value(), 
			qual, frnum);
		}
	else
		{
		Game_object *obj = get_item(objval);
		if (!obj)
			return Usecode_value(0, 0);
		obj = obj->get_outermost();	// Might be inside something.
		obj->find_nearby(vec, shapenum,
			distval.get_int_value(), mval.get_int_value());
		}
	if (vec.size() > 1)		// Sort right-left, near-far to fix
					//   SI/SS cask bug.
		std::sort(vec.begin(), vec.end(), Object_reverse_sorter());
	Usecode_value nearby(vec.size(), 0);	// Create return array.
	int i = 0;
	for (Game_object_vector::const_iterator it = vec.begin(); 
						it != vec.end(); ++it)
		{
		Game_object *each = *it;
		Usecode_value val(each);
		nearby.put_elem(i++, val);
		}
	return (nearby);
	}

/*
 *	Look for a barge that an object is a part of, or on, using the same
 *	sort (right-left, front-back) as ::find_nearby().  If there are more
 *	than one beneath 'obj', the highest is returned.
 *
 *	Output:	->barge if found, else 0.
 */

Barge_object *Get_barge
	(
	Game_object *obj
	)
	{
					// Check object itself.
	Barge_object *barge = dynamic_cast<Barge_object *> (obj);
	if (barge)
		return barge;
	Game_object_vector vec;		// Find it within 20 tiles (egglike).
	obj->find_nearby(vec, 961, 20, 0x10);
	if (vec.size() > 1)		// Sort right-left, near-far.
		std::sort(vec.begin(), vec.end(), Object_reverse_sorter());
					// Object must be inside it.
	Tile_coord pos = obj->get_tile();
	Barge_object *best = 0;
	for (Game_object_vector::const_iterator it = vec.begin();
							it != vec.end(); it++)
		{
		barge = dynamic_cast<Barge_object *> (*it);
		if (barge && barge->get_tile_footprint().has_point(
							pos.tx, pos.ty))
			{
			int lift = barge->get_lift();
			if (!best || 	// First qualifying?
					// First beneath obj.?
			    (best->get_lift() > pos.tz && lift <= pos.tz) ||
					// Highest beneath?
			    (lift <= pos.tz && lift > best->get_lift()))
				best = barge;
			}
		}
	return best;
	}

/*
 *	Return object of given shape nearest given obj.
 */

Usecode_value Usecode_internal::find_nearest
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
	Tile_coord t1 = obj->get_tile();
	for (Game_object_vector::const_iterator it = vec.begin(); 
							it != vec.end(); ++it)
		{
		Game_object *each = *it;
		Tile_coord t2 = each->get_tile();
		int dx = t1.tx - t2.tx, dy = t1.ty - t2.ty, dz = t1.tz - t2.tz;
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

Usecode_value Usecode_internal::find_direction
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

Usecode_value Usecode_internal::count_objects
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

Usecode_value Usecode_internal::get_objects
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
 *	Output:	1 (or the object) if successful, else 0.
 */

Usecode_value Usecode_internal::remove_party_items
	(
	Usecode_value& quantval,	// Quantity to remove.
	Usecode_value& shapeval,	// Shape.
	Usecode_value& qualval,		// Quality??
	Usecode_value& frameval,	// Frame.
	Usecode_value& flagval		// Flag??
	)
	{
	int quantity = quantval.need_int_value();
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	int quality = qualval.get_int_value();
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	if (quantity == -359 && Game::get_game_type() == SERPENT_ISLE)
	{				// Special case. (Check party.)
		Game_object *obj = 0;
		for (int i = 0; i < cnt && !obj; i++)
			{
			Game_object *actor = get_item(party.get_elem(i));
			if (actor)
				obj = actor->find_item(shapenum, quality, 
								framenum);
			}
		if (!obj)
			return Usecode_value(0);

		// +++++++++ problem: we need to really delete this object, but
		// it also has to remain long enough to be processed by the
		// calling usecode function...
		// for now: use temp_to_be_deleted to store the object and
		// delete it afterwards (end of Usecode_internal::run)
		temp_to_be_deleted = obj;
		obj->remove_this(1);
		return Usecode_value(obj);
	}
	Usecode_value all(-357);	// See if they exist.
	Usecode_value avail = count_objects(all, shapeval, qualval, frameval);
	if (avail.get_int_value() < quantity)
		return Usecode_value(0);
					// Look through whole party.
	for (int i = 0; i < cnt && quantity > 0; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (obj)
			quantity = obj->remove_quantity(quantity, shapenum,
							quality, framenum);
		}
	return Usecode_value(quantity == 0);
	}

/*
 *	Add a quantity of an item to the party.
 *
 *	Output:	List of members who got objects.
 */

Usecode_value Usecode_internal::add_party_items
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
	Usecode_value result(0, 0);	// Start with empty array.
	for (int i = 0; i < cnt && quantity > 0; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (!obj)
			continue;
		int prev = quantity;
		quantity = obj->add_quantity(quantity, shapenum,
							quality, framenum);
		if (quantity < prev)	// Added to this NPC.
			result.concat(party.get_elem(i));
		}
	if (GAME_BG)			// Black gate?  Just return result.
		return result;
	int todo = quantity;		// SI:  Put remaining on the ground.
	if (framenum == c_any_framenum)
		framenum = 0;
	while (todo > 0)
		{
		Tile_coord pos = Map_chunk::find_spot(
			gwin->get_main_actor()->get_tile(), 3,
				shapenum, framenum, 2);
		if (pos.tx == -1)	// Hope this rarely happens.
			break;
		Shape_info& info = gwin->get_info(shapenum);
					// Create and place.
		Game_object *newobj = gwin->create_ireg_object(
					info, shapenum, framenum, 0, 0, 0);
		newobj->set_flag(Obj_flags::okay_to_take);
		newobj->move(pos);
		todo--;
		if (todo > 0)		// Create quantity if possible.
			todo = newobj->modify_quantity(todo);
		}
					// SI?  Append # left on ground.
	Usecode_value ground(quantity - todo);
	result.concat(ground);
	return result;
	}

/*
 *	Add a quantity of an item to a container
 *
 *	Output:	Num created
 */

Usecode_value Usecode_internal::add_cont_items
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

Usecode_value Usecode_internal::remove_cont_items
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

#if 0	/* ++++Old way */
/*
 *	Find an unblocked tile within a distance of 3 from a given point.
 *
 *	Output:	Returns tile, or original if not found.
 */

static Tile_coord Find_unblocked
	(
	Tile_coord dest,		// Where to go.
	int ht,				// Height of obj. that needs space.
	int src_tz			// Current lift.
	)
	{
	Tile_coord start = dest, startz = dest;
	startz.tz = src_tz;		// This one is at source height.
	dest.tx = -1;
	for (int i = 0; dest.tx == -1 && i < 3; i++)
		{			// First at given level.
		dest = Game_object::find_unblocked_tile(start, i, ht);
		if (dest.tx == -1)	// Then at source level.
			dest = Game_object::find_unblocked_tile(startz, i, ht);
		}
	if (dest.tx == -1)
		return start;
	else
		return dest;
	}
#endif

/*
 *	Have an NPC walk somewhere and then execute usecode.
 *
 *	Output:	1 if successful, else 0.
 */

int Usecode_internal::path_run_usecode
	(
	Usecode_value& npcval,		// # or ref.
	Usecode_value& locval,		// Where to walk to.
	Usecode_value& useval,		// Usecode #.
	Usecode_value& itemval,		// Use as itemref in Usecode fun.
	Usecode_value& eventval,	// Eventid.
	int find_free,			// Not sure.  For SI.  
	int always			// Always run function, even if failed.
	)
	{
	Actor *npc = as_actor(get_item(npcval));
	if (!npc)
		return 0;
	path_npc = npc;
	int usefun = useval.get_elem0().get_int_value();
	Game_object *obj = get_item(itemval);
	int sz = locval.get_array_size();
	if (!npc || sz < 2)
		{
		CERR("Path_run_usecode: bad inputs");
		return 0;
		}
	Tile_coord src = npc->get_tile();
	Tile_coord dest(locval.get_elem(0).get_int_value(),
			locval.get_elem(1).get_int_value(),
			sz == 3 ? locval.get_elem(2).get_int_value() : 0);
	if (dest.tz < 0)		// ++++Don't understand this.
		dest.tz = 0;
	if (find_free)
		{
#if 1	/* Now works with SI lightning platform */
					// Allow rise of 3 (for SI lightning).
		Tile_coord d = Map_chunk::find_spot(dest, 3, npc, 3);
		if (d.tx == -1)		// No?  Try at source level.
			d = Map_chunk::find_spot(
				Tile_coord(dest.tx, dest.ty, src.tz), 3, npc,
									0);
		if (d.tx != -1)		// Found it?
			dest = d;
#else	/* ++++Old way */
		int ht = gwin->get_info(npc).get_3d_height();
		dest = Find_unblocked(dest, ht, src.tz);
#endif
		if (usefun == 0x60a &&	// ++++Added 7/21/01 to fix Iron
		    src.distance(dest) <= 1)
			return 1;	// Maiden loop in SI.  Kludge+++++++
		}
	if (!obj)			// Just skip the usecode part.
		return npc->walk_path_to_tile(dest, gwin->get_std_delay(), 0);
					// Walk there and execute.
	If_else_path_actor_action *action = 
		new If_else_path_actor_action(npc, dest,
				new Usecode_actor_action(usefun, obj, 
						eventval.get_int_value()));
	if (always)			// Set failure to same thing.
		action->set_failure(
				new Usecode_actor_action(usefun, obj, 
						eventval.get_int_value()));
	npc->set_action(action);	// Get into time queue.
	npc->start(gwin->get_std_delay(), 0);
	return !action->done_and_failed();
}

/*
 *	Schedule a script.
 */

void Usecode_internal::create_script
	(
	Usecode_value& objval,
	Usecode_value& codeval,
	long delay			// Delay from current time.
	)
	{
	Game_object *obj = get_item(objval);
					// Pure kludge for SI wells:
	if (objval.get_array_size() == 2 && 
	    Game::get_game_type() == SERPENT_ISLE &&
	    obj && obj->get_shapenum() == 470 && obj->get_lift() == 0)
		{			// We want the TOP of the well.
		Usecode_value v2 = objval.get_elem(1);
		Game_object *o2 = get_item(v2);
		if (o2->get_shapenum() == obj->get_shapenum() && 
		    o2->get_lift() == 2)
			{
			objval = v2;
			obj = o2;
			}
		}
	if (!obj)
		{
		cerr << "Can't create script for NULL object" << endl;
		return;
		}
					// ++++Better to 'steal' array; this
					//   ends up making a copy.
	Usecode_value *code = new Usecode_value(codeval);
	Usecode_script *script = new Usecode_script(obj, code);
	script->start(delay);
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

Usecode_value Usecode_internal::Execute_Intrinsic(UsecodeIntrinsicFn func,const char *name,int event,int intrinsic,int num_parms,Usecode_value parms[12])
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


typedef	Usecode_value (Usecode_internal::*UsecodeIntrinsicFn)(int event,int intrinsic,int num_parms,Usecode_value parms[12]);

// missing from mingw32 header files, so included manually
#ifndef __STRING
#if defined __STDC__ && __STDC__
#define __STRING(x) #x
#else
#define __STRING(x) "x"
#endif
#endif

#define	USECODE_INTRINSIC_PTR(NAME)	{ &Usecode_internal::UI_ ## NAME, __STRING(NAME) }

struct Usecode_internal::IntrinsicTableEntry
	  Usecode_internal::intrinsic_table[]=
	{
#include "bgintrinsics.h"
	};

// Serpent Isle Intrinsic Function Tablee
// It's different to the Black Gate one.
struct Usecode_internal::IntrinsicTableEntry
	  Usecode_internal::serpent_table[]=
	{
#include "siintrinsics.h"
	};


int	max_bundled_intrinsics=0xff;	// Index of the last intrinsic in this table
/*
 *	Call an intrinsic function.
 */

Usecode_value Usecode_internal::call_intrinsic
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
		struct Usecode_internal::IntrinsicTableEntry *table_entry;
		
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

void Usecode_internal::click_to_continue
	(
	)
	{
	int xx, yy;
	char c;
	if (!gwin->is_palette_faded_out())// If black screen, skip!
		Get_click(xx, yy, Mouse::hand, &c);
	conv->clear_text_pending();
	//	user_choice = 0;		// Clear it.
	}

/*
 *	Set book/scroll to display.
 */

void Usecode_internal::set_book
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

const char *Usecode_internal::get_user_choice
	(
	)
	{
	if (!conv->get_num_answers())
		return (0);		// This does happen (Emps-honey).

	//	if (!user_choice)		// May have already been done.
	// (breaks conversation with Cyclops on Dagger Isle ('foul magic' option))

	get_user_choice_num();
	return (user_choice);
	}

/*
 *	Get user's choice from among the possible responses.
 *
 *	Output:	User choice is set, with choice # returned.
 *		-1 if no possible choices.
 */

int Usecode_internal::get_user_choice_num
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
 *	Create for the outside world.
 */

Usecode_machine *Usecode_machine::create
	(
	Game_window *gw
	)
	{
	return new Usecode_internal(gw);
	}

/*
 *	Create machine from a 'usecode' file.
 */

Usecode_internal::Usecode_internal
	(
	Game_window *gw
	) : Usecode_machine(gw), cur_function(0),
	    book(0), caller_item(0),
	    path_npc(0), user_choice(0), 
	    saved_pos(-1, -1, -1),
	    String(0), stack(new Usecode_value[1024]), intercept_item(0),
		temp_to_be_deleted(0), telekenesis_fun(-1)
	{
					// Clear timers.
	memset((char *) &timers[0], 0, sizeof(timers));
	sp = stack;
	ifstream file;                // Read in usecode.
	try
		{
	        U7open(file, USECODE);
		read_usecode(file);
		file.close();
		}
	catch(const file_exception & f)
		{
		if (!Game::is_editing())	// Ok if map-editing.
			throw f;
		std::cerr << "Warning (map-editing): Couldn't open '" << 
							USECODE << "'" << endl;
		}

					// Get custom usecode functions.
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_USECODE))
		{
		U7open(file, PATCH_USECODE);
		read_usecode(file);
		file.close();
		}
	}

/*
 *	Read in usecode functions.  These may override previously read
 *	functions.
 */

void Usecode_internal::read_usecode
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

Usecode_internal::~Usecode_internal
	(
	)
	{
	delete [] stack;
	delete [] String;
	int num_slots = sizeof(funs)/sizeof(funs[0]);
	for (int i = 0; i < num_slots; i++)
		{
		vector<Usecode_function*>& slot = funs[i];
		int cnt = slot.size();
		for (int j = 0; j < cnt; j++)
			delete slot[j];
		}
	delete book;
	}

#ifdef DEBUG
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

int Usecode_internal::run
	(
	Usecode_function *fun,
	int event,			// Event (??) that caused this call.
	int stack_elems			// # elems. on stack at call.
	)
	{
	call_depth++;
	int abort = 0;			// Flag if ABRT executed.
#if 0
	unsigned char *catch_ip = 0;	// IP for catching an ABRT.
#endif
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
#ifdef DEBUG
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
	bool set_ret_value = false;		// return value set?
	Usecode_value ret_value;	// Gets return value.
	/*
	 *	Main loop.
	 */
	while (ip < endp)
		{
		int opcode = *ip++;
#ifdef DEBUG
		if (usecode_trace)
			{

			int curip = ip - 1 - code;

			if (usecode_trace == 2) {
				uc_trace_disasm(locals, num_locals, (uint8*)data, 
								(uint8*)externals, (uint8*)code, ip-1);
			} else {
				cout << "SP = " << sp - stack << ", IP = " << hex << curip
					 << ", op = "<< opcode << dec << endl;
			}

			if (ucbp_fun == fun->id && ucbp_ip == curip)
				cout << "At breakpoint" << endl;
				
			cout.flush();
			}
#endif
		switch (opcode)
			{
		case 0x04:  // start conversation
			{
			offset = (short) Read2(ip);
			found_answer = false;
			if (!get_user_choice())	// Exit conv. if no choices.
				ip += offset;	// (Emp's and honey.)
			break;
			}
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
		case 0x07:		// CMPS.
			{
				int cnt = Read2(ip);	// # strings.
				offset = (short) Read2(ip);
				bool matched = false;

				// only try to match if we haven't found an answer yet
				while (!matched && !found_answer && cnt-- > 0) {
					Usecode_value s = pop();
					const char *str = s.get_str_value();
					if (str && strcmp(str, user_choice) == 0) {
						matched = true;
						found_answer = true;
					}
				}
				while (cnt-- > 0)	// Pop rest of stack.
					pop();
				if (!matched)		// Jump if no match.
					ip += offset;
			}
			break;
		case 0x09:		// ADD.
			{
			Usecode_value v2 = pop();
			Usecode_value v1 = pop();
			Usecode_value sum = v1 + v2;
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
							" out of range" << endl;
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
			int num = Read2(ip);
			int cnt = num;
			Usecode_value arr(num, 0);
			int to = 0;	// Store at this index.
			while (cnt--)
				{
				Usecode_value val = pop();
				to += arr.add_values(to, val);
				}
			if (to < num)// 1 or more vals empty arrays?
				arr.resize(to);
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
			{
			offset = Read2(ip);
			int result = call_usecode_function(externals[2*offset] + 
							256*externals[2*offset + 1], event,
							sp - save_sp);
#ifdef DEBUG
			// This follows the "RETurning (retvalue) from usecode xxxx"
			cout << "...back into usecode " << hex << setw(4) << 
				setfill((char)0x30) << fun->id << dec << setfill(' ') << endl;
#endif
			if (!result)
				{	// Catch ABRT.
				abort = 1;
#if 0
				if (catch_ip)
					ip = catch_ip;
				else	// No catch?  I think we should exit.
#endif
					{
					sp = save_sp;
					ip = endp;
					}
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
			if (offset < 0 || offset >= num_locals)
				{
				cerr << "Local #" << offset << 
							" out of range" << endl;
				pushi(0);
				break;
				}
			if (sval < 0) {
				cerr << "Negative array index: " << sval << endl;
				pushi(0);
				break;
			}
			Usecode_value& val = locals[offset];

			if (val.is_array()) {
				push(val.get_elem(sval));
			} else if (sval == 0) {
				push(val); // needed for SS keyring (among others, probably)
			} else {
				pushi(0);  // guessing... probably unnecessary
			}
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

			//			if (!set_ret_value)

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
			set_ret_value = true;
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
// 25-09-2001 - Changed to >= 0 to fix money-counting in SI.
//				if (locals[offset].get_int_value() != 0) {
				if (locals[offset].get_int_value() >= 0) {
					char buf[20];
					snprintf(buf, 20, "%ld",
					locals[offset].get_int_value());
					append_string(buf);
				}
				}
			break;
			}
		case 0x30:		// IN.  Is a val. in an array?
			{
			Usecode_value arr = pop();
					// If an array, use 1st elem.
			Usecode_value val = pop().get_elem0();
			pushi(arr.find_elem(val) >= 0);
			break;
			}
		case 0x31:		// Unknown.
			// this opcode only occurs in the 'audition' usecode function (BG)
			// not sure what it's supposed to do, but this function results
			// in the same behaviour as the original
			ip += 2;
			offset = (short)Read2(ip);
			if (!found_answer)
				found_answer = true;
			else
				ip += offset;
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
		case 0x40:		// end conversation
			found_answer = true;
			break;
		case 0x42:		// PUSHF.
			offset = Read2(ip);
			pushi(gflags[offset]);
			break;
		case 0x43:		// POPF.
			offset = Read2(ip);
			gflags[offset] = (unsigned char) popi();
					// ++++KLUDGE for Monk Isle:
			if (offset == 0x272 && Game::get_game_type() ==
							SERPENT_ISLE)
				gflags[offset] = 0;
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
#if 0
				if (catch_ip)
					ip = catch_ip;
				else	// No catch?  I think we should exit.
#endif
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
		case 0x4c: // debugging stuff from spanish SI (linenum)
			{
				int linenum = Read2(ip);
				break;
			}
		case 0x4d: // debugging stuff from spanish SI (function init)
			{
				int funcname = Read2(ip);
				int paramnames = Read2(ip);
				break;
			}
		default:
			cout << "Opcode " << opcode << " not known." << endl;
			break;
			}
		}
	delete [] locals;
#ifdef DEBUG
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
	if (call_depth == 0 && temp_to_be_deleted) {
		temp_to_be_deleted->remove_this(0);
		temp_to_be_deleted = 0;
	}
	return (abort == 0);		// Return 0 if ABRT.
	}

/*
 *	Call a usecode function.
 *
 *	Output:	0 if ABRT executed.
 *		-1 if function not found.
 *		1 if okay.
 */

int Usecode_internal::call_usecode_function
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

int Usecode_internal::call_usecode
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
					// Left hanging (BG)?
	if (conv->get_num_faces_on_screen() > 0)
		{
		conv->init_faces();	// Remove them.
		gwin->set_all_dirty();	// Force repaint.
		}
	return ret;
	}

/*
 *	Add NPC to party.
 *
 *	Output:	false if no room or already a member.
 */

bool Usecode_internal::add_to_party
	(
	Actor *npc			// (Should not be the Avatar.)
	)
	{
	const int maxparty = sizeof(party)/sizeof(party[0]);
	if (!npc || party_count == maxparty || npc->get_party_id() >= 0)
		return false;
	remove_from_dead_party(npc);	// Just to be sure.
	npc->set_party_id(party_count);
	npc->set_flag (Obj_flags::in_party);
					// We can take items.
	npc->set_flag_recursively(Obj_flags::okay_to_take);
	party[party_count++] = npc->get_npc_num();
	return true;
	}

/*
 *	Remove party member.
 *
 *	Output:	false if not found.
 */

bool Usecode_internal::remove_from_party
	(
	Actor *npc
	)
	{
	if (!npc)
		return false;
	int id = npc->get_party_id();
	if (id == -1)			// Not in party?
		return false;
	int npc_num = npc->get_npc_num();
	if (party[id] != npc_num)
		{
		cout << "Party mismatch!!" << endl;
		return false;
		}
					// Shift the rest down.
	for (int i = id + 1; i < party_count; i++)
		{
		Actor *npc2 = gwin->get_npc(party[i]);
		if (npc2)
			npc2->set_party_id(i - 1);
		party[i - 1] = party[i];
		}
	npc->clear_flag (Obj_flags::in_party);
	party_count--;
	party[party_count] = 0;
	npc->set_party_id(-1);
	return true;
	}

/*
 *	Find index of NPC in dead party list.
 *
 *	Output:	Index, or -1 if not found.
 */

int Usecode_internal::in_dead_party
	(
	Actor *npc
	)
	{
	int num = npc->get_npc_num();
	for (int i = 0; i < dead_party_count; i++)
		if (dead_party[i] == num)
			return i;
	return -1;
	}

/*
 *	Add NPC to dead party list.
 *
 *	Output:	false if no room or already a member.
 */

bool Usecode_internal::add_to_dead_party
	(
	Actor *npc			// (Should not be the Avatar.)
	)
	{
	const int maxparty = sizeof(dead_party)/sizeof(dead_party[0]);
	if (!npc || dead_party_count == maxparty || in_dead_party(npc) >= 0)
		return false;
	dead_party[dead_party_count++] = npc->get_npc_num();
	return true;
	}

/*
 *	Remove NPC from dead party list.
 *
 *	Output:	false if not found.
 */

bool Usecode_internal::remove_from_dead_party
	(
	Actor *npc
	)
	{
	if (!npc)
		return false;
	int id = in_dead_party(npc);	// Get index.
	if (id == -1)			// Not in list?
		return false;
	int npc_num = npc->get_npc_num();
					// Shift the rest down.
	for (int i = id + 1; i < dead_party_count; i++)
		dead_party[i - 1] = dead_party[i];
	dead_party_count--;
	dead_party[dead_party_count] = 0;
	return true;
	}

/*
 *	Update party status of an NPC that has died or been resurrected.
 */

void Usecode_internal::update_party_status
	(
	Actor *npc
	)
	{
	if (npc->is_dead())		// Dead?
		{
					// Move party members to dead list.
		if (remove_from_party(npc))
			add_to_dead_party(npc);
		}
	else				// Alive.
		{
		if (remove_from_dead_party(npc))
			add_to_party(npc);
		}
	}

/*
 *	Start speech, or show text if speech isn't enabled.
 */

void Usecode_internal::do_speech
	(
	int num
	)
	{
	speech_track = num;		// Used in Usecode function.
	if (!Audio::get_ptr()->start_speech(num))
					// No speech?  Call text function.
		call_usecode(0x614, gwin->get_main_actor(), double_click);
	}

/*
 *	Write out global data to 'gamedat/usecode.dat'.
 *	(and 'gamedat/keyring.dat')
 *
 *	Output:	0 if error.
 */

void Usecode_internal::write
	(
	)
	{
					// Assume new games will have keyring.
	if (Game::get_game_type() != BLACK_GATE)
		keyring->write();	// write keyring data

	ofstream out;
	U7open(out, FLAGINIT);	// Write global flags.
	out.write((char*)gflags, sizeof(gflags));
	out.close();
	U7open(out, USEDAT);
	Write2(out, party_count);	// Write party.
	size_t i;	// Blame MSVC
	for (i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		Write2(out, party[i]);
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		Write4(out, timers[t]);
	Write2(out, saved_pos.tx);	// Write saved pos.
	Write2(out, saved_pos.ty);
	Write2(out, saved_pos.tz);
	out.flush();
	if( !out.good() )
		throw file_write_exception(USEDAT);
	}

/*
 *	Read in global data from 'gamedat/usecode.dat'.
 *	(and 'gamedat/keyring.dat')
 *
 *	Output:	0 if error.
 */

void Usecode_internal::read
	(
	)
	{
	if (Game::get_game_type() == SERPENT_ISLE)
		keyring->read();	// read keyring data
	

	ifstream in;
	try
	{
		U7open(in, FLAGINIT);	// Read global flags.
		in.read((char*)gflags, sizeof(gflags));
		in.close();
	} catch(exult_exception &e) {
		if (!Game::is_editing())
			throw e;
		memset(&gflags[0], 0, sizeof(gflags));
	}
	try
	{
		U7open(in, USEDAT);
	}
	catch(exult_exception &e) {
		return;		// Not an error if no saved game yet.
	}
	party_count = Read2(in);	// Read party.
	size_t i;	// Blame MSVC
	for (i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		party[i] = Read2(in);
	link_party();
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		timers[t] = Read4(in);
	if (!in.good())
		throw file_read_exception(USEDAT);
	saved_pos.tx = Read2(in);	// Read in saved position.
	saved_pos.ty = Read2(in);
	saved_pos.tz = Read2(in);
	if (!in.good() ||		// Failed.+++++Can remove this later.
	    saved_pos.tz < 0 || saved_pos.tz > 13)
		saved_pos = Tile_coord(-1, -1, -1);
	}

/*
 *	In case NPC's were read after usecode, set party members' id's, and
 *	move dead members into separate list.
 */

void Usecode_internal::link_party
	(
	)
	{
	// avatar is a party member too
	gwin->get_main_actor()->set_flag(Obj_flags::in_party);
					// You own your own stuff.
	gwin->get_main_actor()->set_flag_recursively(Obj_flags::okay_to_take);
	const int maxparty = sizeof(party)/sizeof(party[0]);
	int tmp_party[maxparty];
	int tmp_party_count = party_count;
	int i;
	for (i = 0; i < maxparty; i++)
		tmp_party[i] = party[i];
	party_count = dead_party_count = 0;
					// Now process them.
	for (i = 0; i < tmp_party_count; i++)
		{
		Actor *npc = gwin->get_npc(party[i]);
		int oldid;
		if (!npc ||		// Shouldn't happen!
					// But this has happened:
		    ((oldid = npc->get_party_id()) >= 0 && 
							oldid < party_count))
			continue;	// Skip bad entry.
		int npc_num = npc->get_npc_num();
		if (npc->is_dead())	// Put dead in special list.
			{
			npc->set_party_id(-1);
			if (dead_party_count >= 
				    sizeof(dead_party)/sizeof(dead_party[0]))
				continue;
			dead_party[dead_party_count++] = npc_num;
			continue;
			}
		npc->set_party_id(party_count);
		party[party_count++] = npc_num;
// ++++This messes up places where they should wait, and should be unnecessary.
//		npc->set_schedule_type(Schedule::follow_avatar);
					// We can use all his/her items.
		npc->set_flag_recursively(Obj_flags::okay_to_take);
		npc->set_flag (Obj_flags::in_party);
		}
	}


