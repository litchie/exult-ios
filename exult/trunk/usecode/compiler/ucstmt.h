/**
 **	Ucstmt.h - Usecode compiler statements.
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

#ifndef INCL_UCSTMT
#define INCL_UCSTMT

#include "ucloc.h"

class ostream;
class Uc_expression;

/*
 *	A statement:
 */
class Uc_statement : public Uc_location
	{
public:
	Uc_statement() : Uc_location()
		{  }
	virtual ~Uc_statement() {  }
					// Generate code.
	virtual void gen(ostream& out) = 0;
	};

/*
 *	An assignment statement:
 */
class Uc_assignment_statement : public Uc_statement
	{
	Uc_expression *target, *value;
public:
	Uc_assignment_statement(Uc_expression *t, Uc_expression *v)
		: target(t), value(v)
		{  }
	~Uc_assignment_statement();
					// Generate code.
	virtual void gen(ostream& out);
	};

#endif
