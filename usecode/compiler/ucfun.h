/**
 **	Ucfun.h - Usecode compiler function.
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

#ifndef INCL_UCFUN
#define INCL_UCFUN

#include "ucsym.h"
#include "uclabel.h"

class Uc_location;
class Uc_statement;
#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

/* 
 *	This represents a usecode function:
 */
class Uc_function
	{
	static Uc_scope globals;	// For finding intrinsics, funs.
					// Intrinsics, indexed by number:
	static vector<Uc_intrinsic_symbol *> intrinsics;
					// Some intrinsic numbers:
	static int add_answer, remove_answer, push_answers, pop_answers;
	Uc_scope top;			// Top-level scope.
	Uc_function_symbol *proto;	// Function declaration.
	Uc_scope *cur_scope;		// Current scope.
	int num_parms;			// # parameters.
	int num_locals;			// Counts locals.
					// Stack of loops (where 'break' can
					//   be used.
	vector<Uc_statement *> breakables;
	vector<int> breaks;		// Offsets of generated breaks in 
					//   current loop, with loops separated
					//   by -1's.
					// Links to called functions:
	std::vector<Uc_function_symbol *> links;
	std::map<std::string, Uc_label *> labels;
	char *text_data;		// All strings.
	int text_data_size;
					// Map string to its offset.
	std::map<std::string, int> text_map;
	Uc_statement *statement;	// Statement(s) in function.

	int reloffset; // relative offset of the code being generated
public:
	Uc_function(Uc_function_symbol *p);
	~Uc_function();
	enum Intrinsic_type
		{
		bg,			// Black gate.
		si			// Serpent isle.
		};
	static void set_intrinsics(Intrinsic_type ty);
	void set_statement(Uc_statement *s)
		{ statement = s; }
	void adjust_reloffset(int diff) { reloffset += diff; }
	int get_reloffset() const { return reloffset; }
	void push_scope()		// Start a new scope.
		{ cur_scope = cur_scope->add_scope(); }
	void pop_scope()		// End scope.
		{
		cur_scope = cur_scope->get_parent();
		}
	Uc_symbol *search(char *nm)	// Search current scope.
		{ return cur_scope->search(nm); }
	Uc_symbol *search_up(char *nm)
		{ 
		Uc_symbol *sym = cur_scope->search_up(nm);
		return (sym ? sym : globals.search(nm));
		}
	static Uc_intrinsic_symbol *get_intrinsic(int i)
		{ return (i >= 0 && i < intrinsics.size())? intrinsics[i] : 0;}
	static Uc_intrinsic_symbol *get_add_answer()
		{ return get_intrinsic(add_answer); }
	static Uc_intrinsic_symbol *get_remove_answer()
		{ return get_intrinsic(remove_answer); }
	static Uc_intrinsic_symbol *get_push_answers()
		{ return get_intrinsic(push_answers); }
	static Uc_intrinsic_symbol *get_pop_answers()
		{ return get_intrinsic(pop_answers); }
					// Already declared?
	static bool is_dup(Uc_scope *scope, char *nm);
	Uc_var_symbol *add_symbol(char *nm);// Add var. to current scope.
	int add_function_symbol(Uc_function_symbol *fun)
		{ return cur_scope->add_function_symbol(fun); }
	static int add_global_function_symbol(Uc_function_symbol *fun)
		{ return globals.add_function_symbol(fun); }
					// Add string constant.
	Uc_symbol *add_string_symbol(char *nm, char *text);
					// Add int constant.
	Uc_symbol *add_int_const_symbol(char *nm, int value);
	static Uc_symbol *add_global_int_const_symbol(char *nm, int val);
	int add_string(char *text);
	int find_string_prefix(Uc_location& loc, const char *text);
					// Start/end loop.
	void add_label(Uc_label* l) { labels[l->get_name()] = l; }
	Uc_label *search_label(char *nm);

	void start_breakable(Uc_statement *s);
	void end_breakable(Uc_statement *s, vector<char>& stmt_code);
					// Store 'break' location.
	void add_break(int op_offset);	// DANGER:  Offset is filled in when
					//   end_breakable() is called, so the
					//   string this is in better not have
					//   been deleted!!!
					// Link external function.
	int link(Uc_function_symbol *fun);
	void link_labels(std::vector<char>& code);
	void gen(std::ostream& out);		// Generate Usecode.
	};

#endif
