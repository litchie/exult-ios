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
#include "ucexpr.h"
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

