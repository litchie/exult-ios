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

class Uc_symbol;

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
	virtual void gen_value(ostream& out) = 0;
					// Gen. to assign from stack.
	virtual void gen_assign(ostream& out);
	};

/*
 *	A variable.
 */
class Uc_var_expression : public Uc_expression
	{
	Uc_symbol *var;
public:
					// Use current location.
	Uc_var_expression(Uc_symbol *v) : var(v)
		{  }
					// Gen. code to put result on stack.
	virtual void gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual void gen_assign(ostream& out);
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
	virtual void gen_value(ostream& out);
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
	virtual void gen_value(ostream& out);
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
	virtual void gen_value(ostream& out);
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
	virtual void gen_value(ostream& out);
	};

/*
 *	A concatenation, which generates an array:
 */
class Uc_array_expression : public Uc_expression
	{
	vector<Uc_expression*> exprs;
public:
	Uc_array_expression() {  }
	~Uc_array_expression();
	void add(Uc_expression *e)	// Append an expression.
		{ exprs.push_back(e); }
					// Gen. code to put result on stack.
	virtual void gen_value(ostream& out);
	};

#endif
