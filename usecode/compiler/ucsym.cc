/**
 **	Ucsym.cc - Usecode compiler symbol table.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <stdio.h>
#include "ucsym.h"
#include "opcodes.h"
#include "utils.h"
#include "ucexpr.h"
#include "ucfun.h"
#include "ucclass.h"

using std::strcmp;

int Uc_function_symbol::last_num = -1;
Uc_function_symbol::Sym_nums Uc_function_symbol::nums_used;
bool Uc_function_symbol::new_auto_num = false;

/*
 *	Assign value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_symbol::gen_assign
	(
	vector<char>& out
	)
	{
	return 0;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_symbol::gen_value
	(
	vector<char>& out
	)
	{
	return 0;
	}

/*
 *	Generate function call.
 *
 *	Output: 0 if can't do this.
 */

int Uc_symbol::gen_call
	(
	vector<char>& out,
	Uc_function *fun,
	bool orig,			// Call original (not one from patch).
	Uc_expression *itemref,		// Non-NULL for CALLE.
	Uc_array_expression *parms,	// Parameter list.
	bool retvalue,			// True if a function.
	Uc_class *scope_vtbl	// For method calls using a different scope.
	)
	{
	return 0;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_symbol::create_expression
	(
	)
	{
	return 0;
	}

/*
 *	Assign value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_var_symbol::gen_assign
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_POP);
	Write2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_var_symbol::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSH);
	Write2(out, offset);
	return 1;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_var_symbol::create_expression
	(
	)
	{
	return new Uc_var_expression(this);
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_class_inst_symbol::create_expression
	(
	)
	{
	return new Uc_class_expression(this);
	}

/*
 *	Assign value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_static_var_symbol::gen_assign
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_POPSTATIC);
	Write2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_static_var_symbol::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHSTATIC);
	Write2(out, offset);
	return 1;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_static_class_symbol::create_expression
	(
	)
	{
	return new Uc_class_expression(this);
	}

/*
 *	Create new class symbol and store in global table.
 */

Uc_class_symbol *Uc_class_symbol::create
	(
	char *nm, 
	Uc_class *c
	)
	{
	Uc_symbol *sym = Uc_function::search_globals(nm);
	if (sym)
		{
		char buf[256];
		sprintf(buf, "Class name '%s' already exists.", nm);
		Uc_location::yyerror(buf);
		}
	Uc_class_symbol *csym = new Uc_class_symbol(nm, c);
	Uc_function::add_global_class_symbol(csym);
	return csym;
	}

/*
 *	Assign value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_class_var_symbol::gen_assign
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_POPCLSVAR);
	Write2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_class_var_symbol::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHCLSVAR);
	Write2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_const_int_symbol::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHI);
	Write2(out, value);
	return 1;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_const_int_symbol::create_expression
	(
	)
	{
	return new Uc_int_expression(value);
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_string_symbol::gen_value
	(
	vector<char>& out
	)
	{
	out.push_back((char) UC_PUSHS);
	Write2(out, offset);
	return 1;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_string_symbol::create_expression
	(
	)
	{
	return new Uc_string_expression(offset);
	}

/*
 *	Generate function call.
 *
 *	Output: 0 if can't do this.
 */

int Uc_intrinsic_symbol::gen_call
	(
	vector<char>& out,
	Uc_function *fun,
	bool orig,			// Call original (not one from patch).
	Uc_expression *itemref,		// Non-NULL for CALLE.
	Uc_array_expression *parms,	// Parameter list.
	bool retvalue,			// True if a function.
	Uc_class *scope_vtbl	// For method calls using a different scope.
	)
	{
	int parmcnt = parms->gen_values(out);	// Want to push parm. values.
	if (itemref)			// Happens with 'method' call.
		{
		itemref->gen_value(out);
		parmcnt++;
		}
					// ++++ parmcnt == num_parms.
					// Opcode depends on val. returned.
	out.push_back((char) (retvalue ? UC_CALLIS : UC_CALLI));
	Write2(out, intrinsic_num);	// Intrinsic # is 2 bytes.
	out.push_back((char) parmcnt);	// Parm. count is 1.
	return 1;
	}

/*
 *	Create new function.
 */

Uc_function_symbol::Uc_function_symbol
	(
	char *nm, 
	int num, 			// Function #, or -1 to assign
					//  1 + last_num.
	std::vector<Uc_var_symbol *>& p,
	int shp
	) :  Uc_symbol(nm), parms(p), usecode_num(num), method_num(-1),
	     ret_type(0), has_ret(false), shape_num(shp)
	{
	}

/*
 *	Create new function symbol or return existing one (which could
 *	have been declared EXTERN).
 */

Uc_function_symbol *Uc_function_symbol::create
	(
	char *nm, 
	int num, 			// Function #, or -1 to assign
					//  1 + last_num.
	std::vector<Uc_var_symbol *>& p,
	bool is_extern,
	Uc_scope *scope,
	int shp
	)
	{
	if (shp >= 0x400)
		// Just for safety.
		num = -1;
	else if (shp != -1)
		num = shp;
		
	// Override function number if the function has been declared before this.
	Uc_function_symbol *sym = (Uc_function_symbol *) (scope ?
		scope->search(nm) : Uc_function::search_globals(nm));
	if (sym)
		if (scope)
			{
			if (!sym->is_inherited())
				{
				char buf[256];
				sprintf(buf, "Duplicate declaration of function '%s'.", nm);
				Uc_location::yyerror(buf);
				}
			}
		else if (sym->is_externed() || is_extern)
			if (sym->get_num_parms() == p.size())
				num = sym->get_usecode_num();
			else
				num = -1;
		else
			{
			char buf[256];
			sprintf(buf, "Duplicate declaration of function '%s'.", nm);
			Uc_location::yyerror(buf);
			}
	int ucnum = num >= 0 ? num : (last_num + 1);
			// Set last_num if the function doesn't
			// have a number:
	if (num < 0 || 
			// Or if we are using old-style autonumbers:
		(num >= 0 && !new_auto_num))
		last_num = ucnum;
					// Keep track of #'s used.
	Sym_nums::const_iterator it = nums_used.find(ucnum);
	if (it == nums_used.end())	// Unused?  That's good.
		{
		sym = new Uc_function_symbol(nm, ucnum, p, shp);
		if (is_extern)
			sym->set_externed();
		nums_used[ucnum] = sym;
		return sym;
		}
	sym = (*it).second;
	if (sym->name != nm || sym->get_num_parms() != p.size())
		{
		char buf[256];
		sprintf(buf, 
			"Function 0x%x already used for '%s' with %d params.",
			ucnum, sym->get_name(), sym->get_num_parms());
		Uc_location::yyerror(buf);
		}
	return sym;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_function_symbol::create_expression
	(
	)
	{
	return new Uc_fun_name_expression(this);
	}

/*
 *	Generate function call.
 *
 *	Output: 0 if can't do this.
 */

int Uc_function_symbol::gen_call
	(
	vector<char>& out,
	Uc_function *fun,
	bool orig,			// Call original (not one from patch).
	Uc_expression *itemref,		// Non-NULL for CALLE or method.
	Uc_array_expression *aparms,	// Actual parameter list.
	bool retvalue,		// True if a function.
	Uc_class *scope_vtbl	// For method calls using a different scope.
	)
	{
	char buf[200];
	int parmcnt = aparms->gen_values(out);	// Want to push parm. values.
	parmcnt += (method_num >= 0);		// Count 'this'.
	if (parmcnt != parms.size())
		{
		sprintf(buf,
			"# parms. passed (%d) doesn't match '%s' count (%d)",
			parmcnt, get_name(), parms.size());
		Uc_location::yyerror(buf);
		}				
	if (orig)
		{
		if (!itemref)
			{
			Uc_item_expression item;
			item.gen_value(out);
			}
		else
			itemref->gen_value(out);
		out.push_back((char) UC_CALLO);
		Write2(out, usecode_num);	// Use fun# directly.
		}
	else if (method_num >= 0)		// Class method?
		{
		// If no explicit obj., find 'this'.
		if (!itemref && fun->get_method_num() >= 0)
			{
			Uc_symbol *tsym = fun->search("this");
			if (tsym && dynamic_cast<Uc_var_symbol *>(tsym))
				itemref = tsym->create_expression();
			}
		if (!itemref)
			{
			sprintf(buf,
			"Class method '%s' requires a 'this'.", get_name());
			Uc_location::yyerror(buf);
			}
		else
			itemref->gen_value(out);
		if (scope_vtbl)
			{
			out.push_back((char) UC_CALLMS);
			Write2(out, method_num);
			Write2(out, scope_vtbl->get_num());
			}
		else
			{
			out.push_back((char) UC_CALLM);
			Write2(out, method_num);
			}
		}
	else if (itemref)	// Doing CALLE?  Push item onto stack.
		{
		itemref->gen_value(out);
		out.push_back((char) UC_CALLE);
		Write2(out, usecode_num);	// Use fun# directly.
		}
	else				// Normal CALL.
		{			// Called function sets return.
		out.push_back((char) UC_CALL);
					// Get offset in function's list.
		int link = fun->link(this);
		Write2(out, link);
		}
	return 1;
	}


bool String_compare::operator()(char * const &x, char * const &y) const
	{ return strcmp(x, y) < 0; }

/*
 *	Delete.
 */

Uc_scope::~Uc_scope
	(
	)
	{
	for (std::map<char *, Uc_symbol *, String_compare>::iterator it = symbols.begin();
				it != symbols.end(); it++)
		delete (*it).second;
	for (std::vector<Uc_scope *>::iterator it = scopes.begin();
				it != scopes.end(); it++)
		delete *it;
	}

/*
 *	Search upwards through scope.
 *
 *	Output:	->symbol if found, else 0.
 */

Uc_symbol *Uc_scope::search_up
	(
	const char *nm
	)
	{
	Uc_symbol *found = search(nm);	// First look here.
	if (found)
		return found;
	if (parent)			// Look upwards.
		return parent->search_up(nm);
	else
		return 0;
	}

/*
 *	Add a function symbol.
 *
 *	Output:	0 if already there.  Errors reported.
 */

int Uc_scope::add_function_symbol
	(
	Uc_function_symbol *fun,
	Uc_scope *parent
	)
	{
	char buf[150];
	const char *nm = fun->get_name();
	Uc_symbol *found;	// Already here?
	if (parent)
		found = parent->search(nm);
	else
		found = search(nm);
	if (!found)			// If not, that's good.
		{
		if (parent)
			parent->add(fun);
		else
			add(fun);
		return 1;
		}
	Uc_function_symbol *fun2 = dynamic_cast<Uc_function_symbol *> (found);
	if (fun2 == fun)		// The case for an EXTERN.
		return 1;
	else if (!fun2)			// Non-function name.
		{
		sprintf(buf, "'%s' already declared", nm);
		Uc_location::yyerror(buf);
		}
	else if (fun->get_num_parms() != fun2->get_num_parms())
		{
		if (fun2->is_inherited())
			sprintf(buf, "Decl. of virtual member function '%s' doesn't match decl. from base class", nm);
		else
			sprintf(buf, "Decl. of '%s' doesn't match previous decl", nm);
		Uc_location::yyerror(buf);
		}
	else if (fun->usecode_num != fun2->usecode_num)
		{
		if (fun2->externed || fun->externed || fun2->is_inherited())
			{
			if (!Uc_function_symbol::new_auto_num &&
					Uc_function_symbol::last_num == fun->usecode_num)
				--Uc_function_symbol::last_num;
			}
		else
			{
			sprintf(buf, "Decl. of '%s' has different usecode #.",
									nm);
			Uc_location::yyerror(buf);
			}
		}			
	return 0;
	}

/*
 *	Check for a duplicate symbol and print an error.
 *
 *	Output:	true if dup., with error printed.
 */

bool Uc_scope::is_dup
	(
	char *nm
	)
	{
	Uc_symbol *sym = search(nm);
	if (sym)			// Already in scope?
		{
		char msg[180];
		sprintf(msg, "Symbol '%s' already declared", nm);
		Uc_location::yyerror(msg);
		return true;
		}
	return false;
	}


