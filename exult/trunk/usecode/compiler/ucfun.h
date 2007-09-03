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
class Uc_function : public Uc_design_unit
	{
	static Uc_scope globals;	// For finding intrinsics, funs.
					// Intrinsics, indexed by number:
	static vector<Uc_intrinsic_symbol *> intrinsics;
					// Some intrinsic numbers:
	static int add_answer, remove_answer, push_answers, pop_answers,
		show_face, remove_face, get_item_shape, get_usecode_fun;
	static int num_global_statics;
	Uc_scope top;			// Top-level scope.
	Uc_function_symbol *proto;	// Function declaration.
	Uc_scope *cur_scope;		// Current scope.
	int num_parms;			// # parameters.
	int num_locals;			// Counts locals.
	int num_statics;		// Counts local statics.
					// Stack of loops (where 'break' can
					//   be used.
	vector<Uc_statement *> breakables;
	vector<int> breaks;		// Offsets of generated breaks in 
					//   current loop, with loops separated
					//   by -1's.
	vector<int> conts;		// Offsets of generated continues in 
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
	enum Intrinsic_type
		{
		unset,
		bg,			// Black gate.
		si			// Serpent isle.
		};
private:
	static Intrinsic_type intrinsic_type;
public:
	Uc_function(Uc_function_symbol *p, Uc_scope *parent = 0);
	~Uc_function();
	static void set_intrinsics();
	static void setup_intrinsics()		// Init. the 1st time.
		{ if (!intrinsics.size())
			set_intrinsics();
		}
	static void set_intrinsic_type(Intrinsic_type ty)
		{
		intrinsic_type = ty;
		if (ty != Uc_function::unset)
			set_intrinsics();
		}
	void set_statement(Uc_statement *s)
		{ statement = s; }
	const char *get_name()
		{ return proto->get_name(); }
	bool is_externed()
		{ return proto->is_externed(); }
	bool is_inherited()
		{ return proto->is_inherited(); }
	void set_inherited()
		{ proto->set_inherited(); }
	int get_usecode_num()
		{ return proto->get_usecode_num(); }
	void set_method_num(int n)
		{ proto->set_method_num(n); }
	int get_method_num()
		{ return proto->get_method_num(); }
	virtual bool get_has_ret() const
		{ return proto->get_has_ret(); }
	virtual Uc_class *get_cls() const
		{ return proto->get_cls(); }
	virtual Uc_function_symbol::Function_kind get_function_type() const
		{ return proto->get_function_type(); }
	void adjust_reloffset(int diff) { reloffset += diff; }
	int get_reloffset() const { return reloffset; }
	Uc_scope *get_parent()
		{ return cur_scope->get_parent(); }
	void push_scope()		// Start a new scope.
		{ cur_scope = cur_scope->add_scope(); }
	void pop_scope()		// End scope.
		{
		cur_scope = cur_scope->get_parent();
		}
	Uc_symbol *search(char *nm)	// Search current scope.
		{ return cur_scope->search(nm); }
	Uc_symbol *search_up(const char *nm)
		{ 
		Uc_symbol *sym = cur_scope->search_up(nm);
		if (sym)
			return sym;
		setup_intrinsics();
		return globals.search(nm);
		}
	static Uc_symbol *search_globals(const char *nm)
		{ 
		return globals.search(nm);
		}
	static Uc_intrinsic_symbol *get_intrinsic(int i)
		{ 
		setup_intrinsics();
		return (i >= 0 && i < intrinsics.size())? intrinsics[i] : 0;
		}
	static Uc_intrinsic_symbol *get_add_answer()
		{ return get_intrinsic(add_answer); }
	static Uc_intrinsic_symbol *get_remove_answer()
		{ return get_intrinsic(remove_answer); }
	static Uc_intrinsic_symbol *get_push_answers()
		{ return get_intrinsic(push_answers); }
	static Uc_intrinsic_symbol *get_pop_answers()
		{ return get_intrinsic(pop_answers); }
	static Uc_intrinsic_symbol *get_show_face()
		{ return get_intrinsic(show_face); }
	static Uc_intrinsic_symbol *get_remove_face()
		{ return get_intrinsic(remove_face); }
	static Uc_intrinsic_symbol *get_get_usecode_fun()
		{ return get_intrinsic(get_usecode_fun); }
	static Uc_intrinsic_symbol *get_get_item_shape()
		{ return get_intrinsic(get_item_shape); }
	Uc_var_symbol *add_symbol(char *nm);// Add var. to current scope.
	Uc_var_symbol *add_symbol(char *nm, Uc_class *c);// Add var. to current scope.
	Uc_var_symbol *add_symbol(Uc_var_symbol *var);// Add var. to current scope.
	void add_static(char *nm);	// Add static var. to current scope.
	void add_static(char *nm, Uc_class *c);	// Add static cls. to current scope.
	int add_function_symbol(Uc_function_symbol *fun, Uc_scope *parent=0)
		{ return cur_scope->add_function_symbol(fun, parent); }
	static int add_global_function_symbol(Uc_function_symbol *fun,
			Uc_scope *parent=0)
		{ return globals.add_function_symbol(fun, parent); }
	static void add_global_class_symbol(Uc_class_symbol *c)
		{ globals.add(c); }
					// Add string constant.
	Uc_symbol *add_string_symbol(char *nm, char *text);
					// Add int constant.
	Uc_symbol *add_int_const_symbol(char *nm, int value);
	static Uc_symbol *add_global_int_const_symbol(char *nm, int val);
	static void add_global_static(char *nm);
	static void add_global_static(char *nm, Uc_class *c);
	int add_string(char *text);
	int find_string_prefix(Uc_location& loc, const char *text);
					// Start/end loop.
	void add_label(Uc_label* l) { labels[l->get_name()] = l; }
	Uc_label *search_label(char *nm);

	void start_breakable(Uc_statement *s);
	void end_breakable(Uc_statement *s, vector<char>& stmt_code,
			int testlen = 0, bool dowhile = false);
					// Store 'break' location.
	void add_break(int op_offset);	// DANGER:  Offset is filled in when
					//   end_breakable() is called, so the
					//   string this is in better not have
					//   been deleted!!!
					// Store 'break' location.
	void add_continue(int op_offset);	// See the notes for add_break.
					// Link external function.
	int link(Uc_function_symbol *fun);
	void link_labels(std::vector<char>& code);
	void gen(std::ostream& out);		// Generate Usecode.
	virtual Usecode_symbol *create_sym();
	};

#endif
