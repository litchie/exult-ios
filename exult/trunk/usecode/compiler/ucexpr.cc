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

#include <iostream.h>
#include <stdio.h>
#include "ucexpr.h"
#include "ucsym.h"
#include "utils.h"
#include "opcodes.h"

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
	for (vector<Uc_expression *>::iterator it = exprs.begin(); 
						it != exprs.end(); it++)
		delete (*it);
	}

/*
 *	Generate code to evaluate expression and leave result on stack.
 */

void Uc_array_expression::gen_value
	(
	ostream& out
	)
	{
	int actual = 0;			// (Just to be safe.)
					// Push backwards, so #0 pops first.
	for (vector<Uc_expression *>::reverse_iterator it = exprs.rbegin();
						it != exprs.rend(); it++)
		{
		Uc_expression *expr = *it;
		if (expr)
			{
			actual++;
			expr->gen_value(out);
			}
		}
	out.put((char) UC_ARRC);
	Write2(out, actual);
	}

