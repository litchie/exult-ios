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
class Vector;
#include <vector>	// STL container
#include <deque>	// STL container
#include <string>	// STL string

#define	USECODE_FUNCTION(NAME)	Usecode_value	Usecode_machine::UI_ ## NAME ## (int event,int intrinsic,Usecode_value parms[12])
#define	USECODE_FUNCTION_DECL(NAME)	Usecode_value	UI_ ## NAME ## (int event,int intrinsic,Usecode_value parms[12])
#define	USECODE_FUNCTION_PTR(NAME)	&Usecode_machine::UI_ ## NAME

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
		const char *str;
		Usecode_value *array;
		} value;
					// Count array elements.
	static int count_array(const Usecode_value& val);
public:
	Usecode_value() : type((unsigned char) int_type)
		{ value.intval = 0; }
	Usecode_value(int ival) : type((unsigned char) int_type)
		{ value.intval = ival; }
	Usecode_value(const char *s) : type((unsigned char) string_type)
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
	Usecode_value &operator=(const Usecode_value& v2)
		{
		if (&v2 == this)
			return *this;
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
		return *this;
		}
					// Copy ctor.
	Usecode_value(const Usecode_value& v2) : type((unsigned char) int_type)
		{ *this = v2; }
					// Comparator.
	int operator==(const Usecode_value& v2);
	int get_array_size()		// Get size of array.
		{ return type == (int) array_type ? count_array(*this) : 0; }
	int is_array()
		{ return type == (int) array_type; }
	int is_int()
		{ return type == (int) int_type; }
	unsigned int get_int_value()	// Get integer value.
		{ return (type == (int) int_type ? value.intval : 0); }
					// Get string value.
	const char *get_str_value()
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


/*
 *	A set of answers:
 */
struct Answers
	{
	friend class Usecode_machine;
	vector<string> answers;	// What we can click on.
	Answers();
	void add_answer(const char *str);	// Add to the list.
	void add_answer(Usecode_value& val);
	void remove_answer(Usecode_value& val);
	void _remove_answer(const char *);
	void	clear(void);
	};	

/*
 *	Here's our virtual machine for running usecode.
 */
struct Usecode_machine
	{
	Game_window *gwin;		// Game window.
	Vector *funs;			// I'th entry contains funs for ID's
					//    256*i + n.
	int call_depth;			// How far deep we are.
	unsigned char gflags[1024];	// Global flags.
	int party[8];			// NPC #'s of party members.
	int party_count;		// # of NPC's in party.
	Game_object *caller_item;	// Item this is being called on.
	const char *user_choice;		// String user clicked on.
	char *String;			// The single string register.
	void append_string(const char *txt);	// Append to string.
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
	const char *pops()
		{
		Usecode_value val = pop();
		return (val.get_str_value());
		}
	Answers answers;		// What user can click on.
	deque< Answers > answer_stack;
	Game_object *get_item(long val);// Get ->obj. from 'itemref'.
	/*
	 *	Built-in usecode functions:
	 */
public:
	void show_npc_face(Usecode_value& arg1, Usecode_value& arg2);
	void remove_npc_face(Usecode_value& arg1);
	void set_item_shape(Usecode_value& item_arg, Usecode_value& shape_arg);
	void set_item_frame(Usecode_value& item_arg, Usecode_value& frame_arg);
	int get_item_shape(Usecode_value& item_arg);
	int get_item_frame(Usecode_value& item_arg);
	int npc_in_party(Game_object *npc);
	void add_to_party(Game_object *npc);
	void remove_from_party(Game_object *npc);
	Usecode_value get_party();
	void item_say(Usecode_value& objval, Usecode_value& strval);
	Usecode_value find_nearby(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qval,
							Usecode_value& mval);
	Usecode_value find_direction(Usecode_value& from, Usecode_value& to);
	Usecode_value count_objects(Usecode_value& objval,
						Usecode_value& shapeval);
	Usecode_value get_objects(Usecode_value& objval,
						Usecode_value& shapeval);
	Usecode_value click_on_item();
	void exec_array(Usecode_value& objval, Usecode_value& arrayval);

	/*
         *	Embedded intrinsics
	 */

	USECODE_FUNCTION_DECL(NOP);
	USECODE_FUNCTION_DECL(UNKNOWN);
	USECODE_FUNCTION_DECL(get_random);
	USECODE_FUNCTION_DECL(execute_usecode_array);
	USECODE_FUNCTION_DECL(delayed_execute_usecode_array);
	USECODE_FUNCTION_DECL(show_npc_face);
	USECODE_FUNCTION_DECL(remove_npc_face);
	USECODE_FUNCTION_DECL(add_answer);
	USECODE_FUNCTION_DECL(remove_answer);
	USECODE_FUNCTION_DECL(push_answers);
	USECODE_FUNCTION_DECL(pop_answers);
	USECODE_FUNCTION_DECL(select_from_menu);
	USECODE_FUNCTION_DECL(select_from_menu2);
	USECODE_FUNCTION_DECL(input_numeric_value);
	USECODE_FUNCTION_DECL(set_item_shape);
	USECODE_FUNCTION_DECL(die_roll);
#if 0
	Usecode_value	UI_NOP(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_UNKNOWN(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_get_random(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_execute_usecode_array(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_delayed_execute_usecode_array(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_show_npc_face(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_remove_npc_face(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_add_answer(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_remove_answer(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_push_answers(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_pop_answers(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_select_from_menu(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_select_from_menu2(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_input_numeric_value(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_set_item_shape(int event,int intrinsic,Usecode_value parms[12]);
	Usecode_value	UI_die_roll(int event,int intrinsic,Usecode_value parms[12]);
#endif



	/*
	 *	Other private methods:
	 */
					// Call instrinsic function.
	Usecode_value call_intrinsic(int event, int intrinsic, int num_parms);
	void click_to_continue();	// Wait for user to click.
	const char *get_user_choice();	// Get user's choice.
	int get_user_choice_num();
					// Run the function.
	void run(Usecode_function *fun, int event);
					// Call desired function.
	int call_usecode_function(int id, int event = 0, 
						Usecode_value *parm0 = 0);
public:
	friend class Scheduled_usecode;
	Usecode_machine(istream& file, Game_window *gw);
	~Usecode_machine();
					// Possible events:
	enum Usecode_events {
		npc_proximity = 0,
		double_click = 1,
		internal_exec = 2,	// Internal call via intr. 1 or 2.
		egg_proximity = 3
		};
					// Call desired function.
	int call_usecode(int id, Game_object *obj, Usecode_events event)
		{
					// Avoid these when already execing.
		if (call_depth && event == npc_proximity)
			return (0);
		Game_object *prev_item = caller_item;
		caller_item = obj;
		Usecode_value parm(0);	// They all seem to take 1 parm.
		int ret = call_usecode_function(id, event, &parm);
		caller_item = prev_item;
		return ret;
		}
	};

#endif	/* INCL_USECODE */
