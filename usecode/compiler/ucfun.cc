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

#include <stdio.h>
#include <strstream.h>
#include "ucfun.h"
#include "ucstmt.h"
#include "utils.h"

/*
 *	Create function.
 */

Uc_function::Uc_function
	(
	Uc_function_symbol *p
	) : top(0), proto(p), cur_scope(&top), num_parms(0),
		num_locals(0), num_links(0), statement(0)
	{
	const vector<char *>& parms = proto->get_parms();
	for (vector<char *>::const_iterator it = parms.begin();
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
 *	Add a new variable to the current scope.
 */

Uc_symbol *Uc_function::add_symbol
	(
	char *nm
	)
	{
	Uc_symbol *sym = search(nm);
	if (sym)			// Already in scope?
		{
		char msg[180];
		sprintf(msg, "Symbol '%s' already declared", nm);
		Uc_location::yyerror(msg);
		return sym;
		}
					// Create & assign slot.
	sym = new Uc_symbol(nm, num_parms + num_locals++);
	cur_scope->add(sym);
	return sym;
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
	Uc_symbol *sym = search(nm);
	if (sym)			// Already in scope?
		{
		char msg[180];
		sprintf(msg, "Symbol '%s' already declared", nm);
		Uc_location::yyerror(msg);
		return sym;
		}
					// Create & assign slot.
	sym = new Uc_string_symbol(nm, text_data.length());
	text_data.append(text);
	cur_scope->add(sym);
	return sym;
	}

/*
 *	Generate Usecode.
 */

void Uc_function::gen
	(
	ostream& out
	)
	{
					// Start with function #.
	Write2(out, proto->get_function_num());
	ostrstream code;		// Generate code here first.
	if (statement)
		statement->gen(code);
	int codelen = code.pcount();	// Get its length.
					// Get data length.
	int datalen = text_data.length();
					// Total: datalen + data + #args +
					//   #locals + #links + links +
					//   codelen.
	int totallen =  2 + datalen + 2 + 2 + 2 + 2*num_links + codelen;
	Write2(out, totallen);
	Write2(out, datalen);		// Now data.
	out.write(text_data.data(), datalen);
	Write2(out, num_parms);		// Counts.
	Write2(out, num_locals);
	Write2(out, 0);			// ++++Num_links.
	char *ucstr = code.str();	// Finally, the code itself.
	out.write(ucstr, codelen);
	delete ucstr;
	out.flush();
	}


