/**
 **	Ucfun.h - Usecode compiler function.
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

#ifndef INCL_UCFUN
#define INCL_UCFUN

#include "ucsym.h"

class Uc_statement;
class ostream;

/* 
 *	This represents a usecode function:
 */
class Uc_function
	{
	static Uc_scope globals;	// For finding intrinsics, funs.
	Uc_scope top;			// Top-level scope.
	Uc_function_symbol *proto;	// Function declaration.
	Uc_scope *cur_scope;		// Current scope.
	int num_parms;			// # parameters.
	int num_locals;			// Counts locals.
					// Links to called functions:
	vector<Uc_function_symbol *> links;
	char *text_data;		// All strings.
	int text_data_size;
	Uc_statement *statement;	// Statement(s) in function.
public:
	Uc_function(Uc_function_symbol *p);
	~Uc_function();
	enum Intrinsic_type
		{
		bg,			// Black gate.
		si			// Serpent isle.
		};
	static void set_intrinsics(Intrinsic_type ty);
	void set_statement(Uc_statement *s)
		{ statement = s; }
	void push_scope()		// Start a new scope.
		{ cur_scope = cur_scope->add_scope(); }
	void pop_scope()		// End scope.
		{
		cur_scope = cur_scope->get_parent();
		}
	Uc_symbol *search(char *nm)	// Search current scope.
		{ return cur_scope->search(nm); }
	Uc_symbol *search_up(char *nm)
		{ 
		Uc_symbol *sym = cur_scope->search_up(nm);
		return (sym ? sym : globals.search(nm));
		}
	Uc_var_symbol *add_symbol(char *nm);// Add symbol to current scope.
					// Add string constant.
	Uc_symbol *add_string_symbol(char *nm, char *text);
	int add_string(char *text);
					// Link external function.
	int link(Uc_function_symbol *fun);
	void gen(ostream& out);		// Generate Usecode.
	};

#endif


