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
#include <strstream.h>
#include "ucfun.h"
#include "ucstmt.h"
#include "utils.h"

using std::strlen;
using std::memcpy;

Uc_scope Uc_function::globals(0);	// Stores intrinic symbols.

/*
 *	Create function, and add to global symbol table.
 */

Uc_function::Uc_function
	(
	Uc_function_symbol *p
	) : top(0), proto(p), cur_scope(&top), num_parms(0),
	    num_locals(0), text_data(0), text_data_size(0),
	    statement(0)
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
					// NOTE:  We could search for an
					//   existing string & return that!
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
	return offset;
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
	ostrstream code;		// Generate code here first.
	if (statement)
		statement->gen(code, this);
	int codelen = code.pcount();	// Get its length.
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
	for (std::vector<Uc_function_symbol *>::const_iterator it = links.begin();
						it != links.end(); it++)
		Write2(out, (*it)->get_usecode_num());
	char *ucstr = code.str();	// Finally, the code itself.
	out.write(ucstr, codelen);
	delete ucstr;
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
		}
	else
		{
		table = si_intrinsic_table;
		cnt = sizeof(si_intrinsic_table)/sizeof(si_intrinsic_table[0]);
		}
	for (int i = 0; i < cnt; i++)
		{
		char *nm = (char *)table[i];
		if (!globals.search(nm))
					// ++++Later, get num parms.
			globals.add(new Uc_intrinsic_symbol(nm, i));
		}
	}

