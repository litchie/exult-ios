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
 *	A formal parameter or local symbol within a function.  The base class
 *	represents standard Usecode (untyped) variables.
 */
class Uc_symbol
	{
protected:
	string name;			// This will be the key.
	int offset;			// Within function.  Locals follow
					//   formal parameters.
public:
	friend class Uc_scope;
	Uc_symbol(char *nm, int off) : name(nm), offset(off)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual int gen_assign(ostream& out);
	};

/*
 *	A (constant) string:
 */
class Uc_string_symbol : public Uc_symbol
	{
	string text;			// What is stored.
public:
	Uc_string_symbol(char *nm, int off, char *t)
		: Uc_symbol(nm, off), text(t)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual int gen_assign(ostream& out);
	};

/*
 *	A function-prototype symbol:
 */
class Uc_function_symbol : public Uc_symbol
	{
					// Note:  offset = Usecode fun. #.
	vector<char *> parms;	// Parameters.
public:
	Uc_function_symbol(char *nm, int num, vector<char *>& p)
		: Uc_symbol(nm, num), parms(p)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(ostream& out);
					// Gen. to assign from stack.
	virtual int gen_assign(ostream& out);
	};

/*
 *	A 'scope' in the symbol table:
 */
class Uc_scope
	{
	Uc_scope *parent;		// ->parent.
	map<char *, Uc_symbol *> symbols;// For finding syms. by name.
public:
	Uc_scope(Uc_scope *p) : parent(p)
		{  }
	Uc_symbol *search(char *nm)	// Look in this scope.
		{
		map<char *, Uc_symbol *>::const_iterator it = symbols.find(nm);
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
	};

#endif


