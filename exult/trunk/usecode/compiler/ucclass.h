/**
 **	Ucclass.h - Usecode compiler classes.
 **
 **	Written: 8/26/06 - JSF
 **/

/*
Copyright (C) 2006 The Exult Team

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

#ifndef INCL_UCCLASS
#define INCL_UCCLASS

#include "ucsym.h"

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

class Uc_function;

/* 
 *	This represents a usecode class (vaguely like in C++).
 */
class Uc_class : public Uc_design_unit
	{
	std::string name;
	Uc_scope scope;
	int num_vars;			// # member variables.  The 1st one
					//  will be the class info.
	std::vector<Uc_function *> methods;
public:
	Uc_class(char *nm);
	~Uc_class();
	Uc_scope *get_scope()
		{ return &scope; }
	Uc_var_symbol *add_symbol(char *nm);	// Add class variable.
	void add_method(Uc_function *m);
	void gen(std::ostream& out);	// Generate Usecode.
	virtual Usecode_symbol *create_sym();
	};

#endif
