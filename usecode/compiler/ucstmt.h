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
#include <string>
#include <map>
#include "ucloc.h"

class Uc_expression;
class Uc_call_expression;
class Uc_array_expression;
class Uc_function;
class Uc_var_symbol;
class Uc_del_expression;
class Basic_block;

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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0) = 0;
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	A generic breakable statement:
 */
class Uc_breakable_statement : public Uc_statement
	{
	Uc_statement *stmt;		// What to execute.
public:
	Uc_breakable_statement(Uc_statement *s)
		: stmt(s)
		{  }
	~Uc_breakable_statement();
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	A WHILE statement:
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	A DO...WHILE statement:
 */
class Uc_dowhile_statement : public Uc_statement
	{
	Uc_expression *expr;		// What to test.
	Uc_statement *stmt;		// What to execute.
public:
	Uc_dowhile_statement(Uc_expression *e, Uc_statement *s)
		: expr(e), stmt(s)
		{  }
	~Uc_dowhile_statement();
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	An infinite loop statement:
 */
class Uc_infinite_loop_statement : public Uc_statement
	{
	Uc_statement *stmt;		// What to execute.
public:
	Uc_infinite_loop_statement(Uc_statement *s)
		: stmt(s)
		{  }
	~Uc_infinite_loop_statement();
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	BREAK statement:
 */
class Uc_break_statement : public Uc_statement
	{
public:
	Uc_break_statement() {  }
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	CONTINUE statement:
 */
class Uc_continue_statement : public Uc_statement
	{
public:
	Uc_continue_statement() {  }
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *  a LABEL statement:
 */
class Uc_label_statement : public Uc_statement
	{
	std::string label;
public:
	Uc_label_statement(char *nm) : label(nm)
		{ }
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *  a GOTO statement:
 */
class Uc_goto_statement : public Uc_statement
	{
	std::string label;
public:
	Uc_goto_statement(char *nm) : label(nm)
		{ }
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	Conversation CASE statement:
 */
class Uc_converse_case_statement : public Uc_statement
	{
	std::vector<int> string_offset;		// Offset of string to compare.
	bool remove;			// True to remove answer.
	Uc_statement *statements;	// Execute these.
public:
	Uc_converse_case_statement(std::vector<int> soff, bool rem, Uc_statement *stmts)
		: string_offset(soff), remove(rem), statements(stmts)
		{  }
	~Uc_converse_case_statement()
		{ delete statements; }
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	A CONVERSE2 statement provides a less wordy way to implement a
 *	conversation.  It provides for CASE entries for the comparisons, and
 *	also generates push/pop-answers so these can be nested.
 */
class Uc_converse2_statement : public Uc_statement
	{
	static int nest;		// Keeps track of nesting.
	Uc_expression *answers;		// Answers to add.
	Uc_statement *cases;		// What to execute.
public:
	Uc_converse2_statement(Uc_expression *a, Uc_statement *cs)
		: answers(a), cases(cs)
		{  }
	~Uc_converse2_statement();
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	Switch CASE statement:
 */
class Uc_switch_case_statement : public Uc_statement
	{
	Uc_statement *statements;	// Execute these.
public:
	Uc_switch_case_statement(Uc_statement *stmts)
		: statements(stmts)
		{  }
	~Uc_switch_case_statement()
		{ delete statements; }
	virtual bool is_default() const
		{ return false; }
					// Generate code.
	virtual int gen_check(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *case_block = 0)=0;
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	Switch CASE statement:
 */
class Uc_switch_default_case_statement : public Uc_switch_case_statement
	{
public:
	Uc_switch_default_case_statement(Uc_statement *stmts)
		: Uc_switch_case_statement(stmts)
		{  }
	~Uc_switch_default_case_statement()
		{  }
	virtual bool is_default() const
		{ return true; }
					// Generate code.
	virtual int gen_check(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *case_block = 0)
		{ return 1; }
	};

/*
 *	Switch CASE statement:
 */
class Uc_switch_expression_case_statement : public Uc_switch_case_statement
	{
	Uc_expression *check;		// The case we are interested in.
public:
	Uc_switch_expression_case_statement(Uc_expression *c, Uc_statement *stmts)
		: Uc_switch_case_statement(stmts), check(c)
		{  }
	~Uc_switch_expression_case_statement();
					// Generate code.
	virtual int gen_check(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *case_block = 0);
	};

/*
 *	C++-style switch statement. Just a less wordy form for
 *	long if/else if blocks which check different values of
 *	the same variable.
 */
class Uc_switch_statement : public Uc_statement
	{
	Uc_expression *cond;		// Var to check values of.
	std::vector<Uc_statement *> cases;		// What to execute.
public:
	Uc_switch_statement(Uc_expression *v, std::vector<Uc_statement *> *cs);
	~Uc_switch_statement();
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
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
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	Statement that represents an abort opcode.
 */
class Uc_abort_statement : public Uc_statement
	{
public:
	Uc_abort_statement()
		{  }
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

/*
 *	Statement that just represents a single opcode.
 */
class Uc_delete_statement : public Uc_statement
	{
	Uc_del_expression *expr;
public:
	Uc_delete_statement(Uc_del_expression *e) : expr(e)
		{  }
					// Generate code.
	virtual void gen(Uc_function *fun, std::vector<Basic_block *>& blocks,
			Basic_block *&curr, Basic_block *end,
			std::map<std::string, Basic_block*>& labels,
			Basic_block *start = 0, Basic_block *exit = 0);
	};

#endif
