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

#include <string>
#include <map>
#include <vector>

/*
 *	A formal parameter or local symbol within a function.
 */
class Uc_symbol
	{
protected:
	string name;			// This will be the key.
public:
	friend class Uc_scope;
	Uc_symbol(char *nm) : name(nm)
		{  }
	const char *get_name() { return name.data(); }
					// Gen. code to put result on stack.
	virtual int gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual int gen_assign(ostream& out);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return -1; }
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
	virtual int gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual int gen_assign(ostream& out);
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
	virtual int gen_value(ostream& out);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return offset; }
	};

/*
 *	A function-prototype symbol:
 */
class Uc_function_symbol : public Uc_symbol
	{
					// Note:  offset = Usecode fun. #.
	vector<char *> parms;		// Parameters.
	int usecode_num;		// Usecode function #.
public:
	Uc_function_symbol(char *nm, int num, vector<char *>& p)
		: Uc_symbol(nm), parms(p), usecode_num(num)
		{  }
	const vector<char *>& get_parms()
		{ return parms; }
	int get_usecode_num()
		{ return usecode_num; }
	int get_num_parms()
		{ return parms.size(); }
	};

/*
 *	For comparing names:
 */
class String_compare
	{
public:
	bool operator()(char * const &x, char * const &y) const;
	};

/*
 *	A 'scope' in the symbol table:
 */
class Uc_scope
	{
	Uc_scope *parent;		// ->parent.
	typedef map<char *, Uc_symbol *, String_compare> Sym_map;
	Sym_map symbols;		// For finding syms. by name.
	vector<Uc_scope *> scopes;	// Scopes within.
public:
	Uc_scope(Uc_scope *p) : parent(p)
		{  }
	~Uc_scope();
	Uc_scope *get_parent()
		{ return parent; }
	Uc_symbol *search(char *nm)	// Look in this scope.
		{
		Sym_map::const_iterator it = symbols.find(nm);
		if (it == symbols.end())
			return 0;
		else
			return (*it).second;
		}
	Uc_symbol *search_up(char *nm);	// Search upwards through scopes.
	void add(Uc_symbol *sym)	// Add (does NOT check for dups.)
		{
		char *nm = (char *) sym->name.data();
		symbols[nm] = sym; 
		}
	Uc_scope *add_scope()		// Create new scope.
		{
		Uc_scope *newscope = new Uc_scope(this);
		scopes.push_back(newscope);
		return newscope;
		}
	};

#endif


