/**
 **	Ucstmt.cc - Usecode compiler statements.
 **
 **	Written: 1/2/01 - JSF
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

#include <stdio.h>
#include <strstream.h>
#include "ucstmt.h"
#include "ucexpr.h"
#include "ucsym.h"
#include "opcodes.h"
#include "utils.h"
#include "ucfun.h"

/*
 *	Delete.
 */

Uc_block_statement::~Uc_block_statement
	(
	)
	{
					// Delete all the statements.
	for (vector<Uc_statement *>::const_iterator it = statements.begin();
					it != statements.end(); it++)
		delete (*it);
	}

/*
 *	Generate code.
 */

void Uc_block_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	for (vector<Uc_statement *>::const_iterator it = statements.begin();
					it != statements.end(); it++)
		{
		Uc_statement *stmt = *it;
		stmt->gen(out, fun);
		}
	}


/*
 *	Delete.
 */

Uc_assignment_statement::~Uc_assignment_statement
	(
	)
	{
	delete target;
	delete value;
	}


/*
 *	Generate code.
 */

void Uc_assignment_statement::gen
	(
	ostream& out,
	Uc_function * /* fun */
	)
	{
	value->gen_value(out);		// Get value on stack.
	target->gen_assign(out);
	}

/*
 *	Generate code.
 */

void Uc_if_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	expr->gen_value(out);		// Eval. test expression.
	int elselen = 0;		// Get ELSE code.
	char *elsestr = 0;
	if (else_stmt)
		{
		ostrstream code;
		else_stmt->gen(code, fun);
		elselen = code.pcount();
		elsestr = code.str();
		}
	ostrstream ifcode;		// Generate IF code.
	if (if_stmt)
		if_stmt->gen(ifcode, fun);
	if (elselen)			// JMP past ELSE code.
		{
		ifcode.put((char) UC_JMP);
		Write2(ifcode, elselen);
		}
	int iflen = ifcode.pcount();
	char *ifstr = ifcode.str();
	if (iflen)			// JNE past IF code.
		{
		out.put((char) UC_JNE);
		Write2(out, iflen);
		}
	out.write(ifstr, iflen);	// Now the IF code.
	out.write(elsestr, elselen);	// Then the ELSE code.
	delete elsestr;
	delete ifstr;
	}

/*
 *	Delete.
 */

Uc_if_statement::~Uc_if_statement
	(
	)
	{
	delete expr;
	delete if_stmt;
	delete else_stmt;
	}

/*
 *	Delete.
 */

Uc_while_statement::~Uc_while_statement
	(
	)
	{
	delete expr;
	delete stmt;
	}

/*
 *	Generate code.
 */

void Uc_while_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	long top = out.tellp();		// Get current position.
	expr->gen_value(out);		// Generate expr. value.
	ostrstream stmt_code;
	if (stmt)
		stmt->gen(stmt_code, fun);	// Generate statement's code.
	int stmtlen = stmt_code.pcount();
					// Get distance back to top, including
					//   a JNE and a JMP.
	long dist = stmtlen + (out.tellp() - top) + 3 + 3;
	stmt_code.put((char) UC_JMP);	// Generate JMP back to top.
	Write2(stmt_code, -dist);
	stmtlen = stmt_code.pcount();	// Get total length.
					// Get -> to code.
	char *stmtstr = stmt_code.str();
	out.put((char) UC_JNE);		// Put cond. jmp. after test.
	Write2(out, stmtlen);		// Skip around body if false.
	out.write(stmtstr, stmtlen);	// Write out body.
	delete stmtstr;			// We own this.
	}

/*
 *	Delete.
 */

Uc_arrayloop_statement::~Uc_arrayloop_statement
	(
	)
	{
	delete stmt;
	}

/*
 *	Finish up creation.
 */

void Uc_arrayloop_statement::finish
	(
	Uc_function *fun
	)
	{
	char buf[100];
	if (!index)			// Create vars. if not given.
		{
		sprintf(buf, "_%s_index", array->get_name());
		index = fun->add_symbol(buf);
		}
	if (!array_size);
		{
		sprintf(buf, "_%s_size", array->get_name());
		array_size = fun->add_symbol(buf);
		}
	}

/*
 *	Generate code.
 */

void Uc_arrayloop_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	if (!stmt)
		return;			// Nothing useful to do.
	out.put((char) UC_LOOP);	// Start of loop.
	int top = out.tellp();		// This is where to jump back to.
	out.put((char) UC_LOOPTOP);
	Write2(out, index->get_offset());// Counter, total-count variables.
	Write2(out, array_size->get_offset());
	Write2(out, var->get_offset());	// Loop variable, than array.
	Write2(out, array->get_offset());
					// Still need to write offset to end.
	int testlen = out.tellp() + 2 - top;
	ostrstream stmt_code;
	stmt->gen(stmt_code, fun);	// Generate statement's code.
					// Back to top includes JMP at end.
	int dist = testlen + stmt_code.pcount() + 3;
	stmt_code.put((char) UC_JMP);	// Generate JMP back to top.
	Write2(stmt_code, -dist);
	int stmtlen = stmt_code.pcount();// Get total length.
	Write2(out, stmtlen);		// Finally, offset past loop stmt.
					// Get -> to code.
	char *stmtstr = stmt_code.str();
	out.write(stmtstr, stmtlen);	// Write out body.
	delete stmtstr;			// We own this.
	}

/*
 *	Delete.
 */

Uc_return_statement::~Uc_return_statement
	(
	)
	{
	delete expr;
	}

/*
 *	Generate code.
 */

void Uc_return_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	if (expr)			// Returning something?
		{
		expr->gen_value(out);	// Put value on stack.
		out.put((char) UC_SETR);// Pop into ret_value.
		out.put((char) UC_RTS);
		}
	else
		out.put((char) UC_RET);
	}

/*
 *	Generate code.
 */

void Uc_say_statement::gen
	(
	ostream& out,
	Uc_function * /* fun */
	)
	{
	out.put((char) UC_SAY);
	}

/*
 *	Generate code.
 */

void Uc_message_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	if (!msg)
		return;
					// A known string?
	int offset = msg->get_string_offset();
	if (offset >= 0)
		{
		out.put((char) UC_ADDSI);
		Write2(out, offset);
		}
	else
		{
		Uc_var_symbol *var = msg->need_var(out, fun);
		if (var)		// Shouldn't fail.
			{
			out.put((char) UC_ADDSV);
			Write2(out, var->get_offset());
			}
		}
	}

/*
 *	Create.
 */

Uc_call_statement::Uc_call_statement
	(
	Uc_call_expression *f
	) : function_call(f)
	{
	function_call->set_no_return();
	}

/*
 *	Delete.
 */

Uc_call_statement::~Uc_call_statement
	(
	)
	{
	delete function_call;
	}

/*
 *	Generate code.
 */

void Uc_call_statement::gen
	(
	ostream& out,
	Uc_function *fun
	)
	{
	function_call->gen_value(out);	// (We set 'no_return'.)
	}
