/**
 **	Usecode.h - Interpreter for usecode.
 **
 **	Written: 8/12/99 - JSF
 **/

#ifndef INCL_USECODE
#define INCL_USECODE	1

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

class istream;
class ostream;
class Game_window;
class Game_object;

/*
 *	A value that we store can be an integer, string, or array.
 */
class Usecode_value
	{
protected:
	enum Val_type			// The types:
		{
		int_type = 0,
		string_type = 1,
		array_type = 2,
		end_of_array_type = 3	// Marks end of array.
		};
private:
	unsigned char type;		// Type stored here.
	union
		{
		long intval;
		char *str;
		Usecode_value *array;
		} value;
					// Count array elements.
	static int count_array(const Usecode_value& val);
public:
	Usecode_value() : type((unsigned char) int_type)
		{ value.intval = 0; }
	Usecode_value(int ival) : type((unsigned char) int_type)
		{ value.intval = ival; }
	Usecode_value(char *s) : type((unsigned char) string_type)
		{ value.str = s; }
					// Create array with 1st element.
	Usecode_value(int size, Usecode_value *elem0) 
			: type((unsigned char) array_type)
		{
		value.array = new Usecode_value[size + 1];
		value.array[size].type = (unsigned char) end_of_array_type;
		if (elem0)
			value.array[0] = *elem0;
		}
	~Usecode_value()
		{
		if (type == (unsigned char) array_type)
			delete [] value.array;
		}
	void operator=(const Usecode_value& v2)
		{
		if (&v2 == this)
			return;
		if (type == (int) array_type)
			delete [] value.array;
		type = v2.type;		// Assign new values.
		if (type == (int) int_type)
			value.intval = v2.value.intval;
		else if (type == (int) string_type)
			value.str = v2.value.str;
		else if (type == (int) array_type)
			{
			value.array = new Usecode_value[1+count_array(v2)];
			int i = 0;
			do
				value.array[i] = v2.value.array[i];
			while (value.array[i++].type != 
						(int) end_of_array_type);
			}
		}
					// Copy ctor.
	Usecode_value(const Usecode_value& v2) : type((unsigned char) int_type)
		{ *this = v2; }
					// Comparator.
	int operator==(const Usecode_value& v2);
	int get_array_size()		// Get size of array.
		{ return type == (int) array_type ? count_array(*this) : 0; }
	unsigned int get_int_value()	// Get integer value.
		{ return (type == (int) int_type ? value.intval : 0); }
					// Get string value.
	char *get_str_value()
		{ return (type == (int) string_type ? value.str : 0); }
					// Add array element. (No checking!)
	void put_elem(int i, Usecode_value& val)
		{ value.array[i] = val; }
					// Get an array element.
	Usecode_value& get_elem(int i)
		{ return value.array[i]; }
	int resize(int new_size);	// Resize array.
					// Look in array for given value.
	int find_elem(const Usecode_value& val);
					// Concat. to end of this array.
	Usecode_value& concat(Usecode_value& val2);
	void print(ostream& out);	// Print in ASCII.
	};

/*
 *	A single function:
 */
class Usecode_function
	{
	int id;				// The function #.  (Appears to be the
					//   game item # the function gets
					//   called for.)
	int len;			// Length.
	unsigned char *code;		// The code.
	friend class Usecode_machine;
					// Create from file.
	Usecode_function(istream& file);
	~Usecode_function()
		{ delete code; }
	};

const int max_funs = 1500;
const int max_answers = 40;
const int answer_stack_size = 10;

/*
 *	A set of answers:
 */
class Answers
	{
	friend class Usecode_machine;
	char *answers[max_answers];	// What we can click on.
	int num_answers;
	Answers() : num_answers(0)
		{  }
	void add_answer(char *str);	// Add to the list.
	void add_answer(Usecode_value& val);
	void remove_answer(Usecode_value& val);
	void operator=(Answers& cpy);	// Shallow copy.
	};	

/*
 *	Here's our virtual machine for running usecode.
 */
class Usecode_machine
	{
	Game_window *gwin;		// Game window.
	Usecode_function *funs[max_funs];// List of functions.+++++Hash table.
	int num_funs;			// # in list.
	unsigned char gflags[1024];	// Global flags.
	int party[8];			// NPC #'s of party members.
	int party_count;		// # of NPC's in party.
	Game_object *caller_item;	// Item this is being called on.
	char *user_choice;		// String user clicked on.
	char *string;			// The single string register.
	void append_string(char *txt);	// Append to string.
	void say_string();		// "Say" the string.
	Usecode_value *stack;		// Stack.
	Usecode_value *sp;		// Stack ptr.  Grows upwards.
	void stack_error(int under);
	void push(Usecode_value& val)	// Push/pop stack.
		{ *sp++ = val; }
	Usecode_value pop()		// +++++Watch for too many str. copies.
		{ 
		if (sp <= stack)
			stack_error(1);
		return *--sp; 
		}
	void pushi(long val)		// Push/pop integers.
		{
		Usecode_value v(val);
		push(v);
		}
	int popi()
		{
		Usecode_value val = pop();
		return (val.get_int_value());
		}
					// Push/pop strings.
	void pushs(char *s)
		{
		Usecode_value val(s);
		push(val);
		}
	char *pops()
		{
		Usecode_value val = pop();
		return (val.get_str_value());
		}
	Answers answers;		// What user can click on.
	int saved_answers;		// # of 'saved' answer sets.
	Answers *answer_stack[answer_stack_size];
	Game_object *get_item(long val);// Get ->obj. from 'itemref'.
	/*
	 *	Built-in usecode functions:
	 */
	void show_npc_face(Usecode_value& arg1, Usecode_value& arg2);
	void remove_npc_face(Usecode_value& arg1);
	void set_item_shape(Usecode_value& item_arg, Usecode_value& shape_arg);
	void set_item_frame(Usecode_value& item_arg, Usecode_value& frame_arg);
	int get_item_shape(Usecode_value& item_arg);
	int get_item_frame(Usecode_value& item_arg);
	int npc_in_party(int npc);
	void add_to_party(int npc);
	void remove_from_party(int npc);
	Usecode_value get_party();
	void item_say(Usecode_value& objval, Usecode_value& strval);
	void exec_array(Usecode_value& objval, Usecode_value& arrayval);

	/*
	 *	Other private methods:
	 */
					// Call instrinsic function.
	Usecode_value call_intrinsic(int intrinsic, int num_parms);
	void click_to_continue();	// Wait for user to click.
	char *get_user_choice();	// Get user's choice.
	int get_user_choice_num();
					// Run the function.
	void run(Usecode_function *fun, int event);
					// Call desired function.
	int call_usecode_function(int id, int event = 0, 
						Usecode_value *parm0 = 0);
public:
	Usecode_machine(istream& file, Game_window *gw);
	~Usecode_machine();
					// Possible events:
	enum Usecode_events {
		npc_proximity = 0,
		double_click = 1,
		game_start = 2,		// Definitely guessing.
		egg_proximity = 3
		};
					// Call desired function.
	int call_usecode(int id, Game_object *obj, Usecode_events event)
		{
		caller_item = obj;
		Usecode_value parm(0);	// They all seem to take 1 parm.
		int ret = call_usecode_function(id, event, &parm);
		caller_item = 0;
		return ret;
		}
	};

#endif	/* INCL_USECODE */
