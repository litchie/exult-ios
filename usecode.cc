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
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "usecode.h"
#include "gamewin.h"
#include "objs.h"
#include "vec.h"
#include "SDL.h"
#include "tqueue.h"
#include "gumps.h"
#include "mouse.h"
#include <Audio.h>

/*
 *	A class for executing usecode at a scheduled time:
 */
class Scheduled_usecode : public Time_sensitive
	{
	Usecode_value objval;		// The itemref.
	Usecode_value arrval;		// Array of code to execute.
public:
	Scheduled_usecode(Usecode_value& oval, Usecode_value& aval)
		: objval(oval), arrval(aval)
		{  }
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Execute an array of usecode.
 */

void Scheduled_usecode::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// ->usecode machine.
	)
	{
	Usecode_machine *usecode = (Usecode_machine *) udata;
	Game_window *gwin = usecode->gwin;
	usecode->exec_array(objval, arrval);
	if (gwin->get_mode() == Game_window::conversation)
		{
		gwin->set_mode(Game_window::normal);
		gwin->paint();
		}
	delete this;			// Hope this is safe.
	}

/*
 *	Get array size.
 */

int Usecode_value::count_array
	(
	const Usecode_value& val
	)
	{
	int i;
	for (i = 0; val.value.array[i].type != (int) end_of_array_type; i++)
		;
	return (i);
	}

/*
 *	Resize array.
 *
 *	Output:	false if not an array.
 */

int Usecode_value::resize
	(
	int new_size
	)
	{
	if (type != (int) array_type)
		return (0);
	int size = count_array(*this);	// Get current size.
	if (new_size == size)
		return (1);		// Nothing to do.
	Usecode_value *newvals = new Usecode_value[new_size + 1];
	newvals[new_size].type = (unsigned char) end_of_array_type;
					// Move old values over.
	int cnt = new_size < size ? new_size : size;
	for (int i = 0; i < cnt; i++)
		newvals[i] = value.array[i];
	delete [] value.array;		// Delete old list.
	value.array = newvals;		// Store new.
	return (1);
	}

/*
 *	Comparator.
 *
 *	Output:	1 if they match, else 0.
 */

int Usecode_value::operator==
	(
	const Usecode_value& v2
	)
	{
	if (&v2 == this)
		return (1);		// Same object.
	if (v2.type != type)
		return (0);		// Wrong type.
	if (type == (int) int_type)
		return (value.intval == v2.value.intval);
	else if (type == (int) string_type)
		return (strcmp(value.str, v2.value.str) == 0);
	else
		return (0);
	}

/*
 *	Search an array for a given value.
 *
 *	Output:	Index if found, else -1.
 */

int Usecode_value::find_elem
	(
	const Usecode_value& val
	)
	{
	if (type != array_type)
		return (-1);		// Not an array.
	int i;
	for (i = 0; value.array[i].type != (int) end_of_array_type; i++)
		if (value.array[i] == val)
			return (i);
	return (-1);
	}

/*
 *	Concatenate two values.
 */

Usecode_value& Usecode_value::concat
	(
	Usecode_value& val2		// Concat. val2 onto end.
	)
	{
	Usecode_value *result;
	int size;			// Size of result.
	if (type != array_type)		// Not an array?  Create one.
		{
		result = new Usecode_value(1, this);
		size = 1;
		}
	else
		{
		result = this;
		size = get_array_size();
		}
	if (val2.type != array_type)	// Appending a single value?
		{
		result->resize(size + 1);
		result->put_elem(size, val2);
		}
	else				// Appending an array.
		{
		int size2 = val2.get_array_size();
		result->resize(size + size2);
		for (int i = 0; i < size2; i++)
			result->put_elem(size + i, val2.get_elem(i));
		}
	return (*result);
	}

/*
 *	Print in ASCII.
 */

void Usecode_value::print
	(
	ostream& out
	)
	{
	switch ((Val_type) type)
		{
	case int_type:
		out << (value.intval&0xffff); break;
	case string_type:
		out << '"' << value.str << '"'; break;
	case array_type:
		{
		for (int i = 0; value.array[i].type != (int) end_of_array_type;
									i++)
			{
			if (i)
				out << ", ";
			value.array[i].print(out);
			}
		}
		break;
	default:
		break;
		}
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
	file.read(code, len);
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
	Usecode_value& val		// String.
	)
	{
	const char *str = val.get_str_value();
	if (!str)
		return;
	_remove_answer(str);
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

/*
 *	Get a game object from an "itemref", which might be the actual
 *	pointer, or might be -(npc number).
 *
 *	Output:	->game object.
 */

Game_object *Usecode_machine::get_item
	(
	long val
	)
	{
	Game_object *obj = 0;
	if (val == -356)		// +++++Avatar.  Define in .h file.
		return gwin->get_main_actor();
	if (val < 0)
		obj = gwin->get_npc(-val);
	else if (val < gwin->get_num_npcs())
		obj = gwin->get_npc(val);
	return obj ? obj : (Game_object *) val;
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
					// Make sure prev. text was seen.
	if (gwin->is_npc_text_pending())
		click_to_continue();
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
		cerr << "Stack underflow.\n";
	else
		cerr << "Stack overflow.\n";
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
	if (gwin->is_npc_text_pending())
		click_to_continue();
	Actor *npc = (Actor *) get_item(arg1.get_int_value());
	if (!npc)
		return;
	int shape = npc->get_face_shapenum();
	int frame = arg2.get_int_value();
	gwin->show_face(shape, frame);
	cout << "Show face " << shape << '\n';
	}

/*
 *	Remove an NPC's face.
 */

void Usecode_machine::remove_npc_face
	(
	Usecode_value& arg1		// Shape (NPC #).
	)
	{
	if (gwin->is_npc_text_pending())
		click_to_continue();
	int shape = -arg1.get_int_value();
	gwin->remove_face(shape);
	cout << "Remove face " << shape << '\n';
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
	int val = item_arg.get_int_value();
	int shape = shape_arg.get_int_value();
	cout << "Set_item_shape: " << val << ", " << shape << '\n';
	Game_object *item = get_item(val);
	if (item != 0)
		{
		Chunk_object_list *chunk = gwin->get_objects(item);
		chunk->remove(item);	// Remove and add to update cache.
		item->set_shape(shape);
		chunk->add(item);
		gwin->paint();		// Not sure...
		gwin->show();		// +++++++
		}
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
	Game_object *item = get_item(item_arg.get_int_value());
	int frame = frame_arg.get_int_value();
	cout << "Set_item_frame: " << item->get_shapenum() 
					<< ", " << frame << '\n';
	if (item != 0 && 
	    frame < gwin->get_shape_num_frames(item->get_shapenum()))
		item->set_frame(frame);
//+++++Testing
	gwin->paint();
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
	Game_object *item = get_item(item_arg.get_int_value());
	return (item == 0 ? 0 : item->get_shapenum());
	}

/*
 *	Get an item's frame.
 */

int Usecode_machine::get_item_frame
	(
	Usecode_value& item_arg
	)
	{
	Game_object *item = get_item(item_arg.get_int_value());
	return (item == 0 ? 0 : item->get_framenum());
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
	if (!npc || !party_count)
		return (0);
	int npcnum = npc->get_npc_num();
cout << "Is npc " << npc << " in party?  ";
	for (int i = 0; i < PARTY_MAX; i++)
		if (party[i] == npcnum)
			{
			cout << "Yes\n";
			return (1);
			}
	cout << "No\n";
	return (0);
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
	for (int i = 0; i < PARTY_MAX; i++)
		if (party[i] == 0)	// Find empty spot.
			{
			party[i] = npc->get_npc_num();
			party_count++;
cout << "NPC " << npc->get_npc_num() << " added to party.\n";
			break;
			}
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
	int npcnum = npc->get_npc_num();
	for (int i = 0; i < PARTY_MAX; i++)
		if (party[i] == npcnum)
			{
			party[i] = 0;
			party_count--;
			break;
			}
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
	for (int i = 0; i < PARTY_MAX && num_added < 1 + party_count; i++)
		if (party[i] != 0)
			{
			Game_object *obj = gwin->get_npc(party[i]);
			if (!obj)
				continue;
			Usecode_value val((long) obj);
			arr.put_elem(num_added, val);
			}
	cout << "Party:  "; arr.print(cout); cout << '\n';
	return arr;
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
	Game_object *obj = get_item(objval.get_int_value());
	const char *str = strval.get_str_value();
	if (obj && str && *str)
		{
		Rectangle box = gwin->get_shape_rect(obj);
		gwin->add_text((char *)str, box.x, box.y);	// &&& Fix me later and avoid the ugly cast
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
	Usecode_value& qval,		// Quality??
	Usecode_value& mval		// Some kind of mask?
	)
	{
	Game_object *obj = get_item(objval.get_int_value());
	if (!obj)
		return Usecode_value(0, 0);
	Vector vec;			// Gets list.
	int cnt = obj->find_nearby(vec, shapeval.get_int_value(),
			qval.get_int_value(), mval.get_int_value());
	cout << "Nearby objects found:  " << cnt << '\n';
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
 *	Find the angle (0-7) from one object to another.
 *	++++++Not sure which dir.  0 represents.  Assuming East for now.
 */

Usecode_value Usecode_machine::find_direction
	(
	Usecode_value& from,
	Usecode_value& to
	)
	{
	unsigned angle;			// Gets angle 0-7 (east - southeast).
	Game_object *o1 = get_item(from.get_int_value());
	Game_object *o2 = get_item(to.get_int_value());
	if (!o1 || !o2)
		angle = 0;
	else
		{			// Figure angle from positions.
		int x1, y1, z1, x2, y2, z2;
		o1->get_abs_tile(x1, y1, z1);
		o2->get_abs_tile(x2, y2, z2);
		angle = (int) Get_direction(y2 - y1, x2 - x1);
		}
	return Usecode_value(angle);
	}

/*
 *	Count objects of a given shape in a container, or in the whole party.
 */

Usecode_value Usecode_machine::count_objects
	(
	Usecode_value& objval,		// The container, or -357 for party.
	Usecode_value& shapeval		// Object shape to count (-359=any).
	)
	{
	long oval = objval.get_int_value();
	int shapenum = shapeval.get_int_value();
	if (oval != -357)
		{
		Game_object *obj = get_item(oval);
		return (!obj ? 0 : obj->count_objects(shapenum));
		}
					// Look through whole party.
	Usecode_value party = get_party();
	int cnt = party.get_array_size();
	int total = 0;
	for (int i = 0; i < cnt; i++)
		{
		Game_object *obj = get_item(party.get_elem(i).get_int_value());
		if (obj)
			total += obj->count_objects(shapenum);
		}
	return (total);
	}

/*
 *	Get objects of a given shape in a container.
 */

Usecode_value Usecode_machine::get_objects
	(
	Usecode_value& objval,		// The container.
	Usecode_value& shapeval		// Object shape to get or -359 for any.
	)
	{
	Game_object *obj = get_item(objval.get_int_value());
	if (!obj)
		return Usecode_value(0);
	int shapenum = shapeval.get_int_value();
	Vector vec;			// Gets list.
	int cnt = obj->get_objects(vec, shapenum);
	cout << "Container objects found:  " << cnt << '\n';
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
 *	Have the user choose an object with the mouse.
 *
 *	Output:	Ref. to item, or 0.
 */

Usecode_value Usecode_machine::click_on_item
	(
	)
	{
	extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape);
	cout << "CLICK on an item.\n";
	int x, y;
	if (!Get_click(x, y, Mouse::greenselect))
		return Usecode_value(0);
	Game_object *found[100];	// See what was clicked on.
	int cnt;
					// Look for obj. in open gump.
	Gump_object *gump = gwin->find_gump(x, y);
	if (gump)
		cnt = gump->find_objects(gwin, x, y, found);
	else				// Search rest of world.
		cnt = gwin->find_objects(x, y, found);
	if (cnt)
		return Usecode_value((long) found[cnt - 1]);
	else
		return Usecode_value(0);
	}

/*
 *	Execute a list of instructions in an array.
 */

void Usecode_machine::exec_array
	(
	Usecode_value& objval,
	Usecode_value& arrayval		// Contains instructions.
	)
	{
	Game_object *obj = get_item(objval.get_int_value());
	if (!obj)
		return;
	int cnt = arrayval.get_array_size();
	for (int i = 0; i < cnt; i++)	// Go through instructions.
		{
					// Let's try to animate.
		gwin->get_tqueue()->activate(SDL_GetTicks());
		gwin->show();		// Blit to screen if necessary.
		Usecode_value& opval = arrayval.get_elem(i);
		int opcode = opval.get_int_value();
		switch (opcode)
			{
		case 0x01:		// ??
			break;
		case 0x0b:		// ?? 2 parms, 1st one < 0.
			i += 2;
			break;
		case 0x23:		// ??
			break;
		case 0x27:		// ?? 1 parm. Pure guess:  a delay to
					//   allow other threads to run?
			i++;
			break;
		case 0x2d:		// ?? Remove itemref?
cout << "0x2d:  Deleting itemref\n";
			gwin->get_objects(obj->get_cx(), obj->get_cy())->
						remove(obj);
			delete obj;
			obj = 0;
			gwin->paint();
			break;
		case 0x46:		// ?? 1 parm. This IS a frame.
			{		// Set frame?  Pretty sure.
			Usecode_value& fval = arrayval.get_elem(++i);
			set_item_frame(objval, fval);
			break;
			}
		case 0x50:		// ??
			break;
		case 0x52:		// Say string.
			{
			Usecode_value& strval = arrayval.get_elem(++i);
			item_say(objval, strval);
			break;
			}
		case 0x55:		// Call?
			{
			Usecode_value& val = arrayval.get_elem(++i);
			call_usecode(val.get_int_value(), obj, 
						internal_exec);
			break;
			}
		case 0x58:		// ?? 1 parm.
			i++;
			break;
		case 0x59:		// Parm. is dir. (0-7).  0=north?
				// +++++++Walk in that dir.??
			i++;
			break;
		default:
			break;
			}
		}
	}

/*
 *	Report unhandled intrinsic.
 */

static void Unhandled
	(
	int intrinsic,
	int num_parms,
	Usecode_value parms[12]
	)
	{
	printf("Unhandled intrinsic 0x%03x called with %d parms: ",
					intrinsic, num_parms);
	for (int i = 0; i < num_parms; i++)
		{
		parms[i].print(cout);
		cout << ' ';
		}
	cout << '\n';
	}

static Usecode_value	no_ret;
Usecode_value	Usecode_machine::UI_NOP( int event,int intrinsic,Usecode_value parms[12])
{
	return no_ret;
}

Usecode_value	Usecode_machine::UI_UNKNOWN(int event,int intrinsic,Usecode_value parms[12])
{
	return no_ret;
}

Usecode_value	Usecode_machine::UI_get_random(int event,int intrinsic,Usecode_value parms[12])
{
	int range = parms[0].get_int_value();
	if (range == 0)
		return Usecode_value(0);
	return Usecode_value(1 + (rand() % range));
}

Usecode_value	Usecode_machine::UI_execute_usecode_array(int event,int intrinsic,Usecode_value parms[12])
{
	cout << "Executing intrinsic 1\n";
	exec_array(parms[0], parms[1]);
	return no_ret;
}

Usecode_value	Usecode_machine::UI_delayed_execute_usecode_array(int event,int intrinsic,Usecode_value parms[12])
{			// Delay = .20 sec.?
	int delay = parms[2].get_int_value();
	gwin->get_tqueue()->add(SDL_GetTicks() + 200*delay,
		new Scheduled_usecode(parms[0], parms[1]),
							(long) this);
	cout << "Executing intrinsic 2\n";
	return no_ret;
}

Usecode_value	Usecode_machine::UI_show_npc_face(int event,int intrinsic,Usecode_value parms[12])
{
	show_npc_face(parms[0], parms[1]);
	return no_ret;
}

Usecode_value	Usecode_machine::UI_remove_npc_face(int event,int intrinsic,Usecode_value parms[12])
{
	remove_npc_face(parms[0]);
	return no_ret;
}

Usecode_value	Usecode_machine::UI_add_answer(int event,int intrinsic,Usecode_value parms[12])
{
	answers.add_answer(parms[0]);
	return no_ret;
}
Usecode_value	Usecode_machine::UI_remove_answer(int event,int intrinsic,Usecode_value parms[12])
{
	answers.remove_answer(parms[0]);
	return no_ret;
}

Usecode_value	Usecode_machine::UI_push_answers(int event,int intrinsic,Usecode_value parms[12])
{
	answer_stack.push_front(answers);
	answers.clear();
	return no_ret;
}

Usecode_value	Usecode_machine::UI_pop_answers(int event,int intrinsic,Usecode_value parms[12])
{
	if(answer_stack.size())
		{
		answers=answer_stack.front();
		answer_stack.pop_front();
		}
	return no_ret;
}

Usecode_value	Usecode_machine::UI_select_from_menu(int event,int intrinsic,Usecode_value parms[12])
{
	user_choice = 0;
	const char *choice = get_user_choice();
	user_choice = 0;
	return choice;
}

Usecode_value	Usecode_machine::UI_select_from_menu2(int event,int intrinsic,Usecode_value parms[12])
{			// Return index (1-n) of choice.
	user_choice = 0;
	Usecode_value val(get_user_choice_num() + 1);
	user_choice = 0;
	return val;
}

typedef	Usecode_value (Usecode_machine::*UsecodeIntrinsicFn)(int event,int intrinsic,Usecode_value parms[12]);

UsecodeIntrinsicFn intrinsic_table[]=
	{
	&Usecode_machine::UI_get_random,	// 0
	&Usecode_machine::UI_execute_usecode_array, // 1
	&Usecode_machine::UI_delayed_execute_usecode_array, // 2
	&Usecode_machine::UI_show_npc_face, // 3
	&Usecode_machine::UI_remove_npc_face, // 4
	&Usecode_machine::UI_add_answer, // 5
	&Usecode_machine::UI_remove_answer, // 6
	&Usecode_machine::UI_push_answers, // 7
	&Usecode_machine::UI_pop_answers, // 8
	&Usecode_machine::UI_UNKNOWN, // 9
	&Usecode_machine::UI_select_from_menu, // 0x0a
	&Usecode_machine::UI_select_from_menu2, // 0x0b
	};

int	max_bundled_intrinsics=0xb;	// Index of the last intrinsic in this table

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
	if(intrinsic<=max_bundled_intrinsics)
		{
		UsecodeIntrinsicFn func=intrinsic_table[intrinsic];
		return ((*this).*func)(event,intrinsic,parms);
		}
	else
	switch (intrinsic)
		{
	case 0xc:			// Ask for # (min, max, step, default).
					// (Show slider.)
		//+++++++++++++
		return Usecode_value(parms[0].get_int_value() + 1);
		break;
	case 0xd:			// Set item shape.
		set_item_shape(parms[0], parms[1]);
		break;
	case 0x10:			// Rand. # within range.
		{
		int low = parms[0].get_int_value();
		int high = parms[1].get_int_value();
		if (low > high)
			{
			int tmp = low;
			low = high;
			high = tmp;
			}
		int val = (rand() % (high - low + 1)) + low;
		return (Usecode_value(val));
		}
	case 0x11:			// Get item shape.
		return (Usecode_value(get_item_shape(parms[0])));
	case 0x12:			// Get item frame.
		return (Usecode_value(get_item_frame(parms[0])));
	case 0x13:			// Set item frame.
		set_item_frame(parms[0], parms[1]);
		break;
	case 0x14:			// Get item quality.
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value(obj ? obj->get_quality() : 0);
		break;
		}
	case 0x15:			// Guessing it's 
					//  set_quality(item, value).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			obj->set_quality(parms[1].get_int_value());
		break;
		}
	case 0x16:			// Get # of items in NPC??????
					//   Count(item, -npc).
		//+++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x17:			// Set # of items??? (item, newcount).
		//+++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x18:			// Takes itemref.  ?Think it rets.
					//  hotspot coords: (x, y, z, obj).
		{
		int tx, ty, tz;		// Get tile coords.
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			obj->get_abs_tile(tx, ty, tz);
		else
			tx = ty = tz = 0;
		Usecode_value vx(tx), vy(ty), vz(tz), vobj((long) obj);
		Usecode_value arr(4, &vx);
		arr.put_elem(1, vy);
		arr.put_elem(2, vz);
		arr.put_elem(3, vobj);
		return arr;
		}
	case 0x1a:			// Direction from parm[0] -> parm[1].
					// Rets. 0-7.  Is 0 east?
		return find_direction(parms[0], parms[1]);
	case 0x1b:			// Takes -npc.  Returns object.
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value((long) obj);
		}
	case 0x1c:			// GetSchedule(npc).  Rets. schedtype.
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value(obj ? obj->get_schedule() : 0);
		}
	case 0x1d:			// SetSchedule?(npc, schedtype).
	// Looks like 15=wait here, 11=go home, 0=train/fight... This is the
	// 'bNum' field in schedules.
		Unhandled(intrinsic, num_parms, parms);
		//+++++++++++++++++++++
		break;
	case 0x1e:			// NPC joins party.
		add_to_party(get_item(parms[0].get_int_value()));
		break;
	case 0x1f:			// NPC leaves party.
		remove_from_party(get_item(parms[0].get_int_value()));
		break;
	case 0x20:			// Get NPC prop (item, prop_id).
					//   (9 is food level).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value(obj ? 
			obj->get_property(parms[1].get_int_value()) : 0);
		break;
		}
	case 0x21:			// Set NPC prop (item, prop_id, value).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			obj->set_property(parms[1].get_int_value(),
						parms[2].get_int_value());
		break;
		}
	case 0x22:			// Guessing it's Avatar's itemref.
		return Usecode_value((long) gwin->get_main_actor());
	case 0x23:			// Return array with party members.
		return (get_party());
	case 0x24:			// Takes shape, rets. new obj?
					// Show frames in seq.? (animate?)
		{
		int shapenum = parms[0].get_int_value();
					// Guessing:
		Game_object *at = gwin->get_main_actor();
#if 0
		int tx, ty, tz;		// Get location.
		at->get_abs_tile(tx, ty, tz);
		Game_object *obj = new Frame_cycle_sprite(gwin,
			shapenum, tx, ty, tz, 1);
#endif
		Game_object *obj = new Game_object(shapenum, 0,
				at->get_tx(), at->get_ty(), at->get_lift());
		gwin->get_objects(at->get_cx(), at->get_cy())->add(obj);
		gwin->show();
		return Usecode_value((long) obj);
		break;
		}
	case 0x25:			// Take itemref, rets. flag.
		//++++++++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		return Usecode_value(1); //????
		break;
	case 0x26:			// Think it takes array from 0x18,
					//   updates obj. to new pos., &
					//   renders.  ??guessing??
		{
		Usecode_value& arr = parms[0];
		int sz = arr.get_array_size();
		Game_object *obj = 0;
		if (sz == 4 && 
		    (obj = get_item(arr.get_elem(3).get_int_value())) != 0)
			obj->move(arr.get_elem(0).get_int_value(),
				  arr.get_elem(1).get_int_value(),
				  arr.get_elem(2).get_int_value());
		else
			{ cout << "Intrinsic 0x26:  "; arr.print(cout);
				cout << '\n'; }
		gwin->paint();
		gwin->show();		// ??
		return Usecode_value(1);// ??
		}
	case 0x27:			// Get NPC name.
		{	// +++++Make this work on array of NPC's.
		static char *unknown = "player";
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value(obj ? obj->get_name() : unknown);
		}
	case 0x28:			// How many?
					// ((npc?-357==party, -356=avatar), 
					//   item, quality?, quality?).
					// Quality -359 means any?
		return count_objects(parms[0], parms[1]);
	case 0x2a:			// Get cont. items(item, type, qual,?).
		return get_objects(parms[0], parms[1]);
	case 0x2b:			// Remove items(quantity, item, 
					//   -x?, -x?, T/F).  Often -359.???
		//+++++++++++
		Unhandled(intrinsic, num_parms, parms);
		return Usecode_value(1);	// ++++Pretend we did it.
	case 0x2c:			// Add items(num, item, ??, ??, T/F).
		//++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x2e:			// Play music(item, songnum).
					// ??Show notes by item?
		audio.start_music(parms[1].get_int_value()&0xff);
		break;
	case 0x2f:			// NPC in party? (item).
		return (Usecode_value(npc_in_party(
					get_item(parms[0].get_int_value()))));
	case 0x32:			// Display sign (gump #, text).
		//+++++++++++++
		// Render text into runes for signs, tombstones, plaques and the like
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x33:			// Doesn't ret. until user single-
					//   clicks on an item.  Rets. item.
		return click_on_item();
	case 0x35:			// Think it rets. objs. near parm0.
		return (find_nearby(parms[0], parms[1], parms[2], parms[3]));
	case 0x38:			// Return. game time hour (0-23).
		return Usecode_value(gwin->get_hour());
	case 0x39:			// Return minute (0-59).
		return Usecode_value(gwin->get_minute());
	case 0x3a:			// Returns NPC# of item. (-356 =
					//   avatar).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj == gwin->get_main_actor())
			return Usecode_value(-356);
		int npc = obj ? obj->get_npc_num() : 0;
		return Usecode_value(-npc);
		}
	case 0x3b:			// Return 3-hour # (0-7, 0=midnight).
		return Usecode_value(gwin->get_hour()/3);
	case 0x40:			// Show str. near item (item, str).
		item_say(parms[0], parms[1]);
		break;
	case 0x42:			// ?? Guessing rets. lift(item).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return obj ? Usecode_value(obj->get_lift())
						: Usecode_value(0);
		}
	case 0x43:			// ?? Guessing setlift(item, lift).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			{
			int x, y, z;
			obj->get_abs_tile(x, y, z);
			obj->move(x, y, parms[1].get_int_value());
			gwin->paint();
			gwin->show();
			}
		break;
		}
	case 0x48:			// Display map.
		//++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x5a:			// Is player female?
		return Usecode_value(0);
	case 0x5d:			// Start endgame.
		break;
	case 0x5e:			// Returns size of array (arr).
		return Usecode_value(parms[0].get_array_size());
	case 0x62:			// Are we inside (under a roof)?
		return Usecode_value(gwin->is_main_actor_inside());
	case 0x68:			// Returns 1 if mouse exists.
		return Usecode_value(1);
	case 0x6e:			// Takes itemref, returns obj???
//+++++++++++++++
					// Maybe it's obj's container???
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x6f:			// Animate object (w/ palette?)?
					// Or delete object?
//+++++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	case 0x72:			// Wearing? (npc, where, itemshape, 
					//   frame (-359=any)).
					// Where = (1=weapon hand, 
					//   2=other hand,
					//   6=one finger?, 
					//   7=other finger?,
					//   9=head).
		Unhandled(intrinsic, num_parms, parms);
		//+++++++++++++++++++++
		break;
	case 0x87:			// ?Direction from parm[0] -> parm[1].
					// Rets. 0-7.  Is 0 north?
					// Same as 0x1a??
		Unhandled(intrinsic, num_parms, parms);
		//+++++++++++++++++++++
		break;
	case 0x88:			// Get npc flag(item, flag#).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		return Usecode_value(obj ? 
			obj->get_flag(parms[1].get_int_value())	: 0);
		}
	case 0x89:		// Set npc flag(item, flag#).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			obj->set_flag(parms[1].get_int_value());
		break;
		}
	case 0x8a:		// Clear npc flag(item, flag#).
		{
		Game_object *obj = get_item(parms[0].get_int_value());
		if (obj)
			obj->clear_flag(parms[1].get_int_value());
		break;
		}
	case 0x8d:		// ?? Returns party?
//+++++++++++++++++++++
		Unhandled(intrinsic, num_parms, parms);
		break;
	default:
		Unhandled(intrinsic, num_parms, parms);
		break;
		}
	Usecode_value no_ret;				// Dummy return.
	return (no_ret);
	}

/*
 *	Wait for user to click.  (Mainly for testing).
 */

void Usecode_machine::click_to_continue
	(
	)
	{
	Answers save_answers;		// Save answers list.
	save_answers = answers;
	answers.clear();
	answers.add_answer("Continue");
	get_user_choice_num();
	user_choice = 0;		// Clear it.
	answers = save_answers;
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
	extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape);
	int x, y;			// Get click.
	int choice_num;
	do
		if (!Get_click(x, y, Mouse::hand))
			return (-1);
					// Wait for valid choice.
	while ((choice_num = gwin->conversation_choice(x, y)) < 0 ||
		choice_num >= answers.answers.size());
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
	) : String(0), gwin(gw), call_depth(0), caller_item(0),
	    stack(new Usecode_value[1024]), user_choice(0)
	{
	sp = stack;
					// Clear global flags.
	memset(gflags, 0, sizeof(gflags));
					// Clear party list.
	memset((char *) &party[0], 0, sizeof(party));
	party_count = 0;
//	gflags[0x1b3] = 1;		// Testing Ferryman.
	gflags[0x3d] = 1;	//+++++Password to leave Trinsic.
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
	}

#if DEBUG
int debug = 0;				// 2 for more stuff.
#endif

/*
 *	Interpret a single usecode function.
 */

void Usecode_machine::run
	(
	Usecode_function *fun,
	int event			// Event (??) that caused this call.
	)
	{
	call_depth++;
#if DEBUG
	if (debug >= 0)
		printf("Running usecode %04x with event %d\n", fun->id, event);
#endif
	Usecode_value *save_sp = sp;	// Save TOS.
	Answers save_answers;		// Save answers list.
	save_answers = answers;
	answers.clear();
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
//		locals[i] = val; //+++++I think the above is correct.
		}
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
			printf("SP = %d, IP = %04x, op = %02x\n", sp - stack,
						ip - code, opcode);
#endif
		switch (opcode)
			{
		case 0x04:		// Jump if done with function.
			offset = (short) Read2(ip);
			if (set_ret_value || !answers.answers.size())
				ip += offset;
			break;
		case 0x05:		// JNE.
			{
			offset = (short) Read2(ip);
			Usecode_value val = pop();
			if (val.get_int_value() == 0 &&
			    val.get_array_size() == 0)
				ip += offset;
			break;
			}
		case 0x06:		// JMP.
			offset = (short) Read2(ip);
			ip += offset;
			break;
		case 0x07:		// Guessing CMPS.
			{
			if (!get_user_choice())
				user_choice = "";
			int cnt = Read2(ip);	// # strings.
			offset = (short) Read2(ip);
			while (cnt-- && strcmp(pops(), user_choice) != 0)
				;
			if (cnt == -1)	// Jump if no match.
				ip += offset;
			}
			break;
		case 0x09:		// ADD.+++++Handle strings here.
			pushi(popi() + popi());
			break;
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
			pushi(popi() & popi());
			break;
		case 0x0f:		// OR.
			pushi(popi() | popi());
			break;
		case 0x10:		// NOT.
			pushi(!popi());
			break;
		case 0x12:		// POP into a variable.
			{
			offset = Read2(ip);
					// Get value.
			Usecode_value val = pop();
			if (offset < 0 || offset >= num_locals)
				cerr << "Local #" << offset << 
							"out of range\n";
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
			{		// Get array size.
			offset = Read2(ip);
			Usecode_value arr(offset, 0);
			for (int i = 0; i < offset; i++)
				{
				Usecode_value val = pop();
				arr.put_elem(i, val);
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
			call_usecode_function(externals[2*offset] + 
					256*externals[2*offset + 1]);
			break;
		case 0x25:		// RET.
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
							"out of range\n";
				pushi(0);
				break;
				}
			Usecode_value& val = locals[offset];
			int sz = val.get_array_size();
			if (sz <= 0 || sval < 0 || sval >= sz)
				{
				cerr << 
				"AIDX index out of range, or not an array\n";
				pushi(0);
				break;
				}
			push(val.get_elem(sval));
			break;
			}
		case 0x2c:		// Unknown.
			break;
		case 0x2d:		// Set return value (SETR).
					// But 1st takes precedence.
			if (!set_ret_value)
				ret_value = pop();
			set_ret_value = 1;
			break;
		case 0x2e:		// Looks like a loop.
			if (*ip++ != 2)
				cout << "2nd byte in loop isn't a 2!\n";
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
				sprintf(buf, "%d",
					locals[offset].get_int_value());
				append_string(buf);
				}
			break;
			}
		case 0x30:		// IN.  Is a val. in an array?
			{
			Usecode_value arr = pop();
			Usecode_value val = pop();
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
		case 0x3f:		// Guessing some kind of return.
					// Experimenting...
			if (gwin->is_npc_text_pending())
				click_to_continue();	
			ip = endp;
			sp = save_sp;		// Restore stack.
			break;
		case 0x40:		// Unknown.
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
		case 0x47:		// CALLE.
			offset = Read2(ip);
			call_usecode_function(offset);
			break;
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
			cout << "Opcode " << opcode << " not known.\n";
			break;
			}
		}
	delete [] locals;
					// Restore list of answers.
	answers = save_answers;
#if DEBUG
	if (debug >= 1)
		printf("RETurning from usecode %04x\n", fun->id);
#endif
	call_depth--;
	}

/*
 *	Call a usecode function.
 *
 *	Output:	0 if not found.
 */

int Usecode_machine::call_usecode_function
	(
	int id,
	int event,			// Event (?) that caused this call.
	Usecode_value *parm0		// If non-zero, pass this parm.
	)
	{
					// Look up in table.
	Vector *slot = (Vector *) funs->get(id/0x100);
	Usecode_function *fun = slot ? (Usecode_function *) slot->get(id%0x100)
				     : 0;
	if (!fun)
		{
		cout << "Usecode " << id << " not found.\n";
		return (0);
		}
	if (parm0)
		push(*parm0);
	run(fun, event);		// Do it.
	return (1);
	}

#if 0
/*
 *	Testing...
 */

int main(int argc, char **argv)
	{
	ifstream ufile("static/usecode");
	Usecode_machine interp(ufile);
	Usecode_value parm(0);		// They all seem to take 1 parm.
//	interp.call_usecode(0x9b, &parm);	// Try function #2.
	int id;
	if (argc > 1)
		{
		char *stop;
		id = strtoul(argv[1], &stop, 16);
		}
	else
		id = 0x269;
	interp.call_usecode(id, &parm);
	return (0);
	}
#endif
