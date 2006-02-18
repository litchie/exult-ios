/*
 *	ucsymtbl.cc - Usecode symbol table
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

#include <iostream>
#include "ucsymtbl.h"
#include "utils.h"

using std::string;
using std::istream;
using std::ostream;
/*
 *	Cleanup.
 */
Usecode_symbol_table::~Usecode_symbol_table()
{
	for (Syms_vector::iterator it = symbols.begin(); it != symbols.end();
									++it) {
		Usecode_symbol *sym = *it;
		delete sym;
	}
}

/*
 *	Read from a file.
 */
void Usecode_symbol_table::read(istream& in)
{
	int cnt = Read4(in);
	symbols.reserve(symbols.size() + cnt);
	for (int i = 0; i < cnt; ++i) {
		string nm;
		in >> nm;
		int kind = Read2(in);
		int val = Read4(in);
		symbols.push_back(new Usecode_symbol(nm.c_str(),
			(Usecode_symbol::Symbol_kind) kind, val));
	}
}

/*
 *	Write to a file.
 */
void Usecode_symbol_table::write(ostream& out)
{
	Write4(out, symbols.size());
	for (Syms_vector::iterator it = symbols.begin(); it != symbols.end();
									++it) {
		Usecode_symbol *sym = *it;
		out << sym->get_name();
		Write2(out, (int) sym->get_kind());
		Write4(out, sym->get_val());
	}
}

/*
 *	Setup tables.
 */
void Usecode_symbol_table::setup_by_name()
{
	for (Syms_vector::iterator it = symbols.begin(); it != symbols.end();
									++it) {
		Usecode_symbol *sym = *it;
		by_name[sym->name] = sym;
	}
}
void Usecode_symbol_table::setup_by_val()
{
	for (Syms_vector::iterator it = symbols.begin(); it != symbols.end();
									++it) {
		Usecode_symbol *sym = *it;
		by_val[sym->val] = sym;
	}
}

/*
 *	Lookup by name or by value.
 */
Usecode_symbol *Usecode_symbol_table::operator[](const char *nm)
{
	if (by_name.empty())
		setup_by_name();
	Name_table::iterator it = by_name.find(nm);
	if (it == by_name.end())
		return 0;
	else
		return (*it).second;
}

Usecode_symbol *Usecode_symbol_table::operator[](int val)
{
	if (by_val.empty())
		setup_by_val();
	Val_table::iterator it = by_val.find(val);
	if (it == by_val.end())
		return 0;
	else
		return (*it).second;
}

