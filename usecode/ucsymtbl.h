/*
 *	ucsymtbl.h - Usecode symbol table
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _UCSYMTBL_H
#define _UCSYMTBL_H

#include <iosfwd>
#include <string>
#include <vector>
#include <map>

#define UCSYMTBL_MAGIC0	0xffffffff
#define UCSYMTBL_MAGIC1	(((long)'U'<<24)+((long)'C'<<16)+((long)'S'<<8)+'Y')

class Usecode_symbol_table;

class Usecode_symbol {
public:
	enum Symbol_kind { 
		fun_defined = 1, 
		fun_externed,
		fun_extern_defined	// External, but fun. # was given.
	};
private:
	friend class Usecode_symbol_table;
	std::string name;
	Symbol_kind kind;
	int val;			// Function #.
public:
	Usecode_symbol(const char *nm, Symbol_kind k, int v)
		: name(nm), kind(k), val(v)
		{  }
	const char *get_name() const
		{ return name.c_str(); }
	Symbol_kind get_kind() const
		{ return kind; }
	int get_val() const
		{ return val; }
	
};

/*
 *	This class represents a symbol table read in from a Usecode file.  This
 *	is an Exult construct; ie, U7's 'usecode' file doesn't have a symbol-
 *	table.
 */
class Usecode_symbol_table {
	typedef std::vector<Usecode_symbol *> Syms_vector;
	Syms_vector symbols;		// All symbols.
	typedef std::map<std::string, Usecode_symbol *> Name_table;
	typedef std::map<int, Usecode_symbol *> Val_table;
	Name_table by_name;
	Val_table by_val;
	void setup_by_name(int start = 0);
	void setup_by_val(int start = 0);
public:
	Usecode_symbol_table() {  }
	~Usecode_symbol_table();
	void read(std::istream& in);
	void write(std::ostream& out);
	void add_sym(Usecode_symbol *sym);
	Usecode_symbol *operator[](const char *nm);
	Usecode_symbol *operator[](int val);
};

#endif
