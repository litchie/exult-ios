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

/* 
 *	This represents a usecode function:
 */
class Uc_function
	{
	Uc_scope top;			// Top-level scope.
	Uc_function_symbol *proto;	// Function declaration.
	Uc_scope *cur_scope;		// Current scope.
	int num_locals;			// Counts locals (+parms).
	string text_data;		// All strings.
public:
	Uc_function(Uc_function_symbol *p);
	void push_scope()		// Start a new scope.
		{ cur_scope = cur_scope->add_scope(); }
	void pop_scope()		// End scope.
		{
		Uc_scope *old = cur_scope;
		cur_scope = cur_scope->get_parent();
		delete old;
		}
	Uc_symbol *search(char *nm)	// Search current scope.
		{ return cur_scope->search(nm); }
	Uc_symbol *search_up(char *nm)
		{ return cur_scope->search_up(nm); }
	Uc_symbol *add_symbol(char *nm);// Add symbol to current scope.
					// Add string constant.
	Uc_symbol *add_string_symbol(char *nm, char *text);
	};

#endif


