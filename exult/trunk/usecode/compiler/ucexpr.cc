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


#include <iostream.h>
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
	ostream& out
	)
	{
	gen_value(out);			// Gen. result on stack.
	return 1;
	}

/*
 *	Default jmp-if-false generation.
 */

void Uc_expression::gen_jmp_if_false
	(
	ostream& out,
	int offset			// Offset to jmp (relative).
	)
	{
	gen_value(out);			// Gen. result on stack.
	out.put((char) UC_JNE);		// Pop and jmp if false.
	Write2(out, offset);
	}

/*
 *	Default assignment generation.
 */

void Uc_expression::gen_assign
	(
	ostream& out
	)
	{
	error("Can't assign to this expression");
	}

/*
 *	Need a variable whose value is this expression.
 */

Uc_var_symbol *Uc_expression::need_var
	(
	ostream& out,
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
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_var_expression::gen_value
	(
	ostream& out
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
	ostream& out
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
	ostream& out
	)
	{
	if (!index || !array)
		return;
	index->gen_value(out);		// Want index on stack.
	out.put((char) UC_AIDX);	// Opcode, var #.
	Write2(out, array->get_offset());
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_arrayelem_expression::gen_assign
	(
	ostream& out
	)
	{
	if (!index || !array)
		return;
	index->gen_value(out);		// Want index on stack.
	out.put((char) UC_POPARR);	// Opcode, var #.
	Write2(out, array->get_offset());
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_flag_expression::gen_value
	(
	ostream& out
	)
	{
	out.put((char) UC_PUSHF);	// Opcode, flag #.
	Write2(out, index);
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_flag_expression::gen_assign
	(
	ostream& out
	)
	{
	out.put((char) UC_POPF);
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
	ostream& out
	)
	{
	left->gen_value(out);		// First the left.
	right->gen_value(out);		// Then the right.
	out.put((char) opcode);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_unary_expression::gen_value
	(
	ostream& out
	)
	{
	operand->gen_value(out);
	out.put((char) opcode);
	}

/*
 *	Can't use this expression just anywhere.
 */

void Uc_response_expression::gen_value
	(
	ostream& out
	)
	{
	error("Must use UcResponse in 'if (UcResponse == ...)'");
	}

/*
 *	Jmp-if-false generation for getting conversation response & comparing
 *	to a string or strings.
 */

void Uc_response_expression::gen_jmp_if_false
	(
	ostream& out,
	int offset			// Offset to jmp (relative).
	)
	{
					// Push string(s) on stack.
	int cnt = operand->gen_values(out);
	out.put((char) UC_CMPS);
	Write2(out, cnt);		// # strings on stack.
	Write2(out, offset);		// Offset to jmp if false.
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_int_expression::gen_value
	(
	ostream& out
	)
	{
	out.put((char) UC_PUSHI);
	Write2(out, value);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_bool_expression::gen_value
	(
	ostream& out
	)
	{
	if (tf)
		out.put((char) UC_PUSHTRUE);
	else
		out.put((char) UC_PUSHFALSE);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_event_expression::gen_value
	(
	ostream& out
	)
	{
	out.put((char) UC_PUSHEVENTID);
	}

/*
 *	Generate assignment to this variable.
 */

void Uc_event_expression::gen_assign
	(
	ostream& out
	)
	{
	out.put((char) UC_POPEVENTID);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_item_expression::gen_value
	(
	ostream& out
	)
	{
	out.put((char) UC_PUSHITEMREF);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_string_expression::gen_value
	(
	ostream& out
	)
	{
	out.put((char) UC_PUSHS);
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
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_array_expression::gen_value
	(
	std::ostream& out
	)
	{
	int actual = Uc_array_expression::gen_values(out);
	out.put((char) UC_ARRC);
	Write2(out, actual);
	}

/*
 *	Push all values onto the stack.
 *
 *	Output:	# pushed
 */

int Uc_array_expression::gen_values
	(
	ostream& out
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
	std::ostream& out
	)
	{
	if (!sym)
		return;			// Already failed once.
	if (!sym->gen_call(out, function, parms, return_value))
		{
		char buf[150];
		sprintf(buf, "'%s' isn't a function or intrinsic",
						sym->get_name());
		sym = 0;		// Avoid repeating error if in loop.
		error(buf);
		}
	}
