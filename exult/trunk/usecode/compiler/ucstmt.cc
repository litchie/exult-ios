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

#include "ucstmt.h"
#include "ucexpr.h"

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
	ostream& out
	)
	{
	value->gen_value(out);		// Get value on stack.
	target->gen_assign(out);
	}
