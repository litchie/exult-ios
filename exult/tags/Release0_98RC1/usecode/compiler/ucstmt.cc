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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include "ucstmt.h"
#include "ucexpr.h"
#include "ucsym.h"
#include "opcodes.h"
#include "utils.h"
#include "ucfun.h"

using std::vector;
using std::string;

/*
 *	Delete.
 */

Uc_block_statement::~Uc_block_statement
	(
	)
	{
					// Delete all the statements.
	for (std::vector<Uc_statement *>::const_iterator it = statements.begin();
					it != statements.end(); it++)
		delete (*it);
	}

/*
 *	Generate code.
 */

void Uc_block_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
	for (std::vector<Uc_statement *>::const_iterator it = statements.begin();
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
	vector<char>& out,
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
	vector<char>& out,
	Uc_function *fun
	)
	{
#if 1
					// Gen JNE (or CMPS) past IF code.
	int jne_offset = expr->gen_jmp_if_false(out, 0);
	if (if_stmt)
		if_stmt->gen(out, fun);
	int jmp_past_else_offset = -1;	// Where following JMP offset is.
	if (else_stmt)			// JMP past ELSE code.
		{
		out.push_back((char) UC_JMP);
		jmp_past_else_offset = out.size();
		Write2(out, 0);		// Write place-holder for offset.
		}
	int if_end = out.size();
					// Fixup JNE at start.
	Write2(out, jne_offset, if_end - (jne_offset + 2));
	if (else_stmt)			// Generate ELSE.
		{
		else_stmt->gen(out, fun);
		int else_end = out.size();
		Write2(out, jmp_past_else_offset,
					else_end - (jmp_past_else_offset + 2));
		}
#else
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
					// Gen JNE (or CMPS) past IF code.
	expr->gen_jmp_if_false(out, iflen);
	out.write(ifstr, iflen);	// Now the IF code.
	out.write(elsestr, elselen);	// Then the ELSE code.
	delete elsestr;
	delete ifstr;
#endif
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
	vector<char>& out,
	Uc_function *fun
	)
	{
	int top = out.size();		// Get current position.
	expr->gen_value(out);		// Generate expr. value.
	vector<char> stmt_code;
	stmt_code.reserve(4000);
	fun->start_breakable(this);
	if (stmt) {
		fun->adjust_reloffset(out.size() + 3);
		stmt->gen(stmt_code, fun);	// Generate statement's code.
		fun->adjust_reloffset(-out.size() - 3);
	}
	int stmtlen = stmt_code.size();
					// Get distance back to top, including
					//   a JNE and a JMP.
	long dist = stmtlen + (out.size() - top) + 3 + 3;
					// Generate JMP back to top.
	stmt_code.push_back((char) UC_JMP);
	Write2(stmt_code, -dist);
	stmtlen = stmt_code.size();	// Get total length.
	fun->end_breakable(this, stmt_code);
	out.push_back((char) UC_JNE);	// Put cond. jmp. after test.
	Write2(out, stmtlen);		// Skip around body if false.
//	out.append(stmt_code);
					// Append code to end.
	out.insert(out.end(), stmt_code.begin(), stmt_code.end());
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
	vector<char>& out,
	Uc_function *fun
	)
	{
	if (!stmt)
		return;			// Nothing useful to do.
	out.push_back((char) UC_LOOP);	// Start of loop.
	int top = out.size();		// This is where to jump back to.
	out.push_back((char) UC_LOOPTOP);
	Write2(out, index->get_offset());// Counter, total-count variables.
	Write2(out, array_size->get_offset());
	Write2(out, var->get_offset());	// Loop variable, than array.
	Write2(out, array->get_offset());
					// Still need to write offset to end.
	int testlen = out.size() + 2 - top;
	vector<char> stmt_code;
	stmt_code.reserve(4000);
	fun->start_breakable(this);
	fun->adjust_reloffset(out.size() + 2);
	stmt->gen(stmt_code, fun);	// Generate statement's code.
	fun->adjust_reloffset(-out.size() - 2);
					// Back to top includes JMP at end.
	int dist = testlen + stmt_code.size() + 3;
	stmt_code.push_back((char) UC_JMP);	// Generate JMP back to top.
	Write2(stmt_code, -dist);
	int stmtlen = stmt_code.size();	// Get total length.
	Write2(out, stmtlen);		// Finally, offset past loop stmt.
	fun->end_breakable(this, stmt_code);
					// Write out body.
	out.insert(out.end(), stmt_code.begin(), stmt_code.end());
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
	vector<char>& out,
	Uc_function *fun
	)
	{
	if (expr)			// Returning something?
		{
		expr->gen_value(out);	// Put value on stack.
		out.push_back((char) UC_SETR);// Pop into ret_value.
		out.push_back((char) UC_RTS);
		}
	else
		out.push_back((char) UC_RET);
	}


/*
 *	Generate code.
 */

void Uc_break_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
//++++++++NOOOOO.  The location isn't right, since it might be inside
//   and IF block.
	fun->add_break(out.size());	// Store our location.
	out.push_back((char) UC_JMP);
	Write2(out, 0);			// To be filled in at end of loop.
	}


void Uc_label_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
{
	// use reloffset to compensate for jumps to points in separately
	// generated blocks of code
	label->set_offset(out.size() + fun->get_reloffset());

	// no code
}


void Uc_goto_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
{
	Uc_label *l = fun->search_label(label);
	if (l) {
		// use reloffset to compensate for jumps to points in separately
		// generated blocks of code
		l->add_reference(out.size() + fun->get_reloffset());
		out.push_back((char) UC_JMP);
		Write2(out, 0); // will be filled in later
	} else {
		char buf[255];
		snprintf(buf, 255, "Undeclared label: '%s'", label);
		error(buf);
	}
}

/*
 *	Delete.
 */

Uc_converse_statement::~Uc_converse_statement
	(
	)
	{
	delete stmt;
	}

/*
 *	Generate code.
 */

void Uc_converse_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
	long top = out.size();		// Get current position.
	vector<char> stmt_code;
	fun->start_breakable(this);
	if (stmt) {
		fun->adjust_reloffset(out.size() + 3);
		stmt->gen(stmt_code, fun);	// Generate statement's code.
		fun->adjust_reloffset(-out.size() - 3);
	}
	int stmtlen = stmt_code.size();
					// Get distance back to top, including
					//   a CONVERSE & JMP back to top.
	long dist = stmtlen + 3 + 3;
	stmt_code.push_back((char) UC_JMP);	// Generate JMP back to top.
	Write2(stmt_code, -dist);
	stmtlen = stmt_code.size();	// Get total length.
	fun->end_breakable(this, stmt_code);
	out.push_back((char) UC_CONVERSE);	// Put CONVERSE at top.
	Write2(out, stmtlen);		// Skip around body if no choices.
					// Write out body.
	out.insert(out.end(), stmt_code.begin(), stmt_code.end());
	out.push_back((char) UC_CONVERSELOC);	// Past CONVERSE loop.
	}

/*
 *	Generate code.
 */

void Uc_message_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
	if (!msgs)
		return;
	const std::vector<Uc_expression*>& exprs = msgs->get_exprs();
	for (std::vector<Uc_expression*>::const_iterator it = exprs.begin();
					it != exprs.end(); ++it)
		{
		Uc_expression *msg = *it;
					// A known string?
		int offset = msg->get_string_offset();
		if (offset >= 0)
			{
			out.push_back((char) UC_ADDSI);
			Write2(out, offset);
			}
		else
			{
			Uc_var_symbol *var = msg->need_var(out, fun);
			if (var)	// Shouldn't fail.
				{
				out.push_back((char) UC_ADDSV);
				Write2(out, var->get_offset());
				}
			}
		}
	}

/*
 *	Generate code.
 */

void Uc_say_statement::gen
	(
	vector<char>& out,
	Uc_function *fun
	)
	{
					// Add the messages.
	Uc_message_statement::gen(out, fun);
	out.push_back((char) UC_SAY);		// Show on screen.
	}

/*
 *	Create.
 */

Uc_call_statement::Uc_call_statement
	(
	Uc_call_expression *f
	) : function_call(f)
	{
	if (function_call)
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
	vector<char>& out,
	Uc_function *fun
	)
	{
	function_call->gen_value(out);	// (We set 'no_return'.)
	}
