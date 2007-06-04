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
class Usecode_symbol;
class Uc_class;
class Uc_scope;

#include <iostream>
inline bool is_int_32bit(int val)
	{
	int high = val >> 16;
	return !(high == -1 || high == 0);
	}

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
	virtual int gen_call(vector<char>& out, Uc_function *fun, bool orig,
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue, Uc_class *scope_vtbl = 0);
	virtual int get_string_offset()	// Get offset in text_data.
		{ return -1; }
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	virtual Uc_class *get_cls() const
		{ return 0; }
	virtual bool is_static() const
		{ return false; }
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
	void set_offset(int off)
		{ offset = off; }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual int gen_assign(vector<char>& out);
					// Return var/int expression.
	virtual Uc_expression *create_expression();
	};

/*
 *	A class instance, a typed variable which can be assigned to.
 */
class Uc_class_inst_symbol : public Uc_var_symbol
	{
protected:
	Uc_class *cls;
public:
	Uc_class_inst_symbol(char *nm, Uc_class *c, int off)
		: Uc_var_symbol(nm, off), cls(c)
		{  }
	virtual Uc_expression *create_expression();
	virtual Uc_class *get_cls() const
		{ return cls; }
	};

/*
 *	A static (persistent) variable.
 */
class Uc_static_var_symbol : public Uc_var_symbol
	{
public:
	Uc_static_var_symbol(char *nm, int off) : Uc_var_symbol(nm, off)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual int gen_assign(vector<char>& out);
	virtual bool is_static() const
		{ return true; }
	};

/*
 *	A static (persistent) class.
 */
class Uc_static_class_symbol : public Uc_static_var_symbol
	{
protected:
	Uc_class *cls;
public:
	Uc_static_class_symbol(char *nm, Uc_class *c, int off)
		: Uc_static_var_symbol(nm, off), cls(c)
		{  }
	virtual Uc_expression *create_expression();
	virtual Uc_class *get_cls() const
		{ return cls; }
	virtual bool is_static() const
		{ return true; }
	};

/*
 *	A class.
 */
class Uc_class_symbol : public Uc_symbol
	{
	Uc_class *cls;
public:
	Uc_class_symbol(char *nm, Uc_class *c) : Uc_symbol(nm), cls(c)
		{  }
	static Uc_class_symbol *create(char *nm, Uc_class *c);
	Uc_class *get_cls()
		{ return cls; }
	};

/*
 *	A class member variable.
 */
class Uc_class_var_symbol : public Uc_var_symbol
	{
public:
	Uc_class_var_symbol(char *nm, int off) : Uc_var_symbol(nm, off)
		{  }
					// Gen. code to put result on stack.
	virtual int gen_value(vector<char>& out);
					// Gen. to assign from stack.
	virtual int gen_assign(vector<char>& out);
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
	int get_value() const
		{ return value; }
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
	virtual int gen_call(vector<char>& out, Uc_function *fun, bool orig,
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue, Uc_class *scope_vtbl = 0);
	};

/*
 *	A function-prototype symbol:
 */
class Uc_function_symbol : public Uc_symbol
	{
	static int last_num;		// Last 'usecode_num', so we can
					//   assign automatically.
	static bool new_auto_num;		// Prevents autoassigning of
					//   function numbers if function already
					//   has a number of its own.
public:
					// Keep track of #'s used.
	typedef std::map<int, Uc_function_symbol *> Sym_nums;
private:
	static Sym_nums nums_used;
					// Note:  offset = Usecode fun. #.
	std::vector<Uc_var_symbol *> parms;	// Parameters.
	int usecode_num;		// Usecode function #.
	int method_num;			// Index with class if a method.
	int shape_num;			// Shape # this function is for.
	bool externed;
	bool inherited;
	bool has_ret;
	Uc_class *ret_type;
	bool high_id;
public:
	friend class Uc_scope;
	Uc_function_symbol(char *nm, int num,
				std::vector<Uc_var_symbol *>& p, int shp = -1);
	static Uc_function_symbol *create(char *nm, int num, 
				std::vector<Uc_var_symbol *>& p, bool is_extern=false,
				Uc_scope *scope = 0, int shp = -1);
	const std::vector<Uc_var_symbol *>& get_parms()
		{ return parms; }
	int get_usecode_num()
		{ return usecode_num; }
	const bool has_high_id()
		{ return high_id; }
	void set_method_num(int n)
		{ method_num = n; }
	int get_method_num()
		{ return method_num; }
	int get_shape_num()
		{ return shape_num; }
	int get_num_parms()
		{ return parms.size(); }
	void set_externed()
		{ externed = true; }
	bool is_externed()
		{ return externed; }
	void set_inherited()
		{ inherited = true; }
	bool is_inherited()
		{ return inherited; }
					// Return var/int expression.
	virtual Uc_expression *create_expression();
					// Generate function/procedure call.
	virtual int gen_call(vector<char>& out, Uc_function *fun, bool orig,
		Uc_expression *item, Uc_array_expression *parms, 
							bool retvalue, Uc_class *scope_vtbl = 0);
	static void set_last_num(int n)
		{
		last_num = n;
		new_auto_num = true;
		}
	virtual bool get_has_ret() const
		{ return has_ret; }
	virtual void set_has_ret()
		{ has_ret = true; }
	virtual Uc_class *get_cls() const
		{ return ret_type; }
	virtual bool set_ret_type(Uc_class *r)
		{ ret_type = r; has_ret = true; }
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
	Uc_symbol *search_up(const char *nm);
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
	int add_function_symbol(Uc_function_symbol *fun, Uc_scope *parent=0);
	bool is_dup(char *nm);		// Already declared?
	};

/*
 *	Abstract top-level code (functions, classes). 
 */
class Uc_design_unit
	{
public:
	virtual void gen(std::ostream& out) = 0;	// Generate Usecode.
	virtual Usecode_symbol *create_sym() = 0;
	virtual bool is_class() const
		{ return false; }
	};

#endif


