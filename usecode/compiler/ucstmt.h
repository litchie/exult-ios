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

#include <vector>
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
 *	A group of statements.
 */
class Uc_block_statement : public Uc_statement
	{
	vector<Uc_statement *> statements;
public:
	Uc_block_statement()
		{  }
	~Uc_block_statement();
	void add(Uc_statement *stmt)
		{ statements.push_back(stmt); }
					// Generate code.
	virtual void gen(ostream& out);
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

/*
 *	An IF statement:
 */
class Uc_if_statement : public Uc_statement
	{
	Uc_expression *expr;		// What to test.
					// What to execute:
	Uc_statement *if_stmt, *else_stmt;
public:
	Uc_if_statement(Uc_expression *e, Uc_statement *t, Uc_statement *f)
		: expr(e), if_stmt(t), else_stmt(f)
		{  }
	~Uc_if_statement();
					// Generate code.
	virtual void gen(ostream& out);
	};

/*
 *	An WHILE statement:
 */
class Uc_while_statement : public Uc_statement
	{
	Uc_expression *expr;		// What to test.
	Uc_statement *stmt;		// What to execute.
public:
	Uc_while_statement(Uc_expression *e, Uc_statement *s)
		: expr(e), stmt(s)
		{  }
	~Uc_while_statement();
					// Generate code.
	virtual void gen(ostream& out);
	};

/*
 *	An RETURN statement:
 */
class Uc_return_statement : public Uc_statement
	{
	Uc_expression *expr;		// What to return.  May be 0.
public:
	Uc_return_statement(Uc_expression *e = 0) : expr(e)
		{  }
	~Uc_return_statement();
					// Generate code.
	virtual void gen(ostream& out);
	};

/*
 *	Print current message on screen (in conversation).
 */
class Uc_say_statement : public Uc_statement
	{
public:
	Uc_say_statement() {  }
					// Generate code.
	virtual void gen(ostream& out);
	};

/*
 *	Add string to current message (for conversations).
 */
class Uc_message_statement : public Uc_statement
	{
	Uc_expression *msg;
public:
	Uc_message_statement(Uc_expression *m) : msg(m) {  }
					// Generate code.
	virtual void gen(ostream& out);
	};

#endif
