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
#include "uclabel.h"

class Uc_expression;
class Uc_call_expression;
class Uc_array_expression;
class Uc_function;
class Uc_var_symbol;

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

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
	virtual void gen(std::vector<char>& out, Uc_function *fun) = 0;
	};

/*
 *	A group of statements.
 */
class Uc_block_statement : public Uc_statement
	{
		std::vector<Uc_statement *> statements;
public:
	Uc_block_statement()
		{  }
	~Uc_block_statement();
	void add(Uc_statement *stmt)
		{ statements.push_back(stmt); }
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
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
	virtual void gen(std::vector<char>& out, Uc_function *fun);
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
	virtual void gen(std::vector<char>& out, Uc_function *fun);
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
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *	An array loop statement:
 */
class Uc_arrayloop_statement : public Uc_statement
	{
	Uc_var_symbol *var;		// Loop variable.
	Uc_var_symbol *array;		// Array to loop over.
	Uc_var_symbol *index;		// Counter.
	Uc_var_symbol *array_size;	// Symbol holding array size.
	Uc_statement *stmt;		// What to execute.
public:
	Uc_arrayloop_statement(Uc_var_symbol *v, Uc_var_symbol *a)
		: var(v), array(a), index(0), array_size(0), stmt(0)
		{  }
	~Uc_arrayloop_statement();
	void set_statement(Uc_statement *s)
		{ stmt = s; }
	void set_index(Uc_var_symbol *i)
		{ index = i; }
	void set_array_size(Uc_var_symbol *as)
		{ array_size = as; }
	void finish(Uc_function *fun);	// Create tmps. if necessary.
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
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
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *	BREAK statement:
 */
class Uc_break_statement : public Uc_statement
	{
public:
	Uc_break_statement() {  }
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *  a LABEL statement:
 */
class Uc_label_statement : public Uc_statement
{
	Uc_label *label;
 public:
	Uc_label_statement(Uc_label *l) : label(l)
		{ }
	virtual void gen(std::vector<char>& out, Uc_function *fun);
};

/*
 *  a GOTO statement:
 */
class Uc_goto_statement : public Uc_statement
{
	char *label;
 public:
	Uc_goto_statement(char *l) : label(l)
		{ }
	virtual void gen(std::vector<char>& out, Uc_function *fun);
};



/*
 *	A CONVERSE statement is a loop that prompts for a user response at
 *	the top, or exits the loop if there are no possible answers.
 */
class Uc_converse_statement : public Uc_statement
	{
	Uc_statement *stmt;		// What to execute.
public:
	Uc_converse_statement(Uc_statement *s)
		: stmt(s)
		{  }
	~Uc_converse_statement();
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *	Add string to current message (for conversations).
 */
class Uc_message_statement : public Uc_statement
	{
	Uc_array_expression *msgs;
public:
	Uc_message_statement(Uc_array_expression *m) : msgs(m) {  }
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *	Print message on screen (in conversation).
 */
class Uc_say_statement : public Uc_message_statement
	{
public:
	Uc_say_statement(Uc_array_expression *m) : Uc_message_statement(m) 
		{  }
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

/*
 *	Call a function/intrinsic.
 */
class Uc_call_statement : public Uc_statement
	{
	Uc_call_expression *function_call;
public:
	Uc_call_statement(Uc_call_expression *f);
	~Uc_call_statement();
					// Generate code.
	virtual void gen(std::vector<char>& out, Uc_function *fun);
	};

#endif
