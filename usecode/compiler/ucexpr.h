/**
 **	Ucexpr.h - Expressions for Usecode compiler.
 **
 **	Written: 1/0/01 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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

#ifndef INCL_UCEXPR
#define INCL_UCEXPR	1

#include <vector>
#include "ucloc.h"

using std::vector;

class Uc_symbol;
class Uc_var_symbol;
class Uc_function;

/*
 *	Base class for expressions.
 */
class Uc_expression : public Uc_location
	{
public:
					// Use current location.
	Uc_expression() : Uc_location()
		{  }
	virtual ~Uc_expression() {  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out) = 0;
					// Gen code to push value(s).
	virtual int gen_values(vector<char>& out);
					// Gen. code to jmp if this is false.
	virtual int gen_jmp_if_false(vector<char>& out, int offset);
					// Gen. to assign from stack.
	virtual void gen_assign(vector<char>& out);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return -1; }
					// Get/create var == this.
	virtual Uc_var_symbol *need_var(vector<char>& out, Uc_function *fun);
					// Evaluate constant.
	virtual bool eval_const(int& val);
	};

/*
 *	A variable.
 */
class Uc_var_expression : public Uc_expression
	{
	Uc_var_symbol *var;
public:
					// Use current location.
	Uc_var_expression(Uc_var_symbol *v) : var(v)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual void gen_assign(vector<char>& out);
	virtual int get_string_offset();// Get offset in text_data.
	virtual Uc_var_symbol *need_var(vector<char>& , Uc_function *)
		{ return var; }
	};

/*
 *	An array element.
 */
class Uc_arrayelem_expression : public Uc_expression
	{
	Uc_var_symbol *array;
	Uc_expression *index;
public:
	Uc_arrayelem_expression(Uc_var_symbol *a, Uc_expression *i)
		: array(a), index(i)
		{  }
	~Uc_arrayelem_expression()
		{ delete index; }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual void gen_assign(vector<char>& out);
	};

/*
 *	Global flag.
 */
class Uc_flag_expression : public Uc_expression
	{
	int index;
public:
	Uc_flag_expression(int i)
		: index(i)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual void gen_assign(vector<char>& out);
	};

/*
 *	Binary expressions.
 */
class Uc_binary_expression : public Uc_expression
	{
	int opcode;			// Should be the UC_<opcode>
	Uc_expression *left, *right;	// Operands to add, sub, etc.
public:
	Uc_binary_expression(int o, Uc_expression *l, Uc_expression *r)
		: opcode(o), left(l), right(r)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Evaluate constant.
	virtual bool eval_const(int& val);
	};

/*
 *	Unary expressions.
 */
class Uc_unary_expression : public Uc_expression
	{
	int opcode;			// Should be the UC_<opcode>
	Uc_expression *operand;
public:
	Uc_unary_expression(int o, Uc_expression *r)
		: opcode(o), operand(r)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
	};

/*
 *	Compare user conversation response to a given string (or list of
 *	strings.
 */
class Uc_response_expression : public Uc_expression
	{
	Uc_expression *operand;
public:
	Uc_response_expression(Uc_expression *r)
		: operand(r)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen. code to jmp if this is false.
	virtual int gen_jmp_if_false(vector<char>& out, int offset);
	};

/*
 *	Integer value.
 */
class Uc_int_expression : public Uc_expression
	{
	int value;
public:
	Uc_int_expression(int v) : value(v)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Evaluate constant.
	virtual bool eval_const(int& val);
	};

/*
 *	Boolean value.
 */
class Uc_bool_expression : public Uc_expression
	{
	bool tf;
public:
	Uc_bool_expression(bool t) : tf(t)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
	};

/*
 *	Eventid (a special int variable passed to each function):
 */
class Uc_event_expression : public Uc_expression
	{
public:
	Uc_event_expression() {  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual void gen_assign(vector<char>& out);
	};

/*
 *	Item (a special ptr. variable passed to each function):
 */
class Uc_item_expression : public Uc_expression
	{
public:
	Uc_item_expression() {  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
	};

/*
 *	String value.
 */
class Uc_string_expression : public Uc_expression
	{
	int offset;			// Offset in function's data area.
public:
	Uc_string_expression(int o) : offset(o)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return offset; }
	};

/*
 *	A concatenation, which generates an array:
 */
class Uc_array_expression : public Uc_expression
	{
		std::vector<Uc_expression*> exprs;
public:
	Uc_array_expression() {  }
	~Uc_array_expression();
	void add(Uc_expression *e)	// Append an expression.
		{ exprs.push_back(e); }
	void concat(Uc_expression *e);	// Concat e's elements onto this.
	const std::vector<Uc_expression*>& get_exprs()
		{ return exprs; }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
					// Gen code to push value(s).
	virtual int gen_values(vector<char>& out);
	};

/*
 *	A function or intrinsic call.
 */
class Uc_call_expression : public Uc_expression
	{
	Uc_symbol *sym;			// Function or intrinsic.
	Uc_expression *itemref;		// Non-null for CALLE.
	Uc_array_expression *parms;
	Uc_function *function;		// May need function this is in.
	bool return_value;		// True for a function (to return
					//   its value).
public:
	Uc_call_expression(Uc_symbol *s, Uc_array_expression *prms,
			Uc_function *fun, Uc_expression *item = 0)
		: sym(s), itemref(item), parms(prms), 
		  function(fun), return_value(true)
		{  }
	~Uc_call_expression()
		{ delete parms; delete itemref; }
	void set_no_return()
		{ return_value = false; }
					// Gen. code to put result on stack.
	virtual void gen_value(vector<char>& out);
	};

/*
 *	Write a 2-byte value to the end/position of a character stream.
 */

inline void Write2(vector<char>& out, unsigned short val)
	{
	out.push_back((char) (val&0xff));
	out.push_back((char) ((val>>8)&0xff));
	}
inline void Write2(vector<char>& out, int pos, unsigned short val)
	{
	out[pos] = (char) (val&0xff);
	out[pos + 1] = (char) ((val>>8)&0xff);
	}

#endif
