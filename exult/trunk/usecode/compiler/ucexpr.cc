/**
 **	Ucexpr.cc - Expressions for Usecode compiler.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <stdio.h>
#include "ucexpr.h"
#include "ucsym.h"
#include "utils.h"
#include "opcodes.h"
#include "ucfun.h"

/*
 *	Default.  Just push the one value.
 *
 *	Output:	# pushed
 */

int Uc_expression::gen_values
	(
	vector<char>& out
	)
	{
	gen_value(out);			// Gen. result on stack.
	return 1;
	}

/*
 *	Default jmp-if-false generation.
 *
 *	Output:	offset where offset is stored.
 */

int Uc_expression::gen_jmp_if_false
	(
	vector<char>& out,
	int offset			// Offset to jmp (relative).
	)
	{
	gen_value(out);			// Gen. result on stack.
	out.push_back((char) UC_JNE);		// Pop and jmp if false.
	Write2(out, offset);
	return out.size() - 2;
	}

/*
 *	Default assignment generation.
 */

void Uc_expression::gen_assign
	(
	vector<char>& out
	)
	{
	error("Can't assign to this expression");
	}

/*
 *	Need a variable whose value is this expression.
 */

Uc_var_symbol *Uc_expression::need_var
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
	static int cnt = 0;
	char buf[50];
	sprintf(buf, "_tmpval_%d", cnt++);
					// Create a 'tmp' variable.
	Uc_var_symbol *var = fun->add_symbol(buf);
	if (!var)
		return 0;		// Shouldn't happen.  Err. reported.
	gen_value(out);			// Want to assign this value to it.
	var->gen_assign(out);
	return var;
	}

/*
 *	Evaluate constant.
 *
 *	Output:	true if successful, with result returned in 'val'.
 */

bool Uc_expression::eval_const
	(
	int& val			// Value returned here.
	)
	{
	val = 0;
	error("Integer constant expected.");
	return false;
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_var_expression::gen_value
	(
	vector<char>& out
	)
	{
	char buf[150];
	if (!var->gen_value(out))
		{
		sprintf(buf, "Can't use value of '%s'", var->get_name());
		error(buf);
		}
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_var_expression::gen_assign
	(
	vector<char>& out
	)
	{
	char buf[150];
	if (!var->gen_assign(out))
		{
		sprintf(buf, "Can't assign to '%s'", var->get_name());
		error(buf);
		}
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_arrayelem_expression::gen_value
	(
	vector<char>& out
	)
	{
	if (!index || !array)
		return;
	index->gen_value(out);		// Want index on stack.
	out.push_back((char) UC_AIDX);	// Opcode, var #.
	Write2(out, array->get_offset());
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_arrayelem_expression::gen_assign
	(
	vector<char>& out
	)
	{
	if (!index || !array)
		return;
	index->gen_value(out);		// Want index on stack.
	out.push_back((char) UC_POPARR);	// Opcode, var #.
	Write2(out, array->get_offset());
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_flag_expression::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHF);	// Opcode, flag #.
	Write2(out, index);
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_flag_expression::gen_assign
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_POPF);
	Write2(out, index);
	}

/*
 *	Get offset in function's text_data.
 *
 *	Output:	Offset.
 */

int Uc_var_expression::get_string_offset
	(
	)
	{ 
	return var->get_string_offset(); 
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_binary_expression::gen_value
	(
	vector<char>& out
	)
	{
	left->gen_value(out);		// First the left.
	right->gen_value(out);		// Then the right.
	out.push_back((char) opcode);
	}

/*
 *	Evaluate constant.
 *
 *	Output:	true if successful, with result returned in 'val'.
 */

bool Uc_binary_expression::eval_const
	(
	int& val			// Value returned here.
	)
	{
	int val1, val2;			// Get each side.
	if (!left->eval_const(val1) || !right->eval_const(val2))
		return false;
	switch (opcode)
		{
	case UC_ADD:	val = val1 + val2; return true;
	case UC_SUB:	val = val1 - val2; return true;
	case UC_MUL:	val = val1*val2; return true;
	case UC_DIV:
		if (!val2)
			{
			error("Division by 0");
			return false;
			}
		val = val1/val2;
		return true;
		}
	val = 0;
	error("This operation not supported for integer constants");
	return false;
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_unary_expression::gen_value
	(
	vector<char>& out
	)
	{
	operand->gen_value(out);
	out.push_back((char) opcode);
	}

/*
 *	Can't use this expression just anywhere.
 */

void Uc_response_expression::gen_value
	(
	vector<char>& out
	)
	{
	error("Must use UcResponse in 'if (UcResponse == ...)'");
	}

/*
 *	Jmp-if-false generation for getting conversation response & comparing
 *	to a string or strings.
 *
 *	Output:	offset where offset is stored.
 */

int Uc_response_expression::gen_jmp_if_false
	(
	vector<char>& out,
	int offset			// Offset to jmp (relative).
	)
	{
					// Push string(s) on stack.
	int cnt = operand->gen_values(out);
	out.push_back((char) UC_CMPS);
	Write2(out, cnt);		// # strings on stack.
	Write2(out, offset);		// Offset to jmp if false.
	return out.size() - 2;
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_int_expression::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHI);
	Write2(out, value);
	}

/*
 *	Evaluate constant.
 *
 *	Output:	true if successful, with result returned in 'val'.
 */

bool Uc_int_expression::eval_const
	(
	int& val			// Value returned here.
	)
	{
	val = value;
	return true;
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_bool_expression::gen_value
	(
	vector<char>& out
	)
	{
	if (tf)
		out.push_back((char) UC_PUSHTRUE);
	else
		out.push_back((char) UC_PUSHFALSE);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_event_expression::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHEVENTID);
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_event_expression::gen_assign
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_POPEVENTID);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_item_expression::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHITEMREF);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_string_expression::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHS);
	Write2(out, offset);
	}

/*
 *	Delete a list of expressions.
 */

Uc_array_expression::~Uc_array_expression
	(
	)
	{
	for (std::vector<Uc_expression *>::iterator it = exprs.begin(); 
						it != exprs.end(); it++)
		delete (*it);
	}

/*
 *	Concatenate another expression, or its values if an array, onto this.
 *	If the expression is an array, it's deleted after its elements are
 *	taken.
 */

void Uc_array_expression::concat
	(
	Uc_expression *e
	)
	{
	Uc_array_expression *arr = dynamic_cast<Uc_array_expression *> (e);
	if (!arr)
		add(e);			// Singleton?  Just add it.
	else
		{
		for (std::vector<Uc_expression *>::iterator it = 
			arr->exprs.begin(); it != arr->exprs.end(); it++)
			add(*it);
		arr->exprs.clear();	// Don't want to delete elements.
		delete arr;		// But this array is history.
		}
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_array_expression::gen_value
	(
	vector<char>& out
	)
	{
	int actual = Uc_array_expression::gen_values(out);
	out.push_back((char) UC_ARRC);
	Write2(out, actual);
	}

/*
 *	Push all values onto the stack.
 *
 *	Output:	# pushed
 */

int Uc_array_expression::gen_values
	(
	vector<char>& out
	)
	{
	int actual = 0;			// (Just to be safe.)
					// Push backwards, so #0 pops first.
	for (std::vector<Uc_expression *>::reverse_iterator it = 
				exprs.rbegin(); it != exprs.rend(); it++)
		{
		Uc_expression *expr = *it;
		if (expr)
			{
			actual++;
			expr->gen_value(out);
			}
		}
	return actual;
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_call_expression::gen_value
	(
	vector<char>& out
	)
	{
	if (!sym)
		return;			// Already failed once.
	if (!sym->gen_call(out, function, itemref,  parms, return_value))
		{
		char buf[150];
		sprintf(buf, "'%s' isn't a function or intrinsic",
						sym->get_name());
		sym = 0;		// Avoid repeating error if in loop.
		error(buf);
		}
	}
