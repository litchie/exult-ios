/**
 **	Ucfun.cc - Usecode compiler function.
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
#include "ucfun.h"
#include "ucstmt.h"
#include "utils.h"
#include "opcodes.h"
#include "ucexpr.h"			/* Needed only for Write2(). */

using std::strlen;
using std::memcpy;

Uc_scope Uc_function::globals(0);	// Stores intrinic symbols.
vector<Uc_intrinsic_symbol *> Uc_function::intrinsics;
int Uc_function::add_answer = -1, Uc_function::remove_answer = -1,
	Uc_function::push_answers = -1, Uc_function::pop_answers = -1,
	Uc_function::show_face = -1, Uc_function::remove_face = -1;

/*
 *	Create function, and add to global symbol table.
 */

Uc_function::Uc_function
	(
	Uc_function_symbol *p
	) : top(0), proto(p), cur_scope(&top), num_parms(0),
	    num_locals(0), text_data(0), text_data_size(0),
	    statement(0), reloffset(0)
	{
	char *nm = (char *) proto->get_name();
	add_global_function_symbol(proto);// Add prototype to globals.
#if 0
	if (!globals.search(nm))		
		globals.add(proto);
	else
		{
		char buf[100];
		sprintf(buf, "Name '%s' already defined", nm);
		Uc_location::yyerror(buf);
		}
#endif
	const std::vector<char *>& parms = proto->get_parms();
	for (std::vector<char *>::const_iterator it = parms.begin();
				it != parms.end(); it++)
		add_symbol(*it);
	num_parms = num_locals;		// Set counts.
	num_locals = 0;
	}

/*
 *	Delete.
 */

Uc_function::~Uc_function
	(
	)
	{
	delete statement;
	delete proto;

	std::map<std::string, Uc_label*>::iterator iter;
	for (iter = labels.begin(); iter != labels.end(); ++iter)
		delete iter->second;
	}

/*
 *	Check for a duplicate symbol and print an error.
 *
 *	Output:	true if dup., with error printed.
 */

bool Uc_function::is_dup
	(
	Uc_scope *scope,
	char *nm
	)
	{
	Uc_symbol *sym = scope->search(nm);
	if (sym)			// Already in scope?
		{
		char msg[180];
		sprintf(msg, "Symbol '%s' already declared", nm);
		Uc_location::yyerror(msg);
		return true;
		}
	return false;
	}

/*
 *  Find a label in this function
 *
 *  Output: label, or 0 if not found
 */

Uc_label *Uc_function::search_label(char *nm)
{
	std::map<std::string,Uc_label*>::iterator iter;
	iter = labels.find(nm);
	if (iter != labels.end())
		return iter->second;
	return 0;
}

/*
 *	Add a new variable to the current scope.
 *
 *	Output:	New sym, or 0 if already declared.
 */

Uc_var_symbol *Uc_function::add_symbol
	(
	char *nm
	)
	{
	if (is_dup(cur_scope, nm))
		return 0;
					// Create & assign slot.
	Uc_var_symbol *var = new Uc_var_symbol(nm, num_parms + num_locals++);
	cur_scope->add(var);
	return var;
	}

/*
 *	Add a new string constant to the current scope.
 */

Uc_symbol *Uc_function::add_string_symbol
	(
	char *nm,
	char *text
	)
	{
	if (is_dup(cur_scope, nm))
		return 0;
					// Create & assign slot.
	Uc_symbol *sym = new Uc_string_symbol(nm, add_string(text));
	cur_scope->add(sym);
	return sym;
	}

/*
 *	Add a new integer constant variable to the current scope.
 *
 *	Output:	New sym, or 0 if already declared.
 */

Uc_symbol *Uc_function::add_int_const_symbol
	(
	char *nm,
	int value
	)
	{
	if (is_dup(cur_scope, nm))
		return 0;
					// Create & assign slot.
	Uc_const_int_symbol *var = new Uc_const_int_symbol(nm, value);
	cur_scope->add(var);
	return var;
	}

/*
 *	Add a new integer constant variable to the global scope.
 *
 *	Output:	New sym, or 0 if already declared.
 */

Uc_symbol *Uc_function::add_global_int_const_symbol
	(
	char *nm,
	int value
	)
	{
	if (is_dup(&globals, nm))
		return 0;
					// Create & assign slot.
	Uc_const_int_symbol *var = new Uc_const_int_symbol(nm, value);
	globals.add(var);
	return var;
	}

/*
 *	Add a string to the data area.
 *
 *	Output:	offset of string.
 */

int Uc_function::add_string
	(
	char *text
	)
	{
					// Search for an existing string.
	map<std::string, int>::const_iterator exist = text_map.find(text);
	if (exist != text_map.end())
		return (*exist).second;
	int offset = text_data_size;	// This is where it will go.
	int textlen = strlen(text) + 1;	// Got to include ending null.
	char *new_text_data = new char[text_data_size + textlen];
	if (text_data_size)		// Copy over old.
		memcpy(new_text_data, text_data, text_data_size);
					// Append new.
	memcpy(new_text_data + text_data_size, text, textlen);
	delete text_data;
	text_data = new_text_data;
	text_data_size += textlen;
	text_map[text] = offset;	// Store map entry.
	return offset;
	}

/*
 *	Find the (unique) string for a given prefix.
 *
 *	Output:	Offset of string.  Error printed if more than one.
 *		0 if not found, with error printed.
 */

int Uc_function::find_string_prefix
	(
	Uc_location& loc,		// For printing errors.
	const char *text
	)
	{
	int len = strlen(text);
					// Find 1st entry >= text.
	map<std::string, int>::const_iterator exist = 
					text_map.lower_bound(text);
	if (exist == text_map.end() ||
	    strncmp(text, (*exist).first.c_str(), len) != 0)
		{
		char *buf = new char[len + 100];
		sprintf(buf, "Prefix '%s' matches no string in this function",
									text);
		loc.error(buf);
		delete buf;
		return 0;
		}
	map<std::string, int>::const_iterator next = exist;
	++next;
	if (next != text_map.end() &&
	    strncmp(text, (*next).first.c_str(), len) == 0)
		{
		char *buf = new char[len + 100];
		sprintf(buf, "Prefix '%s' matches more than one string", text);
		loc.error(buf);
		delete buf;
		}
	return (*exist).second;		// Return offset.
	}

/*
 *	Start a loop.
 */

void Uc_function::start_breakable
	(
	Uc_statement *s			// Loop.
	)
	{
	breakables.push_back(s);
	breaks.push_back(-1);		// Set marker in 'break' list.
	}

/*
 *	Fix up stuff when a loop's body has been generated.
 */

void Uc_function::end_breakable
	(
	Uc_statement *s,		// Loop.  For verification.
	vector<char>& stmt_code
	)
	{
					// Just make sure things are right.
	assert(!breakables.empty() && s == breakables.back());
	breakables.pop_back();
	int stmtlen = stmt_code.size();
					// Fix all the 'break' statements,
					//   going backwards.
	while (!breaks.empty() && breaks.back() >= 0)
		{			// Get offset within loop.
		int break_offset = breaks.back();
		breaks.pop_back();	// Remove from end of list.
		assert(break_offset < stmtlen - 2);
					// Store offset.
		Write2(stmt_code, break_offset + 1, 
						stmtlen - (break_offset + 3));
		}
	assert(!breaks.empty() && breaks.back() == -1);
	breaks.pop_back();		// Remove marker (-1).
	}

/*
 *	Store a 'break' statement's offset so it can be filled in at the end
 *	of the current loop.
 */

void Uc_function::add_break
	(
	int op_offset			// Offset in loop's code of JMP.
	)
	{
	assert(op_offset >= 0);
	if (breakables.empty())		// Not in a loop?
		Uc_location::yyerror("'break' is not valid here");
	else
		breaks.push_back(op_offset);
	}

/*
 *	Lookup/add a link to an external function.
 *
 *	Output:	Link offset.
 */

int Uc_function::link
	(
	Uc_function_symbol *fun
	)
	{
	for (std::vector<Uc_function_symbol *>::const_iterator it = links.begin();
						it != links.end(); it++)
		if (*it == fun)		// Found it?  Return offset.
			return (it - links.begin());
	int offset = links.size();	// Going to add it.
	links.push_back(fun);
	return offset;
	}


void Uc_function::link_labels(vector<char>& code)
{
	std::map<std::string, Uc_label*>::iterator iter;
	for (iter = labels.begin(); iter != labels.end(); ++iter) {
		Uc_label *label = iter->second;
		if (label->is_valid()) {
			int target = label->get_offset(); 
			std::vector<int>& references = label->get_references();
			std::vector<int>::iterator i;
			for (i = references.begin(); i != references.end(); ++i) {
				int offset = (*i) + 1;
				Write2(code, offset, target - (offset + 2));
			}
		}
	}
}

/*
 *	Generate Usecode.
 */

void Uc_function::gen
	(
	std::ostream& out
	)
	{
					// Start with function #.
	Write2(out, proto->get_usecode_num());
	vector<char> code;		// Generate code here first.
	code.reserve(30000);
	if (statement)
		statement->gen(code, this);
	code.push_back((char) UC_RET);	// Always end with a RET.
	link_labels(code);
	int codelen = code.size();	// Get its length.
	int num_links = links.size();
					// Total: text_data_size + data + 
					//   #args + #locals + #links + links +
					//   codelen.
	int totallen =  2 + text_data_size + 2 + 2 + 2 + 2*num_links + codelen;
	Write2(out, totallen);
	Write2(out, text_data_size);		// Now data.
	out.write(text_data, text_data_size);
	Write2(out, num_parms);		// Counts.
	Write2(out, num_locals);
	Write2(out, num_links);
					// Write external links.
	for (std::vector<Uc_function_symbol *>::const_iterator it = 
				links.begin(); it != links.end(); it++)
		Write2(out, (*it)->get_usecode_num());
	char *ucstr = &code[0];		// Finally, the code itself.
	out.write(ucstr, codelen);
	out.flush();
	}

#ifndef __STRING
#if defined __STDC__ && __STDC__
#define __STRING(x) #x
#else
#define __STRING(x) "x"
#endif
#endif

/*
 *	Tables of usecode intrinsics:
 */
#define	USECODE_INTRINSIC_PTR(NAME)	__STRING(UI_##NAME)

const char *bg_intrinsic_table[] =
	{
#include "../bgintrinsics.h"
	};

const char *si_intrinsic_table[] = 
	{
#include "../siintrinsics.h"
	};

/*
 *	Add one of the intrinsic tables to the 'intrinsics' scope.
 */

void Uc_function::set_intrinsics
	(
	Intrinsic_type ty
	)
	{
	int cnt;
	const char **table;
	if (ty == bg)
		{
		table = bg_intrinsic_table;
		cnt = sizeof(bg_intrinsic_table)/sizeof(bg_intrinsic_table[0]);
		add_answer = 5;
		remove_answer = 6;
		push_answers = 7;
		pop_answers = 8;
		}
	else
		{
		table = si_intrinsic_table;
		cnt = sizeof(si_intrinsic_table)/sizeof(si_intrinsic_table[0]);
		add_answer = 0xc;
		remove_answer = 0xd;
		push_answers = 0xe;
		pop_answers = 0xf;
		}
	show_face = 3;
	remove_face = 4;
	intrinsics.resize(cnt);
	for (int i = 0; i < cnt; i++)
		{
		char *nm = (char *)table[i];
		Uc_intrinsic_symbol *sym = new Uc_intrinsic_symbol(nm, i);
		intrinsics[i] = sym;	// Store in indexed list.
		if (!globals.search(nm))
					// ++++Later, get num parms.
			globals.add(sym);
		}
	}

