/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Usecode.cc - Interpreter for usecode.
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

#include <stdio.h>			/* Debugging.			*/
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "usecode.h"
#include "gamewin.h"
#include "actors.h"
#include "schedule.h"
#include "objs.h"
#include "delobjs.h"
#include "animate.h"
#include "vec.h"
#include "SDL.h"
#include "tqueue.h"
#include "gumps.h"
#include "effects.h"
#include "mouse.h"
#include "Audio.h"
#include "useval.h"
#include "game.h"
#include "barge.h"
#include "egg.h"
#include <iomanip>
#ifdef XWIN
#include <signal.h>
#endif
#if USECODE_DEBUGGER
#include <algo.h>       // STL function things
#endif


// External globals..

extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *key = 0);
extern void Wait_for_arrival(Actor *actor);
extern	bool	usecode_trace,usecode_debugging;
extern Mouse *mouse;
extern unsigned char quitting_time;

#if USECODE_DEBUGGER
vector<int> intrinsic_breakpoints;

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
 *	A class for executing usecode at a scheduled time:
 */
class Scheduled_usecode : public Time_sensitive
	{
	static int count;		// Total # of these around.
	static Scheduled_usecode *first;// ->chain of all of them.
	Scheduled_usecode *next, *prev;	// Next/prev. in global chain.
	Usecode_value objval;		// The 'itemref' object.
	Game_object *obj;		// From objval.
	Tile_coord objpos;		// Abs. tile coord.
	Usecode_value arrval;		// Array of code to execute.
	int cnt;			// Length of arrval.
	int i;				// Current index.
	int frame_index;		// For taking steps.
	int no_halt;			// 1 to ignore halt().
public:
	Scheduled_usecode(Usecode_machine *usecode,
				Usecode_value& oval, Usecode_value& aval)
		: objval(oval), arrval(aval), i(0), frame_index(0), no_halt(0)
		{
		cnt = arrval.get_array_size();
		obj = usecode->get_item(objval);
		objpos = obj ? obj->get_abs_tile_coord() 
				: Tile_coord(-1, -1, -1);
					// Not an array?
		if (!cnt && !arrval.is_array())
			cnt = 1;	// Get_elem(0) works for non-arrays.
		count++;		// Keep track of total.
		next = first;		// Put in chain.
		prev = 0;
		if (first)
			first->prev = this;
		first = this;
		int opval0 = arrval.get_elem(0).get_int_value();
		if (opval0 == 0x23)	// PURE GUESS:
			no_halt = 1;
		}
					// Execute when due.
	virtual ~Scheduled_usecode()
		{
		count--;
		if (next)
			next->prev = prev;
		if (prev)
			prev->next = next;
		else
			first = next;
		}
	void halt()			// Stop executing.
		{
		if (!no_halt)
			i = cnt;
		}
	int is_activated()		// Started already?
		{ return i > 0; }
	void activate_egg(Usecode_machine *usecode, Game_object *e, int type)
		{
		if (e && e->is_egg() && (type == -1 || 
				((Egg_object *) e)->get_type() == type))
			((Egg_object *) e)->activate(usecode,
				usecode->gwin->get_main_actor(), 1);
		}
	static int get_count()
		{ return count; }
					// Find for given item.
	static Scheduled_usecode *find(Game_object *srch);
					// Activate itemref eggs.
	void activate_eggs(Usecode_machine *usecode);
	virtual void handle_event(unsigned long curtime, long udata);
	};

int Scheduled_usecode::count = 0;
Scheduled_usecode *Scheduled_usecode::first = 0;

/*
 *	Search list for one for a given item.
 *
 *	Output:	->Scheduled_usecode if found, else 0.
 */

Scheduled_usecode *Scheduled_usecode::find
	(
	Game_object *srch
	)
	{
	for (Scheduled_usecode *each = first; each; each = each->next)
		if (each->obj == srch)
			return each;	// Found it.
	return (0);
	}

/*
 *	Activate all cached-in usecode eggs near a given spot.
 */

static void Activate_cached
	(
	Usecode_machine *uc,
	Tile_coord pos
	)
	{
	if (Game::get_game_type() != BLACK_GATE)
		return;			// ++++Since we're not sure about it.
	const int dist = 8;
	Vector vec;			// Find all usecode eggs.
	int cnt = Game_object::find_nearby(vec, pos, 275, dist, 16, -359, 7);
	for (int i = 0; i < cnt; i++)
		{
		Egg_object *egg = (Egg_object *) vec.get(i);
		if (egg->get_criteria() == Egg_object::cached_in)
			egg->activate(uc);
		}
	}

/*
 *	Execute eggs.
 */

void Scheduled_usecode::activate_eggs
	(
	Usecode_machine *usecode
	)
	{
	int size = objval.get_array_size();
	if (!size)			// Not an array?
		{
		activate_egg(usecode, obj, -1);
		return;
		}
	int i;
	for (i = 0; i < size; i++)	// First do monsters.
		{
		activate_egg(usecode, usecode->get_item(objval.get_elem(i)),
						(int) Egg_object::monster);
		Usecode_value z(0);
		objval.put_elem(i, z);
		}
	for (i = 0; i < size; i++)	// Do the rest.
		activate_egg(usecode, usecode->get_item(objval.get_elem(i)),
									-1);
	}

/*
 *	Execute an array of usecode, generally one instruction per tick.
 */

void Scheduled_usecode::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->usecode machine.
	)
	{
	Usecode_machine *usecode = (Usecode_machine *) udata;
	Game_window *gwin = usecode->gwin;
	int delay = 200;			// Trying default delay.
	int do_another = 1;			// Flag to keep going.
	for ( ; i < cnt && do_another; i++)
		{
		do_another = 0;
		Usecode_value& opval = arrval.get_elem(i);
		int opcode = opval.get_int_value();
		switch (opcode)
			{
		case 0x01:		// ??
			break;
		case 0x0b:		// ?? 2 parms, 1st one < 0.
			{		// Loop(offset, cnt).
			do_another = 1;
			Usecode_value& cntval = arrval.get_elem(i + 2);
			int cnt = cntval.get_int_value();
			if (cnt <= 0)
					// Done.
				i += 2;
			else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = arrval.get_elem(i + 1);
				i += offval.get_int_value() - 1;
				}
			break;
			}
		case 0x0c:		// Loop with 3 parms.???
			{		// Loop(offset, cnt1, cnt2?).++++
			do_another = 1;
			Usecode_value& cntval = arrval.get_elem(i + 2);
			int cnt = cntval.get_int_value();
			if (cnt <= 0)
					// Done.
				i += 3;
			else
				{	// Decr. and loop.
				cntval = Usecode_value(cnt - 1);
				Usecode_value& offval = arrval.get_elem(i + 1);
				i += offval.get_int_value() - 1;
				}
			break;
			}
		case 0x23:		// ?? Always appears first.
					// Maybe means "don't let
					//    intrinsic 5c stop it".
			no_halt = 1;	// PURE GUESS.
			do_another = 1;
			break;
		case 0x27:		// ?? 1 parm. Guessing:
			{		//   delay before next instruction.
			Usecode_value& delayval = arrval.get_elem(++i);
					// ?? Guessing at time.
//			delay = 200*(delayval.get_int_value()); 9/17/00:
// NOTE: Changing this can have a major impact!
			delay = 400*(delayval.get_int_value());
			break;		
			}
#if 0
		case 0x2c:		// Quit if there's already scheduled
					//   code for item?
					// Or supercede the existing one?
			break;
#endif
		case 0x2d:		// Remove itemref.
			usecode->remove_item(obj);
			break;
		case 0x39:		// Rise?  (For flying carpet.
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz < 11)
				t.tz++;
			obj->move(t);
			break;
			}
		case 0x38:		// Descend?
			{
			Tile_coord t = obj->get_abs_tile_coord();
			if (t.tz > 0)
				t.tz--;
			obj->move(t);
			break;
			}
		case 0x46:
			{		// Set frame.
			Usecode_value& fval = arrval.get_elem(++i);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x48:		// Guessing:  activate egg.
			activate_eggs(usecode);
			break;
		case 0x4e:		// Show next frame.
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			Usecode_value fval((1 + obj->get_framenum())%nframes);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x50:		// Guessing: Show prev. frame.
			{
			int nframes = gwin->get_shapes().get_num_frames(
							obj->get_shapenum());
			int pframe = obj->get_framenum() - 1;
			Usecode_value fval((pframe + nframes)%nframes);
			usecode->set_item_frame(objval, fval);
			break;
			}
		case 0x52:		// Say string.
			{
			Usecode_value& strval = arrval.get_elem(++i);
			usecode->item_say(objval, strval);
			break;
			}
		case 0x55:		// Call?
			{
			Usecode_value& val = arrval.get_elem(++i);
			int fun = val.get_int_value();
					// REALLY guessing (for Forge):
			Usecode_machine::Usecode_events ev = 
					Usecode_machine::internal_exec;
			if (obj->is_egg() && ((Egg_object *)obj)->get_type() ==
			    Egg_object::usecode)
				{
				ev = Usecode_machine::egg_proximity;
				cout << "0x55:  guessing with egg" << endl;
				}
			usecode->call_usecode(fun, obj, ev);
			break;
			}
		case 0x58:		// ?? 1 parm, fairly large byte.
			i++;		// Wield weapon (tmporarily). parm=?+++
			break;
		case 0x59:		// Parm. is dir. (0-7).  0=north?
			{
					// Look in that dir.
			Usecode_value& val = arrval.get_elem(++i);
					// It may be 0x3x.  Face dir?
			int dir = val.get_int_value()&7;
			obj->set_usecode_dir(dir);
			Usecode_value v(obj->get_dir_framenum(
							dir, Actor::standing));
			usecode->set_item_frame(objval, v);
			frame_index = 0;// Reset walking frame index.
			break;
			}
		default:
					// Frames with direction.
			if (opcode >= 0x60 && opcode <= 0x7f)
				{
				Usecode_value v(obj->get_dir_framenum(
					obj->get_usecode_dir(), opcode));
				usecode->set_item_frame(objval, v);
				}
					// ++++Guessing:
			else if (opcode >= 0x30 && opcode < 0x38)
				{	// Step in dir. opcode&7.????
				int dir = opcode&7;
				int frame = obj->get_framenum();
				Actor *act = usecode->as_actor(obj);
				if (act)
					{
					Frames_sequence *frames = 
							act->get_frames(dir);
					// Get frame (updates frame_index).
					frame = frames->get_next(frame_index);
					}
				Barge_object *brg = 
					dynamic_cast<Barge_object *> (obj);
				int repeat = brg ? 4 : 1;
				for (int i = 0; i < repeat; i++)
					{
					Tile_coord tile = 
						obj->get_abs_tile_coord().
							get_neighbor(dir);
					obj->step(tile, frame);
					gwin->paint_dirty();
					gwin->show();
					}
				}
			else
			        cout << "Und sched. opcode " << hex << 
			"0x" << setfill((char)0x30) << setw(2) << opcode << endl;

			break;
			}
		}
	if (i < cnt)			// More to do?
		{
		gwin->get_tqueue()->add(curtime + delay, this, udata);
		return;
		}
					// Don't get stuck in conv. mode.
	if (gwin->get_mode() == Game_window::conversation)
		{
//		gwin->set_mode(Game_window::normal);
		gwin->end_gump_mode();	// This also sets mode=normal.
		gwin->paint();
		}
	if (count == 1 &&		// Last one?  GUESSING:
	    objpos.tx != -1)		// And valid pos.
		Activate_cached(usecode, objpos);
	delete this;			// Hope this is safe.
	}

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
		delete String;
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
	long val = elemval.get_int_value();
	if (!val)
		return (0);
	Game_object *obj = 0;
	if (val == -356)		// Avatar.
		return gwin->get_main_actor();
	if (val < 0)
		obj = gwin->get_npc(-val);
	else if (val < gwin->get_num_npcs())
		obj = gwin->get_npc(val);
					// Special case:  palace guards.
	else if (val < 0x400)		// Looks like a shape #?
		{
		if (!itemref.is_array() &&
 		    caller_item && val == caller_item->get_shapenum())
			obj = caller_item;
		else
			return 0;	// Can't be an object.
		}
	return obj ? obj : (Game_object *) val;
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
	delete String;
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
	delete String;
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
	user_choice = 0;		// Seems like a good idea.
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
			Gump_object *gump = gwin->find_gump(item);
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
#if 0	/* ++++Messes up with rotated frames. */
	if (frame < gwin->get_shape_num_frames(item->get_shapenum()))
#endif
		item->set_frame(frame);
	if (item->get_owner())		// Inside a container?
		{
		Gump_object *gump = gwin->find_gump(item);
		if (gump)
			gump->paint(gwin);
		}
	else
		{			// Figure area to repaint.
		Rectangle rect = gwin->get_shape_rect(item);
		rect.enlarge(8);
		rect = gwin->clip_to_win(rect);
		gwin->paint(rect);
		}
	gwin->show();
	}

/*
 *	Get an item's shape.
 */

int Usecode_machine::get_item_shape
	(
	Usecode_value& item_arg
	)
	{
	Game_object *item = get_item(item_arg);
	return (item == 0 ? 0 : item->get_shapenum());
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
 *	Add an NPC to the party.
 */

void Usecode_machine::add_to_party
	(
	Game_object *npc
	)
	{
	if (!npc || party_count == PARTY_MAX || npc_in_party(npc))
		return;			// Can't add.
	npc->set_party_id(party_count);
	party[party_count++] = npc->get_npc_num();
	npc->set_schedule_type(Schedule::follow_avatar);
// cout << "NPC " << npc->get_npc_num() << " added to party." << endl;
	}

/*
 *	Remove an NPC from the party.
 */

void Usecode_machine::remove_from_party
	(
	Game_object *npc
	)
	{
	if (!npc)
		return;
	int id = npc->get_party_id();
	if (id == -1)			// Not in party?
		return;
	if (party[id] != npc->get_npc_num())
		{
		cout << "Party mismatch!!" << endl;
		return;
		}
					// Shift the rest down.
	for (int i = id + 1; i < party_count; i++)
		{
		Actor *npc2 = gwin->get_npc(party[i]);
		if (npc2)
			npc2->set_party_id(i - 1);
		party[i - 1] = party[i];
		}
	party_count--;
	party[party_count] = 0;
	npc->set_party_id(-1);
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
	Usecode_value aval((long) gwin->get_main_actor());
	arr.put_elem(0, aval);	
	int num_added = 1;
	for (int i = 0; i < party_count; i++)
		{
		Game_object *obj = gwin->get_npc(party[i]);
		if (!obj)
			continue;
		Usecode_value val((long) obj);
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

static Barge_object *Get_barge
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
		Gump_object *gump = gwin->find_gump(obj);
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
	Vector vec;			// Gets list.
	int cnt;
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
		cnt = Game_object::find_nearby(vec,
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
		cnt = obj->find_nearby(vec, shapeval.get_int_value(),
			distval.get_int_value(), mval.get_int_value());
		}
	Usecode_value nearby(cnt, 0);	// Create return array.
	for (int i = 0; i < cnt; i++)
		{
		Game_object *each = (Game_object *) vec.get(i);
		Usecode_value val((long) each);
		nearby.put_elem(i, val);
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
		return Usecode_value(0);
	Vector vec;			// Gets list.
	obj = obj->get_outermost();	// Might be inside something.
	int cnt = obj->find_nearby(vec, shapeval.get_int_value(), 
						distval.get_int_value(), 0);
	Game_object *closest = 0;
	unsigned long bestdist = 100000;// Distance-squared in tiles.
	int x1, y1, z1;
	obj->get_abs_tile(x1, y1, z1);
	for (int i = 0; i < cnt; i++)
		{
		Game_object *each = (Game_object *) vec.get(i);
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
	return Usecode_value((long) closest);
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
		return Usecode_value(0);
	int shapenum = shapeval.get_int_value();
	int framenum = frameval.get_int_value();
	int qual = qualval.get_int_value();
	Vector vec;			// Gets list.
	int cnt = obj->get_objects(vec, shapenum, qual, framenum);

//	cout << "Container objects found:  " << cnt << << endl;
	Usecode_value within(cnt, 0);	// Create return array.
	for (int i = 0; i < cnt; i++)
		{
		Game_object *each = (Game_object *) vec.get(i);
		Usecode_value val((long) each);
		within.put_elem(i, val);
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
	Usecode_value result(0);
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
	Gump_object *gump = gwin->find_gump(x, y);
	Game_object *obj;
	if (gump)
		obj = gump->find_object(x, y);
	else				// Search rest of world.
		{
		obj = gwin->find_object(x, y);
		if (obj)		// Found object?  Use its coords.
			obj->get_abs_tile(tx, ty, tz);
		}
	Usecode_value oval((long) obj);	// Ret. array with obj as 1st elem.
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

static Usecode_value	no_ret;

Usecode_value Usecode_machine::Execute_Intrinsic(UsecodeIntrinsicFn func,const char *name,int event,int intrinsic,int num_parms,Usecode_value parms[12])
{
#ifdef XWIN
#if USECODE_DEBUGGER
	if(usecode_debugging)
		{
		// Examine the list of intrinsics for function breakpoints.
		if(find(intrinsic_breakpoints.begin(),intrinsic_breakpoints.end(),intrinsic)!=intrinsic_breakpoints.end())
			{
			raise(SIGIO);	// Breakpoint
			}
		}
#endif
#endif
	if(usecode_trace)
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

USECODE_INTRINSIC(NOP)
{
	return(no_ret);
}

USECODE_INTRINSIC(UNKNOWN)
{
//	Unhandled(intrinsic, num_parms, parms);
	return(no_ret);
}

USECODE_INTRINSIC(get_random)
{
	int range = parms[0].get_int_value();
	if (range == 0)
		{
		Usecode_value u(0);
		return(u);
		}
	Usecode_value u=(1 + (rand() % range));
	return(u);
}

USECODE_INTRINSIC(execute_usecode_array)
{
	cout << "Executing intrinsic 1" << endl;
					// 9/17/00:  New guess to make it
					//   possible to heat Black sword.
	Game_object *item = get_item(parms[0]);
	Scheduled_usecode *uc;
	if (item && (uc = Scheduled_usecode::find(item)) != 0 &&
	    uc->is_activated())
		uc->halt();		// Stop current one.

	gwin->get_tqueue()->add(SDL_GetTicks() + 1,
		new Scheduled_usecode(this, parms[0], parms[1]), (long) this);
	return(no_ret);
}

USECODE_INTRINSIC(delayed_execute_usecode_array)
{
	// Delay = .20 sec.?
					// +++++Special problem with inf. loop:
	if (Game::get_game_type() == BLACK_GATE &&
	    event == internal_exec && parms[1].get_array_size() == 3 &&
	    parms[1].get_elem(2).get_int_value() == 0x6f7)
		return(no_ret);
	int delay = parms[2].get_int_value();
	gwin->get_tqueue()->add(SDL_GetTicks() + 200*delay,
		new Scheduled_usecode(this, parms[0], parms[1]),
							(long) this);
	cout << "Executing intrinsic 2" << endl;
	return(no_ret);
}

USECODE_INTRINSIC(show_npc_face)
{
	show_npc_face(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(remove_npc_face)
{
	remove_npc_face(parms[0]);
	return(no_ret);
}

USECODE_INTRINSIC(add_answer)
{
	answers.add_answer(parms[0]);
	user_choice = 0;
	return(no_ret);
}

USECODE_INTRINSIC(remove_answer)
{
	answers.remove_answer(parms[0]);
// Commented out 'user_choice = 0' 8/3/00 for Tseramed conversation.
//	user_choice = 0;
	return(no_ret);
}

USECODE_INTRINSIC(push_answers)
{
	answer_stack.push_front(answers);
	answers.clear();
	return(no_ret);
}

USECODE_INTRINSIC(pop_answers)
{
	if(answer_stack.size())
		{
		answers=answer_stack.front();
		answer_stack.pop_front();
		user_choice = 0;	// Added 7/24/2000.
		}
	return(no_ret);
}

USECODE_INTRINSIC(clear_answers)
{
	answers.clear();
	return(no_ret);
}

USECODE_INTRINSIC(select_from_menu)
{
	user_choice = 0;
	const char *choice = get_user_choice();
	user_choice = 0;
	Usecode_value u(choice);
	return(u);
}

USECODE_INTRINSIC(select_from_menu2)
{
	// Return index (1-n) of choice.
	user_choice = 0;
	Usecode_value val(get_user_choice_num() + 1);
	user_choice = 0;
	return(val);
}

USECODE_INTRINSIC(input_numeric_value)
{
	// Ask for # (min, max, step, default).
	extern int Prompt_for_number(int minval, int maxval, 
							int step, int def);
	Usecode_value ret(Prompt_for_number(
		parms[0].get_int_value(), parms[1].get_int_value(),
		parms[2].get_int_value(), parms[3].get_int_value()));
	gwin->clear_text_pending();	// Answered a question.
	return(ret);
}

USECODE_INTRINSIC(set_item_shape)
{
	// Set item shape.
	set_item_shape(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(find_nearest)
{
	// Think it rets. nearest obj. near parm0.
	Usecode_value u(find_nearest(parms[0], parms[1], parms[2]));
	return(u);
}

USECODE_INTRINSIC(die_roll)
{
	// Rand. # within range.
	int low = parms[0].get_int_value();
	int high = parms[1].get_int_value();
	if (low > high)
		{
		int tmp = low;
		low = high;
		high = tmp;
		}
	int val = (rand() % (high - low + 1)) + low;
	Usecode_value u(val);
	return(u);
}

USECODE_INTRINSIC(get_item_shape)
{
	Usecode_value u(get_item_shape(parms[0]));
	return(u);
}

USECODE_INTRINSIC(get_item_frame)
{
	Game_object *item = get_item(parms[0]);
					// Don't count rotated frames.
	return Usecode_value(item == 0 ? 0 : item->get_framenum()&31);
}

USECODE_INTRINSIC(set_item_frame)
{
	set_item_frame(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(get_item_quality)
{
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? obj->get_quality() : 0);
	return(u);
}

USECODE_INTRINSIC(set_item_quality)
{
	// Guessing it's 
	//  set_quality(item, value).
	Game_object *obj = get_item(parms[0]);
	if (obj)
		obj->set_quality((unsigned int) parms[1].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(get_item_quantity)
{
	// Get quantity of an item.
	//   Get_quantity(item, mystery).
	Usecode_value ret(0);
	Game_object *obj = get_item(parms[0]);
	if (obj)
		ret = Usecode_value(obj->get_quantity());
	return(ret);
}

USECODE_INTRINSIC(set_item_quantity)
{
	// Set_quantity (item, newcount).  Rets ???.
	Usecode_value ret(0);
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		int oldquant = obj->get_quantity();
		int delta = parms[1].get_int_value() - oldquant;
					// Note:  This can delete the obj.
		int newdelta = obj->modify_quantity(delta);
					// Guess:  Return new quantity.
		ret = Usecode_value(oldquant + newdelta);
					// ++++Maybe repaint?
		}
	return(ret);
}

USECODE_INTRINSIC(get_object_position)
{
	// Takes itemref.  ?Think it rets.
	//  hotspot coords: (x, y, z).
	Game_object *obj = get_item(parms[0]);
	Tile_coord c(0, 0, 0);
	if (obj)		// (Watch for animated objs' wiggles.)
		c = obj->get_outermost()->get_original_tile_coord();
	Usecode_value vx(c.tx), vy(c.ty), vz(c.tz);
	Usecode_value arr(3, &vx);
	arr.put_elem(1, vy);
	arr.put_elem(2, vz);
	return(arr);
}

USECODE_INTRINSIC(get_distance)
{
	// Distance from parm[0] -> parm[1].  Guessing how it's computed.
	Game_object *obj0 = get_item(parms[0]);
	Game_object *obj1 = get_item(parms[1]);
	Usecode_value u((obj0 && obj1) ? obj0->get_abs_tile_coord().distance(
					obj1->get_abs_tile_coord()) : 0);
	return(u);
}

USECODE_INTRINSIC(find_direction)
{
	// Direction from parm[0] -> parm[1].
	// Rets. 0-7.  Is 0 east?
	Usecode_value u=find_direction(parms[0], parms[1]);
	return(u);
}

USECODE_INTRINSIC(get_npc_object)
{
	// Takes -npc.  Returns object, or array of objects.
	Usecode_value& v = parms[0];
	if (v.is_array())		// Do it for each element of array.
		{
		int sz = v.get_array_size();
		Usecode_value ret(sz, 0);
		for (int i = 0; i < sz; i++)
			{
			Usecode_value elem((long) get_item(v.get_elem(i)));
			ret.put_elem(i, elem);
			}
		return ret;
		}
	Game_object *obj = get_item(parms[0]);
	Usecode_value u((long) obj);
	return(u);
}

USECODE_INTRINSIC(get_schedule_type)
{
	// GetSchedule(npc).  Rets. schedtype.
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? obj->get_schedule_type() : 0);
	return(u);
}

USECODE_INTRINSIC(set_schedule_type)
{
	// SetSchedule?(npc, schedtype).
	// Looks like 15=wait here, 11=go home, 0=train/fight... This is the
	// 'bNum' field in schedules.
	Game_object *obj = get_item(parms[0]);
	if (obj)
		obj->set_schedule_type(parms[1].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(add_to_party)
{
	// NPC joins party.
	add_to_party(get_item(parms[0]));
	return(no_ret);
}

USECODE_INTRINSIC(remove_from_party)
{
	// NPC leaves party.
	remove_from_party(get_item(parms[0]));
	return(no_ret);
}

USECODE_INTRINSIC(get_npc_prop)
{
	// Get NPC prop (item, prop_id).
	//   (9 is food level).
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? 
		obj->get_property(parms[1].get_int_value()) : 0);
	return(u);
}

USECODE_INTRINSIC(set_npc_prop)
{
	// Set NPC prop (item, prop_id, delta_value).
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{			// NOTE: 3rd parm. is a delta!
		int prop = parms[1].get_int_value();
		obj->set_property(prop, obj->get_property(prop) +
						parms[2].get_int_value());
		}
	return(no_ret);
}

USECODE_INTRINSIC(get_avatar_ref)
{
	// Guessing it's Avatar's itemref.
	Usecode_value u((long) gwin->get_main_actor());
	return(u);
}

USECODE_INTRINSIC(get_party_list)
{
	// Return array with party members.
	Usecode_value u(get_party());
	return(u);
}

USECODE_INTRINSIC(create_new_object)
{
	int shapenum = parms[0].get_int_value();
	unsigned int tx;
	unsigned int ty;
	unsigned int cx;
	unsigned int cy;
	unsigned int lift;
	extern int Is_body(int);// +++++Pretty kludgy.

	if (num_parms == 2)
	{
		tx = parms[1].get_elem(0).get_int_value()%tiles_per_chunk;
		ty = parms[1].get_elem(1).get_int_value()%tiles_per_chunk;
		cx = parms[1].get_elem(0).get_int_value()/tiles_per_chunk;
		cy = parms[1].get_elem(1).get_int_value()/tiles_per_chunk;
		lift = parms[1].get_elem(2).get_int_value();
		cout << "LOC " << endl;
	}
	else
	{
		Game_object *at = caller_item ? caller_item->get_outermost()
								: 0;
		if (!at || at->get_cx() == 255)	// Invalid chunk?
			at = gwin->get_main_actor();
		
		tx = at->get_tx();
		ty = at->get_ty();
		cx = at->get_cx();
		cy = at->get_cy();
		lift = at->get_lift();
		cout << " AT " << endl;
	}

	Game_object *obj;		// Create to be written to Ireg.
	Monster_info *inf = gwin->get_monster_info(shapenum);

	if (inf)
	{
		Monster_actor *monster = inf->create(
			cx, cy, tx, ty, lift);
		gwin->add_dirty(monster);
		gwin->add_nearby_npc(monster);
		gwin->show();
		last_created = monster;
		return Usecode_value((long) monster);
	}
	else
	{
		if (Is_body(shapenum))
		{
			obj = new Dead_body(shapenum, 0, tx, ty, lift, -1);
			cout << " body " << endl;
		}
		else
		{
			obj = gwin->create_ireg_object(
				gwin->get_info(shapenum), shapenum, 0,
								tx, ty, lift);
			cout << " ireg object " << endl;
		}
	}
	Chunk_object_list *chunk = gwin->get_objects(cx, cy);
	if (obj->is_egg())
		chunk->add_egg((Egg_object *) obj);
	else
		chunk->add(obj);
	gwin->show();
	last_created = obj;
	Usecode_value u((long) obj);
	return(u);
}

USECODE_INTRINSIC(set_last_created)
{
	// Take itemref, sets last_created to it.
	Game_object *obj = get_item(parms[0]);
	last_created = obj;
	Usecode_value u((long) obj);
	return(u);
}

USECODE_INTRINSIC(update_last_created)
{
	// Think it takes array from 0x18,
	//   updates last-created object.
	//   ??guessing??
	if (!last_created)
		{
		Usecode_value u(0);
		return(u);
		}
	Usecode_value& arr = parms[0];
	int sz = arr.get_array_size();
	if (sz == 3)
		last_created->move(arr.get_elem(0).get_int_value(),
			  arr.get_elem(1).get_int_value(),
			  arr.get_elem(2).get_int_value());
				// Taking a guess here:
	else if (parms[0].get_int_value() == -358)
		last_created->remove_this();
#if DEBUG
	else
		{
		cout << " { Intrinsic 0x26:  "; arr.print(cout); cout << endl << "} ";
		}
#endif
	gwin->paint_dirty();
	gwin->show();		// ??
	Usecode_value u(1);// ??
	return(u);
}

USECODE_INTRINSIC(get_npc_name)
{
	// Get NPC name(s).  Works on arrays, too.
	static const char *unknown = "??name??";
	int cnt = parms[0].get_array_size();
	if (cnt)
		{			// Do array.
		Usecode_value arr(cnt, 0);
		for (int i = 0; i < cnt; i++)
			{
			Game_object *obj = get_item(parms[0].get_elem(i));
			Usecode_value v(obj->get_name().c_str());
			arr.put_elem(i, v);
			}
		return(arr);
		}
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? obj->get_name().c_str() : unknown);
	return(u);
}

USECODE_INTRINSIC(count_objects)
{
	// How many?
	// ((npc?-357==party, -356=avatar), 
	//   item, quality, frame (-359 = any)).
	// Quality/frame -359 means any.
	Usecode_value u(count_objects(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}

USECODE_INTRINSIC(find_in_owner)
{
	// Find_in_owner(container(-357=party), shapenum, qual?? (-359=any), 
	//						frame??(-359=any)).
	int oval  = parms[0].get_int_value(),
	    shnum = parms[1].get_int_value(),
	    qual  = parms[2].get_int_value(),
	    frnum = parms[3].get_int_value();
	if (oval != -357)		// Not the whole party?
		{
		Game_object *obj = get_item(parms[0]);
		if (!obj)
			return Usecode_value(0);
		Game_object *f = obj->find_item(shnum, qual, frnum);
		return Usecode_value((long) f);
		}
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_item(party.get_elem(i));
		if (obj)
			{
			Game_object *f = obj->find_item(shnum, qual, frnum);
			if (f)
				return Usecode_value((long) f);
			}
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(get_cont_items)
{
        // Get cont. items(container, shape, qual, frame).
        Usecode_value u(get_objects(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}


USECODE_INTRINSIC(remove_party_items)
{
	// Remove items(quantity, item, ??quality?? (-359), frame(-359), T/F).
	Usecode_value u(remove_party_items(parms[0], parms[1], parms[2],
						parms[3], parms[4]));
	return(u);
}

USECODE_INTRINSIC(add_party_items)
{
	// Add items(num, item, ??quality?? (-359), frame (or -359), T/F).
	// Returns array of NPC's (->'s) who got the items.
	Usecode_value u(add_party_items(parms[0], parms[1], parms[2],
						parms[3], parms[4]));
	return(u);
}

USECODE_INTRINSIC(play_music)
{
	// Play music(item, songnum).
	// ??Show notes by item?
#if DEBUG
	cout << "Music request in usecode" << endl;
	cout << "Parameter data follows" << endl;
	cout << "0: " << ((parms[0].get_int_value()>>8)&0xff) << " " <<  ((parms[0].get_int_value())&0xff) << endl;
	cout << "1: " << ((parms[1].get_int_value()>>8)&0x01) << " " <<  ((parms[1].get_int_value())&0x01) << endl;
#endif
	int track = parms[0].get_int_value()&0xff;
	if (track == 0xff)		// I think this is right:
		audio->cancel_streams();	// Stop playing.
	else
		audio->start_music(track, (parms[0].get_int_value()>>8)&0x01);
	return(no_ret);
}

USECODE_INTRINSIC(npc_nearby)
{
	// NPC nearby? (item).
	Game_object *npc = get_item(parms[0]);
	int near = (npc != 0 && npc->get_abs_tile_coord().distance(
		gwin->get_main_actor()->get_abs_tile_coord()) < 12);
	Usecode_value u(near);
	return(u);
}

USECODE_INTRINSIC(find_nearby_avatar)
{
	// Find objs. with given shape near Avatar?
	Usecode_value av((long) gwin->get_main_actor());
	Usecode_value dist(64), mask(0);
	Usecode_value u(find_nearby(av, parms[0], dist, mask));
	return(u);
}

USECODE_INTRINSIC(is_npc)
{
	// Is item an NPC?
	Game_object *obj = get_item(parms[0]);
					// ++++In future, check for monsters.
	if(!obj)
		{
#if DEBUG
		cerr << "is_npc: get_item returned a NULL pointer" << endl;
#endif
		Usecode_value u(0);
		return(u);
		}
	Usecode_value u(obj == gwin->get_main_actor() ||
			obj->get_npc_num());// > 0);
	return(u);
}

USECODE_INTRINSIC(display_runes)
{
	// Render text into runes for signs, tombstones, plaques and the like
	// Display sign (gump #, array_of_text).
	int cnt = parms[1].get_array_size();
	if (!cnt)
		cnt = 1;		// Try with 1 element.
	Sign_gump *sign = new Sign_gump(parms[0].get_int_value(), cnt);
	for (int i = 0; i < cnt; i++)
		{			// Paint each line.
		Usecode_value& lval = parms[1].get_elem(i);
		sign->add_text(i, lval.get_str_value());
		}
	sign->paint(gwin);		// Paint it, and wait for click.
	int x, y;
	Get_click(x, y, Mouse::hand);
	delete sign;
	gwin->paint();
	return(no_ret);
}

USECODE_INTRINSIC(click_on_item)
{
	// Doesn't ret. until user single-
	//   clicks on an item.  Rets. item.
	Usecode_value u(click_on_item());
	return(u);
}

USECODE_INTRINSIC(find_nearby)
{
	// Think it rets. objs. near parm0.
	Usecode_value u(find_nearby(parms[0], parms[1], parms[2], parms[3]));
	return(u);
}

USECODE_INTRINSIC(give_last_created)
{
	// Think it's give_last_created_to_npc(npc).
	Game_object *npc = get_item(parms[0]);
	int ret = 0;
	if (npc && last_created)
		{			// Remove, but don't delete, last.
		last_created->remove_this(1);
		ret = npc->add(last_created);
		}
	Usecode_value u(ret);
	return(u);
}

USECODE_INTRINSIC(is_dead)
{
	// Return 1 if parm0 is a dead NPC.
	Game_object *npc = get_item(parms[0]);
	Usecode_value u(npc->is_dead_npc());
	return(u);
}

USECODE_INTRINSIC(game_hour)
{
	// Return. game time hour (0-23).
	Usecode_value u(gwin->get_hour());
	return(u);
}

USECODE_INTRINSIC(game_minute)
{
	// Return minute (0-59).
	Usecode_value u(gwin->get_minute());
	return(u);
}

USECODE_INTRINSIC(get_npc_number)
{
	// Returns NPC# of item. (-356 =
	//   avatar).
	Game_object *obj = get_item(parms[0]);
	if (obj == gwin->get_main_actor())
		{
		Usecode_value u(-356);
		return(u);
		}
	int npc = obj ? obj->get_npc_num() : 0;
	Usecode_value u(-npc);
	return(u);
}

USECODE_INTRINSIC(part_of_day)
{
	// Return 3-hour # (0-7, 0=midnight).
	Usecode_value u(gwin->get_hour()/3);
	return(u);
}

USECODE_INTRINSIC(get_alignment)
{
	// Get npc's alignment.
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? obj->get_alignment() : 0);
	return(u);
}

USECODE_INTRINSIC(set_alignment)
{
	// Set npc's alignment.
	// 2,3==bad towards Ava. 0==good.
	Game_object *obj = get_item(parms[0]);
	if (obj)
		obj->set_alignment(parms[1].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(move_object)
{
	// move_object(obj(-357=party), (tx, ty, tz)).
	Usecode_value& p = parms[1];
	Tile_coord tile(p.get_elem(0).get_int_value(),
			p.get_elem(1).get_int_value(),
			p.get_elem(2).get_int_value());
	Actor *ava = gwin->get_main_actor();
	if (parms[0].get_int_value() == -357)
		{			// Move whole party.
		gwin->teleport_party(tile);
		return (no_ret);
		}
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return (no_ret);
	Tile_coord oldpos = obj->get_abs_tile_coord();
	obj->move(tile.tx, tile.ty, tile.tz);
	Actor *act = as_actor(obj);
	if (act)
		{
		act->set_action(0);
		if (act == ava)
			{		// Teleported Avatar?
					// Make new loc. visible, test eggs.
			gwin->center_view(tile);
			Chunk_object_list::try_all_eggs(ava, tile.tx, 
				tile.ty, tile.tz, oldpos.tx, oldpos.ty);
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(remove_npc)
{
	// Remove_npc(npc) - Remove npc from world.
	Game_object *npc = get_item(parms[0]);
	if (npc)
		{
		gwin->add_dirty(npc);
		npc->remove_this(1);	// Remove, but don't delete.
		}
	return (no_ret);
}

USECODE_INTRINSIC(item_say)
{
	// Show str. near item (item, str).
	item_say(parms[0], parms[1]);
	return(no_ret);
}

USECODE_INTRINSIC(projectile_effect)
{
	// animate(fromitem, toitem, anim_shape_in_shapesdotvga).
	// ???? When it reaches toitem, toitem is 'attacked' by anim_shape.
	//   Returns??}
	Game_object *from = get_item(parms[0]),
		    *to = get_item(parms[1]);
	if (!from || !to)
		return Usecode_value(0);
	Actor *attacker = as_actor(from);
	if (!attacker)
		return Usecode_value(0);
	gwin->add_effect(new Projectile_effect(attacker, to,
						parms[2].get_int_value()));

	return Usecode_value(0);	// Not sure what this should be.
}

USECODE_INTRINSIC(get_lift)
{
	// ?? Guessing rets. lift(item).
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(obj ? Usecode_value(obj->get_lift())
					: Usecode_value(0));
	return(u);
}

USECODE_INTRINSIC(set_lift)
{
	// ?? Guessing setlift(item, lift).
	Game_object *obj = get_item(parms[0]);
	if (obj)
		{
		int x, y, z;
		obj->get_abs_tile(x, y, z);
		obj->move(x, y, parms[1].get_int_value());
		gwin->paint();
		gwin->show();
		}
	return(no_ret);
}

USECODE_INTRINSIC(sit_down)
{
	// Sit_down(npc, chair).
	Game_object *nobj = get_item(parms[0]);
	Actor *npc = as_actor(nobj);
	if (!npc)
		return (no_ret);	// Doesn't look like an NPC.
	Game_object *chair = get_item(parms[1]);
	if (!chair)
		return(no_ret);
	Vector vec;			// See if someone already there.
	int cnt = chair->find_nearby(vec, -359, 1, 0, -359, -359);
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = (Game_object *) vec.get(i);
		if ((obj->get_framenum()&0xf) == Actor::sit_frame)
			return no_ret;	// Occupied.
		}
	npc->set_schedule_type(Schedule::sit, new Sit_schedule(npc, chair));
	return(no_ret);
}

USECODE_INTRINSIC(summon)
{
	// summon(shape, flag??).  Create monster of desired shape.

	int shapenum = parms[0].get_int_value();
	Monster_info *info = gwin->get_monster_info(shapenum);
	if (info)
		{
		//+++++++Create monster & find free spot near Avatar.
		// return Usecode_value((long) monst);
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(display_map)
{
	// Display map.
	Shape_frame *map = gwin->get_sprite_shape(22, 0);
					// Get coords. for centered view.
	int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
	int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
	gwin->paint_shape(x, y, map, 1);

	//count all sextants in party
	Usecode_value v_357(-357), v650(650), v_359(-359);
	long sextants = count_objects(v_357, v650, v_359, v_359).get_int_value();
	if ((!gwin->is_main_actor_inside()) && (sextants > 0)) {
		// mark location
		int tx, ty, z, xx, yy;
		gwin->get_main_actor()->get_abs_tile(tx, ty, z);

		//the 5 and 10 below are the map-borders, 3072 dimensions of the world
		//the +1 seems to improve the location, maybe something to do with "/3072" ?
		xx = ((tx * (map->get_width() - 10)) / 3072) + (5 + x - map->get_xleft()) + 1;
		yy = ((ty * (map->get_height() - 10)) / 3072) + (5 + y - map->get_yabove()) + 1;

		gwin->get_win()->fill8(0, 1, 5, xx, yy - 2); // black isn't the correct colour
		gwin->get_win()->fill8(0, 5, 1, xx - 2, yy); // ++++ should be yellow!
	}

	gwin->show(1);
	int xx, yy;
	Get_click(xx, yy, Mouse::hand);
	gwin->paint();
	return(no_ret);
}

USECODE_INTRINSIC(kill_npc)
{
	// kill_npc(npc).
	Game_object *item = get_item(parms[0]);
	Actor *npc = as_actor(item);
	if (npc)
		npc->die();
	return (no_ret);
}

USECODE_INTRINSIC(set_attack_mode)
{
	// set_attack_mode(npc, mode).
	Actor *npc = as_actor(get_item(parms[0]));
	if (npc)
		npc->set_attack_mode((Actor::Attack_mode) 
					parms[1].need_int_value());
	return (no_ret);
}

USECODE_INTRINSIC(set_opponent)
{
	// set_opponent(npc, new_opponent).
	Actor *npc = as_actor(get_item(parms[0]));
	Game_object *opponent = get_item(parms[1]);
	if (npc && opponent)
		npc->set_opponent(opponent);
	return (no_ret);
}

USECODE_INTRINSIC(display_area)
{
	// display_area(tilepos) - used for crystal balls.
	int size = parms[0].get_array_size();
	if (size >= 3)
		{
		int tx = parms[0].get_elem(0).get_int_value();
		int ty = parms[0].get_elem(1).get_int_value();
		int unknown = parms[0].get_elem(2).get_int_value();
					// Figure in tiles.
		int tw = gwin->get_width()/tilesize, 
		    th = gwin->get_height()/tilesize;
					// Paint game area.
		gwin->paint_map_at_tile(tx - tw/2, ty - th/2, 4);
					// Paint sprite #10 (black gate!)
					//   over it.
		Shape_frame *sprite = gwin->get_sprite_shape(10, 0);
		if (sprite)		// They have translucency.
			{		// Center it.
			int topx = (gwin->get_width() - sprite->get_width())/2,
			    topy = (gwin->get_height() - 
						sprite->get_height())/2;
			gwin->paint_shape(topx + sprite->get_xleft(),
					  topy + sprite->get_yabove(), 
								sprite, 1);
			}
		gwin->show();
		int x, y;		// Wait for click.
		Get_click(x, y, Mouse::hand);
		gwin->paint();		// Repaint normal area.
		}
	return (no_ret);
}

USECODE_INTRINSIC(resurrect)
{
	// resurrect(body).  Returns actor if successful.
	Game_object *body = get_item(parms[0]);
	int npc_num = body ? body->get_live_npc_num() : -1;
	if (npc_num < 0)
		return Usecode_value(0);
	Actor *actor = gwin->get_npc(npc_num);
	if (actor)
		actor = actor->resurrect((Dead_body *) body);
	return Usecode_value((long) actor);
}

USECODE_INTRINSIC(add_spell)
{
	// add_spell(spell# (0-71), ??, spoolbook).
	// Returns 0 if book already has that spell.
	Game_object *obj = get_item(parms[2]);
	if (!obj)
		return Usecode_value(0);
	Spellbook_object *book = (Spellbook_object *) obj;
	return Usecode_value(book->add_spell(parms[0].get_int_value()));
}

USECODE_INTRINSIC(sprite_effect)
{
	// Display animation from sprites.vga.
	// show_sprite(sprite#, tx, ty, tz, ?, ?, ?);
	int lift = parms[3].get_int_value();	// ???Guessing.
	gwin->add_effect(new Sprites_effect(parms[0].get_int_value(),
		parms[1].get_int_value() - lift/2,
		parms[2].get_int_value() - lift/2));
	return(no_ret);
}

USECODE_INTRINSIC(book_mode)
{
	// Display book or scroll.
	Text_gump *gump;
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		{
		return(no_ret);
		}
	if (obj->get_shapenum() == 797)
		gump = new Scroll_gump();
	else
		gump = new Book_gump();
	set_book(gump);
	return(no_ret);
}

USECODE_INTRINSIC(cause_light)
{
	// Cause_light(game_minutes??)

	gwin->add_special_light(parms[0].get_int_value());
	return no_ret;
}

USECODE_INTRINSIC(get_barge)
{
	// get_barge(obj) - returns barge object is part of or lying on.

	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	return Usecode_value((long) Get_barge(obj));
}

USECODE_INTRINSIC(earthquake)
{
	int len = parms[0].get_int_value();
	gwin->get_tqueue()->add(SDL_GetTicks() + 10,
		new Earthquake(len), (long) this);
	return(no_ret);
}

USECODE_INTRINSIC(is_pc_female)
{
	// Is player female?
	Usecode_value u(gwin->get_main_actor()->get_type_flag(Actor::tf_sex));
	return(u);
}

USECODE_INTRINSIC(halt_scheduled)
{
	// Halt_scheduled(item)
#if 1	/* May be okay with no_halt flag enabled. */
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return(no_ret);
					// Taking a >complete< guess here:
	Scheduled_usecode *uc;
	if ((uc = Scheduled_usecode::find(obj)) != 0)
		uc->halt();
#endif
	return(no_ret);
}

USECODE_INTRINSIC(get_array_size)
{
	int cnt;
	if (parms[0].is_array())	// An array?  We might return 0.
		cnt = parms[0].get_array_size();
	else				// Not an array?  Usecode wants a 1.
		cnt = 1;
	Usecode_value u(cnt);
	return(u);
}

USECODE_INTRINSIC(mark_virtue_stone)
{
	Game_object *obj = get_item(parms[0]);
	int frnum;
	if (obj && obj->get_shapenum() == 330 &&
	    (frnum = obj->get_framenum()) < 8)
		virtue_stones[frnum] = 
				obj->get_outermost()->get_abs_tile_coord();
	return no_ret;
}

USECODE_INTRINSIC(recall_virtue_stone)
{
	Game_object *obj = get_item(parms[0]);
	int frnum;
	if (obj && obj->get_shapenum() == 330 &&
	    (frnum = obj->get_framenum()) < 8)
		{
					// Pick it up if necessary.
		Game_object *owner = obj->get_outermost();
		if (!npc_in_party(owner))
			{		// Go through whole party.
			obj->remove_this(1);
			Usecode_value party = get_party();
			int cnt = party.get_array_size();
			int i;
			for (i = 0; i < cnt; i++)
				{
				Game_object *npc = get_item(party.get_elem(i));
				if (npc && npc->add(obj))
					break;
				}
			if (i == cnt)	// Failed?  Force it on Avatar.
				gwin->get_main_actor()->add(obj, 1);
			}
		Tile_coord t = virtue_stones[frnum];
		if (t.tx > 0 || t.ty > 0)
			gwin->teleport_party(t);
		}
	return no_ret;
}

USECODE_INTRINSIC(is_pc_inside)
{
	Usecode_value u(gwin->is_main_actor_inside());
	return(u);
}

USECODE_INTRINSIC(get_timer)
{
	int tnum = parms[0].get_int_value();
	int ret;
	if (tnum >= 0 && tnum < (int)(sizeof(timers)/sizeof(timers[0])))
		ret = gwin->get_total_hours() - timers[tnum];
	else
		{
		cerr << "Attempt to use invalid timer " << tnum << endl;
		ret = 0;
		}
	return Usecode_value(ret);
}

USECODE_INTRINSIC(set_timer)
{
	int tnum = parms[0].get_int_value();
	if (tnum >= 0 && tnum < (int)(sizeof(timers)/sizeof(timers[0])))
		timers[tnum] = gwin->get_total_hours();
	else
		cerr << "Attempt to use invalid timer " << tnum << endl;
	return(no_ret);
}

USECODE_INTRINSIC(wearing_fellowship)
{
	Game_object *obj = gwin->get_main_actor()->get_readied(Actor::neck);
	if (obj && obj->get_shapenum() == 955 && obj->get_framenum() == 1)
		return Usecode_value(1);
	else
		return Usecode_value(0);
}

USECODE_INTRINSIC(mouse_exists)
{
	Usecode_value u(1);
	return(u);
}

USECODE_INTRINSIC(get_speech_track)
{
	// Get speech track set by 0x74 or 0x8f.
	return Usecode_value(speech_track);
}

USECODE_INTRINSIC(flash_mouse)
{
	// flash_mouse(mouse_shape).
	mouse->flash_shape((Mouse::Mouse_shapes) parms[0].get_int_value());
	return (no_ret);
}

USECODE_INTRINSIC(get_item_frame_rot)
{
	// Same as get_item_frame, but (guessing!) leave rotated bit.
	Game_object *obj = get_item(parms[0]);
	return Usecode_value(obj ? obj->get_framenum() : 0);
}

USECODE_INTRINSIC(okay_to_fly)
{
	// Only used once, in usecode for magic-carpet.
	return Usecode_value(1);
}

USECODE_INTRINSIC(get_container)
{
	// Takes itemref, returns container.
	Game_object *obj = get_item(parms[0]);
	Usecode_value u(0);
	if (obj)
		u = Usecode_value((long) obj->get_owner());
	return(u);
}

USECODE_INTRINSIC(remove_item)
{
	// ?Think it's 'delete object'.
	remove_item(get_item(parms[0]));
	return(no_ret);
}

USECODE_INTRINSIC(is_readied)
{
	// is_readied(npc, where, itemshape, frame (-359=any)).
	// Where:
	//   1=weapon hand, 
	//   2=other hand,
	//   6=one finger, 
	//   7=other finger,
	//   9=head
	//  20=???

	Actor *npc = as_actor(get_item(parms[0]));
	if (!npc)
		return Usecode_value(0);
	int where = parms[1].get_int_value();
	int shnum = parms[2].get_int_value();
	int frnum = parms[3].get_int_value();
	int spot;			// Spot defined in Actor class.
	switch(where)
		{
	case 1:
		spot = Actor::lhand; break;
	case 2:
		spot = Actor::rhand; break;
	case 6:
		spot = Actor::lfinger; break;
	case 7:
		spot = Actor::rfinger; break;
	case 9:
		spot = Actor::head; break; 
	default:
		cerr << "Is_readied: spot #" << where <<
						" not known yet" << endl;
		spot = -1;
		break;
		}
	if (spot >= 0)
		{			// See if it's the right one.
		Game_object *obj = npc->get_readied(spot);
		if (obj && obj->get_shapenum() == shnum &&
		    (frnum == -359 || obj->get_framenum() == frnum))
			return Usecode_value(1);
		}
	return Usecode_value(0);
}

USECODE_INTRINSIC(restart_game)
{
	// Think it's 'restart game'.  
	// Happens if you die before leaving trinsic.
	// +++++++Got to set a flag that's looked at higher up.  Too deeply
	//        nested to restart here.
	cout << "Got to restart game." << endl;
	return(no_ret);
}

USECODE_INTRINSIC(start_speech)
{
	// Start_speech(num).  Also sets speech_track.
	bool okay = false;
	speech_track = parms[0].get_int_value();
	if (speech_track >= 0)
		okay = audio->start_speech(speech_track);
	return(Usecode_value(okay ? 1 : 0));
}

USECODE_INTRINSIC(run_endgame)
{
	Game::get_game()->end_game(parms[0].get_int_value() != 0);
	// If successful play credits afterwards
	if(parms[0].get_int_value() != 0)
		Game::get_game()->show_credits();
	quitting_time = 1;
	return(no_ret);
}

USECODE_INTRINSIC(nap_time)
{
	// nap_time(bed)
	char *msgs[] = {"Avatar!  Please restrain thyself!",
			"Hast thou noticed that this bed is occupied?",
			"Please, Avatar, the resident of this bed may not be desirouth of company at the moment."
			};
	const int nummsgs = sizeof(msgs)/sizeof(msgs[0]);
	Game_object *bed = get_item(parms[0]);
	if (!bed)
		return no_ret;
					// !!! Seems 622 handles sleeping.
	Vector npcs;			// See if bed is occupied.
	int cnt = bed->find_nearby(npcs, -359, 0, 0);
	if (cnt > 0)
		{
		int i;
		for (i = 0; i < cnt; i++)
			{
			Game_object *npc = (Game_object *) npcs.get(i);
			int zdiff = npc->get_lift() - bed->get_lift();
			if (npc != gwin->get_main_actor() &&
						zdiff <= 2 && zdiff >= -2)
				break;	// Found one.
			}
		if (i < cnt)
			{		// Show party member's face.
			int npcnum = get_party_member(
						rand()%get_party_count());
			Usecode_value actval(-npcnum), frval(0);
			show_npc_face(actval, frval);
			gwin->show_npc_message(msgs[rand()%nummsgs]);
			remove_npc_face(actval);
			return no_ret;
			}
		}
	call_usecode(0x622, bed, double_click);
	return(no_ret);
}

USECODE_INTRINSIC(advance_time)
{
	// Incr. clock by (parm[0]*.04min.).
	gwin->increment_clock(parms[0].get_int_value()/25);
	return(no_ret);
}

USECODE_INTRINSIC(in_usecode)
{
	// in_usecode(item):  Return 1 if executing usecode on parms[0].

	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	return Usecode_value(Scheduled_usecode::find(obj) != 0);
}

USECODE_INTRINSIC(path_run_usecode)
{
	// exec(loc(x,y,z)?, usecode#, itemref, eventid).
	// Think it should have Avatar walk path to loc, return 0
	//  if he can't get there (and return), 1 if he can.
	Usecode_value u(0);
	Usecode_value& loc = parms[0];
	int sz = loc.get_array_size();
	if (sz == 3)			// Looks like tile coords.
		{
					// Get source, dest.
		Tile_coord src = gwin->get_main_actor()->get_abs_tile_coord();
		int dx = loc.get_elem(0).get_int_value();
		int dy = loc.get_elem(1).get_int_value();
		int dz = loc.get_elem(2).get_int_value();
		Tile_coord dest(dx, dy, dz);
		cout << endl << "Paty_run_usecode:  first walk to (" << 
			dx << ", " << dy << ", " << dz << ")" << endl;
		if (src != dest &&
		    !gwin->get_main_actor()->walk_path_to_tile(dest))
			{		// Failed to find path.  Return 0.
			cout << "Failed to find path" << endl;
			return(u);
			}
		}
	else
		{	//++++++Not sure about this.
		cout << "0x7d Location not a 3-int array" << endl;
		return(u);	// Return 0.
		}
	Wait_for_arrival(gwin->get_main_actor());
	Game_object *obj = get_item(parms[2]);
	if (obj)
		{
		call_usecode(parms[1].get_int_value(), obj, 
				(Usecode_events) parms[3].get_int_value());
		u = Usecode_value(1);	// Success.
		}
	return(u);
}

USECODE_INTRINSIC(close_gumps)
{
	// Guessing+++++ close all gumps.
	gwin->end_gump_mode();
	return(no_ret);
}

USECODE_INTRINSIC(in_gump_mode)
{
	return Usecode_value(gwin->get_mode() == Game_window::gump);
}

USECODE_INTRINSIC(is_not_blocked)
{
	// Is_not_blocked(tile, shape, frame (or -359).
	Usecode_value fail(0);
					// Parm. 0 should be tile coords.
	Usecode_value& pval = parms[0];
	if (pval.get_array_size() < 3)
		return fail;
	Tile_coord lcpos(-1, -1, -1);	// Don't let last_created block.
	if (last_created && !last_created->get_owner() &&
	    gwin->get_info(last_created).is_solid())
		{
		lcpos = last_created->get_abs_tile_coord();
		last_created->remove_this(1);
		last_created->set_invalid();
		}
	Tile_coord tile(pval.get_elem(0).get_int_value(),
			pval.get_elem(1).get_int_value(),
			pval.get_elem(2).get_int_value());
	int shapenum = parms[1].get_int_value();
#if 0	/* Unused for now. */
	int framenum = parms[2].get_int_value();
#endif
					// Find out about given shape.
	Shape_info& info = gwin->get_info(shapenum);
	int new_lift;
	int blocked = Chunk_object_list::is_blocked(
		info.get_3d_height(), tile.tz, 
		tile.tx - info.get_3d_xtiles() + 1,
		tile.ty - info.get_3d_ytiles() + 1,
		info.get_3d_xtiles(), info.get_3d_ytiles(), 
		new_lift, MOVE_ALL_TERRAIN);
//Don't know why this is causing trouble in Forge with mage at end:
//	blocked = (blocked || new_lift != tile.tz);
	if (lcpos.tx != -1)		// Put back last_created.
		last_created->move(lcpos);
	return Usecode_value(!blocked);
}

USECODE_INTRINSIC(direction_from)
{
	// ?Direction from parm[0] -> parm[1].
	// Rets. 0-7, with 0 = North, 1 = Northeast, etc.
	// Same as 0x1a??
	Usecode_value u=find_direction(parms[0], parms[1]);
	return(u);
}

USECODE_INTRINSIC(get_item_flag)
{
	// Get npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	if (!obj)
		return Usecode_value(0);
	int fnum = parms[1].get_int_value();
					// Special cases:
	if (fnum == (int) Actor::on_moving_barge ||
	    fnum == (int) Actor::in_motion)
		{			// Test for moving barge.
		Barge_object *barge;
		if (!gwin->get_moving_barge() || !(barge = Get_barge(obj)))
			return Usecode_value(0);
		return Usecode_value(barge == gwin->get_moving_barge());
		}
	else if (fnum == (int) Actor::okay_to_land)
		{			// Okay to land flying carpet?
		Barge_object *barge = Get_barge(obj);
		if (!barge || barge != gwin->get_moving_barge())
			return Usecode_value(0);
		return Usecode_value(barge->okay_to_land());
		}
	Usecode_value u(obj->get_flag(fnum));
	return(u);
}

USECODE_INTRINSIC(set_item_flag)
{
	// Set npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	int flag = parms[1].get_int_value();
	if (obj)
		{
		obj->set_flag(flag);
		if (flag == Actor::dont_render)
			{	// Show change in status.
			gwin->paint();
			gwin->show();
			}
		else if (flag == (int) Actor::on_moving_barge ||
					flag == (int) Actor::in_motion)
			{	// Set barge in motion.
			Barge_object *barge = Get_barge(obj);
			if (barge)
				gwin->set_moving_barge(barge);
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(clear_item_flag)
{
	// Clear npc flag(item, flag#).
	Game_object *obj = get_item(parms[0]);
	int flag = parms[1].get_int_value();
	if (obj)
		{
		obj->clear_flag(flag);
		if (flag == Actor::dont_render)
			{	// Show change in status.
			gwin->paint();
			gwin->show();
			}
		else if (flag == (int) Actor::on_moving_barge ||
					flag == (int) Actor::in_motion)
			{	// Stop barge object is on or part of.
			Barge_object *barge = Get_barge(obj);
			if (barge && barge == gwin->get_moving_barge())
				gwin->set_moving_barge(0);
			}
		}
	return(no_ret);
}

USECODE_INTRINSIC(run_usecode)
{
	// run_usecode(fun, itemref, eventid)
	Game_object *obj = get_item(parms[1]);
	if (obj)
		call_usecode(parms[0].get_int_value(), obj, 
				(Usecode_events) parms[2].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(fade_palette)
{
	// Fade(cycles?, ??(always 1), in_out (0=fade to black, 1=fade in)).
	int cycles = parms[0].get_int_value();
	int inout = parms[2].get_int_value();
	gwin->fade_palette(cycles, inout);
	return(no_ret);
}

USECODE_INTRINSIC(get_party_list2)
{
	// Return party.  Same as 0x23
	// Probably returns a list of everyone with (or without) some flag
	// List of live chars? Dead chars?
	Usecode_value u(get_party());
	return(u);
}

USECODE_INTRINSIC(in_combat)
{
	// Are we in combat mode?
	return Usecode_value(gwin->in_combat());
}

USECODE_INTRINSIC(get_dead_party)
{
	// Return list of dead companions' bodies.
	Dead_body *list[10];
	int cnt = Dead_body::find_dead_companions(list);
	Usecode_value ret(cnt, 0);
	for (int i = 0; i < cnt; i++)
		{
		Usecode_value v((long) list[i]);
		ret.put_elem(i, v);
		}
	return ret;
}

USECODE_INTRINSIC(play_sound_effect)
{
	if (num_parms < 1) return(no_ret);
	// Play music(item, songnum).
	// ??Show notes by item?
#if DEBUG
	cout << "Sound effect " << parms[0].get_int_value() << " request in usecode" << endl;
#endif
	audio->play_sound_effect (parms[0].get_int_value());
	return(no_ret);
}

USECODE_INTRINSIC(get_npc_id)
{
	// GetSchedule(npc).  Rets. schedtype.
	Actor *obj = (Actor *) get_item(parms[0]);
	Usecode_value u(obj ? obj->get_ident() : 0);
	return(u);
}

USECODE_INTRINSIC(set_npc_id)
{
	Actor *obj = (Actor *) get_item(parms[0]);
	if (obj)
		obj->set_ident(parms[1].get_int_value());

	return(no_ret);
}


USECODE_INTRINSIC(add_cont_items)
{
	// Add items(num, item, ??quality?? (-359), frame (or -359), T/F).
	add_cont_items(parms[0], parms[1], parms[2],
					parms[3], parms[4], parms[5]);
	return(no_ret);
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
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4a
	USECODE_INTRINSIC_PTR(set_attack_mode),	// 0x4b
	USECODE_INTRINSIC_PTR(set_opponent),	// 0x4c
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4d     CloneNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x4e UNUSED
	USECODE_INTRINSIC_PTR(display_area), // 0x4f// ShowCrystalBall(ucdump)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x50     ShowWizardEye (ucdump.c)
	USECODE_INTRINSIC_PTR(resurrect),// 0x51     ResurrectNPC (ucdump.c)
	USECODE_INTRINSIC_PTR(add_spell),// 0x52     AddSpellToBook (ucdump.c)
	USECODE_INTRINSIC_PTR(sprite_effect),// 0x53 ExecuteSprite (ucdump.c)
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x54  ++++Explode???
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
	USECODE_INTRINSIC_PTR(UNKNOWN),	// 0x7c
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
	funs = new Vector(10);		// A slot for funs. n/256.
					// Read in all the functions.
	while (file.tellg() < size)
		{
		Usecode_function *fun = new Usecode_function(file);
					// Get slot.
		Vector *slot = (Vector *) funs->get(fun->id/0x100);
		if (!slot)
			funs->put(fun->id/0x100, (slot = new Vector(10)));
					// Store in slot.
		slot->put(fun->id%0x100, fun);
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
	delete String;
	delete removed;
	int num_slots = funs->get_cnt();
	for (int i = 0; i < num_slots; i++)
		{
		Vector *slot = (Vector *) funs->get(i);
		if (!slot)
			continue;
		int cnt = slot->get_cnt();
		for (int j = 0; j < cnt; j++)
			delete (Usecode_function *) slot->get(j);
		delete slot;
		}
	delete funs;
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
	int event			// Event (??) that caused this call.
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
		if (debug >= 2)
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
					256*externals[2*offset + 1], event))
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
			pushi((long) caller_item);
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
			if (!call_usecode_function(offset, event))
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
	Usecode_value *parm0		// If non-zero, pass this parm.
	)
	{
					// Nothing going on?
	if (!call_depth && !Scheduled_usecode::get_count())
		removed->flush();	// Flush removed objects.
					// Look up in table.
	Vector *slot = (Vector *) funs->get(id/0x100);
	Usecode_function *fun = slot ? (Usecode_function *) slot->get(id%0x100)
				     : 0;
	if (!fun)
		{
		cout << "Usecode " << id << " not found."<<endl;
		return (-1);
		}
	if (parm0)
		push(*parm0);
	return run(fun, event);		// Do it.  Rets. 0 if aborted.
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
	Usecode_value parm(0);		// They all seem to take 1 parm.
	int ret = call_usecode_function(id, event, &parm);
	set_book(0);
	caller_item = prev_item;
	return ret;
	}

/*
 *	Write out global data to 'gamedat/usecode.dat'.
 *
 *	Output:	0 if error.
 */

int Usecode_machine::write
	(
	)
	{
	ofstream out;
	if (!U7open(out, FLAGINIT))	// Write global flags.
		return (0);
	out.write((char*)gflags, sizeof(gflags));
	out.close();
	if (!U7open(out, USEDAT))
		return (0);
	Write2(out, party_count);	// Write party.
	for (size_t i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		Write2(out, party[i]);
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		Write4(out, timers[t]);
	for (size_t i = 0; i < 8; i++)	// Virtue stones.
		{
		Write2(out, virtue_stones[i].tx);
		Write2(out, virtue_stones[i].ty);
		Write2(out, virtue_stones[i].tz);
		}
	out.flush();
	return out.good();
	}

/*
 *	Read in global data to 'gamedat/usecode.dat'.
 *
 *	Output:	0 if error.
 */

int Usecode_machine::read
	(
	)
	{
	ifstream in;
	if (!U7open(in, FLAGINIT))	// Read global flags.
					// +++++Eventually, remove this:
		if (!U7open(in, "<STATIC>/flaginit.dat"))
			return (0);
	in.read((char*)gflags, sizeof(gflags));
	in.close();
	if (!U7open(in, USEDAT))
		return (1);		// Not an error if no saved game yet.
	party_count = Read2(in);	// Read party.
	for (size_t i = 0; i < sizeof(party)/sizeof(party[0]); i++)
		party[i] = Read2(in);
	link_party();
					// Timers.
	for (size_t t = 0; t < sizeof(timers)/sizeof(timers[0]); t++)
		timers[t] = Read4(in);
	int result = in.good();		// ++++Just added timers.
	for (size_t i = 0; i < 8; i++)	// Virtue stones.
		{
		virtue_stones[i].tx = Read2(in);
		virtue_stones[i].ty = Read2(in);
		virtue_stones[i].tz = Read2(in);
		}
	if (!in.good())			// Failed.+++++Can remove this later.
		for (size_t i = 0; i < 8; i++)
			virtue_stones[i] = Tile_coord(0, 0, 0);
	return result;
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
