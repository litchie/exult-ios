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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <cassert>

#include "ucclass.h"
#include "ucsymtbl.h"

/*
 *	Create.
 */

Uc_class::Uc_class
	(
	char *nm
	) : scope(0), num_vars(0), name(nm)
	{
	}

/*
 *	Cleanup.
 */
Uc_class::~Uc_class
	(
	)
	{
	}

/*
 *	Add a new class variable.
 *
 *	Output:	New sym, or 0 if already declared.
 */

Uc_var_symbol *Uc_class::add_symbol
	(
	char *nm
	)
	{
	if (scope.is_dup(nm))
		return 0;
					// Create & assign slot.
	Uc_var_symbol *var = new Uc_class_var_symbol(nm, num_vars++);
	scope.add(var);
	return var;
	}

/*
 *	Generate Usecode.
 */

void Uc_class::gen
	(
	std::ostream& out
	)
	{
	std::vector<Uc_design_unit *>::iterator it;
	for (it = methods.begin(); it != methods.end(); it++)
		{
		(*it)->gen(out);		// Generate function.
		}
	}

/*
 *	Create symbol for this function.
 */

Usecode_symbol *Uc_class::create_sym
	(
	)
	{
	Usecode_symbol::Symbol_kind kind = Usecode_symbol::class_scope;
	Usecode_scope_symbol *cs = 
		new Usecode_scope_symbol(name.c_str(), kind);
	std::vector<Uc_design_unit *>::iterator it;
	for (it = methods.begin(); it != methods.end(); it++)
		cs->add_sym((*it)->create_sym());
	return cs;
	}

