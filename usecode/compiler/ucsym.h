/**
 **	Ucsym.h - Usecode compiler symbol table.
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

#ifndef INCL_UCSYM
#define INCL_UCSYM

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string>
#include <map>
#include <vector>
using std::vector;

class Uc_array_expression;
class Uc_expression;
class Uc_function;

/*
 *	For comparing names:
 */
class String_compare
	{
public:
	bool operator()(char * const &x, char * const &y) const;
	};

/*
 *	A formal parameter or local symbol within a function.
 */
class Uc_symbol
	{
protected:
	std::string name;			// This will be the key.
public:
	friend class Uc_scope;
	Uc_symbol(char *nm) : name(nm)
		{  }
	const char *get_name() { return name.c_str(); }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual int gen_assign(vector<char>& out);
					// Generate function/procedure call.
	virtual int gen_call(vector<char>& out, Uc_function *fun, 
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return -1; }
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	};

/*
 *	A variable (untyped) that can be assigned to.
 */
class Uc_var_symbol : public Uc_symbol
	{
protected:
	int offset;			// Within function.  Locals follow
					//   formal parameters.
public:
	friend class Uc_scope;
	Uc_var_symbol(char *nm, int off) : Uc_symbol(nm), offset(off)
		{  }
	int get_offset()
		{ return offset; }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual int gen_assign(vector<char>& out);
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	};

/*
 *	A constant integer variable.
 */
class Uc_const_int_symbol : public Uc_symbol
	{
	int value;
public:
	Uc_const_int_symbol(char *nm, int v) : Uc_symbol(nm), value(v)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	};

/*
 *	A (constant) string.  The offset is within the usecode function's
 *	text_data field.
 */
class Uc_string_symbol : public Uc_symbol
	{
	int offset;			// In function's text_data.
public:
	Uc_string_symbol(char *nm, int off) : Uc_symbol(nm), offset(off)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return offset; }
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	};

/*
 *	An intrinsic symbol:
 */
class Uc_intrinsic_symbol : public Uc_symbol
	{
	int intrinsic_num;		// Intrinsic #.
	int num_parms;			// # parms. +++++Not used/set yet.
public:
	Uc_intrinsic_symbol(char *nm, int n) : Uc_symbol(nm), intrinsic_num(n),
		num_parms(0)
		{  }
	int get_intrinsic_num()
		{ return intrinsic_num; }
	int get_num_parms()		// ++++Not valid yet.
		{ return num_parms; }
					// Generate function/procedure call.
	virtual int gen_call(vector<char>& out, Uc_function *fun, 
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue);
	};

/*
 *	A function-prototype symbol:
 */
class Uc_function_symbol : public Uc_symbol
	{
					// Note:  offset = Usecode fun. #.
	std::vector<char *> parms;	// Parameters.
	int usecode_num;		// Usecode function #.
public:
	Uc_function_symbol(char *nm, int num, std::vector<char *>& p)
		: Uc_symbol(nm), parms(p), usecode_num(num)
		{  }
	const std::vector<char *>& get_parms()
		{ return parms; }
	int get_usecode_num()
		{ return usecode_num; }
	int get_num_parms()
		{ return parms.size(); }
					// Generate function/procedure call.
	virtual int gen_call(vector<char>& out, Uc_function *fun, 
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue);
	};

/*
 *	A 'scope' in the symbol table:
 */
class Uc_scope
	{
	Uc_scope *parent;		// ->parent.
					// For finding syms. by name.
	typedef std::map<char *, Uc_symbol *, String_compare> Sym_map;
	Sym_map symbols;
	std::vector<Uc_scope *> scopes;	// Scopes within.
public:
	Uc_scope(Uc_scope *p) : parent(p)
		{  }
	~Uc_scope();
	Uc_scope *get_parent()
		{ return parent; }
	Uc_symbol *search(const char *nm)	// Look in this scope.
		{
		char *nm1 = (char *) nm;
		Sym_map::const_iterator it = symbols.find(nm1);
		if (it == symbols.end())
			return 0;
		else
			return (*it).second;
		}
					// Search upwards through scopes.
	Uc_symbol *search_up(char *nm);
	void add(Uc_symbol *sym)	// Add (does NOT check for dups.)
		{
		const char *nm = sym->name.c_str();
		char *nm1 = (char *)nm;	// ???Can't figure this out!
		symbols[nm1] = sym; 
		}
	Uc_scope *add_scope()		// Create new scope.
		{
		Uc_scope *newscope = new Uc_scope(this);
		scopes.push_back(newscope);
		return newscope;
		}
					// Add a function decl.
	int add_function_symbol(Uc_function_symbol *fun);
	};

#endif


