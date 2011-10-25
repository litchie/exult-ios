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


#include <cstring>
#include <stdio.h>
#include "ucsym.h"
#include "opcodes.h"
#include "utils.h"
#include "ucexpr.h"
#include "ucfun.h"
#include "ucclass.h"
#include "basic_block.h"

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
	Basic_block *out
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
	Basic_block *out
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
	Basic_block *out,
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
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_POP);
	WriteOpParam2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_var_symbol::gen_value
	(
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_PUSH);
	WriteOpParam2(out, offset);
	return 1;
	}

int Uc_var_symbol::is_object_function(bool print_error) const
	{
	if (print_error)
		{
		char buf[180];
		switch (is_obj_fun)
			{
			case -2:
				sprintf(buf, "Shape # is equal to fun. ID only for shapes < 0x400; use UI_get_usecode_fun instead");
				Uc_location::yywarning(buf);
				break;
			case 1:
				sprintf(buf, "Var '%s' contains fun. not declared as 'shape#' or 'object#'",
						name.c_str());
				Uc_location::yyerror(buf);
				break;
			case 2:
				sprintf(buf, "Var '%s' contains a negative number", name.c_str());
				Uc_location::yyerror(buf);
				break;
			case 3:
				sprintf(buf, "Return of intrinsics are generally not fun. IDs");
				Uc_location::yyerror(buf);
				break;
			}
		}
	return is_obj_fun;
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
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_POPSTATIC);
	WriteOpParam2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_static_var_symbol::gen_value
	(
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_PUSHSTATIC);
	WriteOpParam2(out, offset);
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
 *	Check for a duplicate symbol and print an error.
 *
 *	Output:	true if dup., with error printed.
 */

bool Uc_struct_symbol::is_dup
	(
	char *nm
	)
	{
	int index = search(nm);
	if (index >= 0)			// Already declared?
		{
		char msg[180];
		sprintf(msg, "Symbol '%s' already declared", nm);
		Uc_location::yyerror(msg);
		return true;
		}
	return false;
	}

Uc_struct_symbol::~Uc_struct_symbol()
	{
	for (Var_map::iterator it = vars.begin(); it != vars.end(); ++it)
		delete it->first;
	vars.clear();
	}

void Uc_struct_symbol::merge_struct(Uc_struct_symbol *other)
	{
	for (Var_map::iterator it = other->vars.begin();
			it != other->vars.end(); ++it)
		add(it->first);
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
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_POPTHV);
	WriteOpParam2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_class_var_symbol::gen_value
	(
	Basic_block *out
	)
	{
	WriteOp(out, (char) UC_PUSHTHV);
	WriteOpParam2(out, offset);
	return 1;
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_const_int_symbol::gen_value
	(
	Basic_block *out
	)
	{
	WriteOp(out, (char) opcode);
	if (opcode == UC_PUSHB)
		WriteOpParam1(out, value);
	else if (opcode == UC_PUSHI)
		WriteOpParam2(out, value);
	else
		WriteOpParam4(out, value);
	return 1;
	}

/*
 *	Create an expression with this value.
 */

Uc_expression *Uc_const_int_symbol::create_expression
	(
	)
	{
	return new Uc_int_expression(value, opcode);
	}

/*
 *	Generate code to push variable's value on stack.
 *
 *	Output: 0 if can't do this.
 */

int Uc_string_symbol::gen_value
	(
	Basic_block *out
	)
	{
	if (is_int_32bit(offset))
		{
		WriteOp(out, (char) UC_PUSHS32);
		WriteOpParam4(out, offset);
		}
	else
		{
		WriteOp(out, (char) UC_PUSHS);
		WriteOpParam2(out, offset);
		}
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
	Basic_block *out,
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
	WriteOp(out, (char) (retvalue ? UC_CALLIS : UC_CALLI));
	WriteOpParam2(out, intrinsic_num);	// Intrinsic # is 2 bytes.
	WriteOpParam1(out, (char) parmcnt);	// Parm. count is 1.
	return 1;
	}

/*
 *	Create new function.
 */

Uc_function_symbol::Uc_function_symbol
	(
	const char *nm, 
	int num, 			// Function #, or -1 to assign
					//  1 + last_num.
	std::vector<Uc_var_symbol *>& p,
	int shp,
	Function_kind kind
	) :	Uc_symbol(nm), parms(p), usecode_num(num), method_num(-1),
		shape_num(shp), externed(false), inherited(false),
		ret_type(no_ret), high_id(false), type(kind)
	{
	high_id = is_int_32bit(usecode_num);
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
	Function_kind kind
	)
	{
	// Checking num and kind for backward compatibility.
	if (num < 0)
		{
		// Treat as autonumber request.
		num = -1;
		if (kind == shape_fun)
			{
			char buf[180];
			sprintf(buf, "Shape number cannot be negative");
			Uc_location::yyerror(buf);
			}
		}
	else if (num < 0x400)
		{
		if (kind == utility_fun)
			{
			char buf[180];
			sprintf(buf, "Treating function '%s' as being a 'shape#()' function.", nm);
			Uc_location::yywarning(buf);
			kind = shape_fun;
			}
		}
	else if (num < 0x800)
		{
		if (kind == utility_fun)
			{
			char buf[180];
			sprintf(buf, "Treating function '%s' as being an 'object#()' function.", nm);
			Uc_location::yywarning(buf);
			kind = object_fun;
			}
		}

	int shp = (kind == shape_fun) ? num : -1;
	if (shp >= 0x400)
		num = new_auto_num ? -1 : 0xC00 + shp;
	else if (shp != -1)
		num = shp;
		
	// Override function number if the function has been declared before this.
	Uc_function_symbol *sym = (Uc_function_symbol *) (scope ?
		scope->search(nm) : Uc_function::search_globals(nm));
	if (sym)
		{
		if (sym->get_function_type() != kind)
			{
			std::string msg = "Incompatible declarations of function '";
			msg = msg + nm + "': ";
			switch (sym->get_function_type())
				{
				case utility_fun:
					msg += "new decl. should not use ";
					if (kind == shape_fun)
						msg += "'shape#'";
					else
						msg += "'object#'";
					break;
				case shape_fun:
					if (kind == utility_fun)
						msg += "'shape#' missing in new decl.";
					else
						msg += "new decl. should use 'shape#' instead of 'object#'";
					break;
				case object_fun:
					if (kind == utility_fun)
						msg += "'object#' missing in new decl.";
					else
						msg += "new decl. should use 'object#' instead of 'shape#'";
					break;
				}
			Uc_location::yyerror(const_cast<char *>(msg.c_str()));
			}
		else if (scope)
			{
			if (!sym->is_inherited())
				{
				char buf[256];
				sprintf(buf, "Duplicate declaration of function '%s'.", nm);
				Uc_location::yyerror(buf);
				}
			}
		else if (sym->is_externed() || is_extern)
			if ((size_t)sym->get_num_parms() == p.size())
			{
				// If the new symbol is not externed, then the function
				// has been defined afterwards and we need to update
				// sym to not be extern anymore.
				if (!is_extern)
					sym->clear_externed();
				num = sym->get_usecode_num();
			}
			else
				num = -1;
		else
			{
			char buf[256];
			sprintf(buf, "Duplicate declaration of function '%s'.", nm);
			Uc_location::yyerror(buf);
			}
		}

	if (num < 0 && !new_auto_num)
		{
		char buf[256];
		sprintf(buf,
			"Auto-numbering function '%s', but '#autonumber' directive not used.",
			nm);
		Uc_location::yywarning(buf);
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
		sym = new Uc_function_symbol(nm, ucnum, p, shp, kind);
		if (is_extern)
			sym->set_externed();
		nums_used[ucnum] = sym;
		return sym;
		}
	sym = (*it).second;
	if (sym->name != nm || (size_t)sym->get_num_parms() != p.size())
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
	Basic_block *out,
	Uc_function *fun,
	bool orig,			// Call original (not one from patch).
	Uc_expression *itemref,		// Non-NULL for CALLE or method.
	Uc_array_expression *aparms,	// Actual parameter list.
	bool retvalue,		// True if a function.
	Uc_class *scope_vtbl	// For method calls using a different scope.
	)
	{
	char buf[200];
	size_t parmcnt = aparms->gen_values(out);	// Want to push parm. values.
	parmcnt += (method_num >= 0);		// Count 'this'.
	if (parmcnt != parms.size())
		{
		sprintf(buf,
			"# parms. passed (%lu) doesn't match '%s' count (%lu)",
			parmcnt, get_name(), parms.size());
		Uc_location::yyerror(buf);
		}
	// See if expecting a return value from a function that has none.
	if (retvalue && !has_ret())
		{
		sprintf(buf,
			"Function '%s' does not have a return value",
			get_name());
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
		WriteOp(out, (char) UC_CALLO);
		WriteOpParam2(out, usecode_num);	// Use fun# directly.
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
			WriteOp(out, (char) UC_CALLMS);
			WriteOpParam2(out, method_num);
			WriteOpParam2(out, scope_vtbl->get_num());
			}
		else
			{
			WriteOp(out, (char) UC_CALLM);
			WriteOpParam2(out, method_num);
			}
		}
	else if (high_id)
		{
		if (itemref)
			{
			itemref->gen_value(out);
			WriteOp(out, (char) UC_CALLE32);
			}
		else
			WriteOp(out, (char) UC_CALL32);
		WriteOpParam4(out, usecode_num);	// Use fun# directly.
		}
	else if (itemref)	// Doing CALLE?  Push item onto stack.
		{
		// The originals would need this.
		fun->link(this);
		itemref->gen_value(out);
		WriteOp(out, (char) UC_CALLE);
		WriteOpParam2(out, usecode_num);	// Use fun# directly.
		}
	else				// Normal CALL.
		{			// Called function sets return.
		// Add to externs list.
		int link = fun->link(this);
		WriteOp(out, (char) UC_CALL);
		WriteOpParam2(out, link);
		}
	if (!retvalue && has_ret())
		{
		// Function returns a value, but caller does not use it.
		// Generate the code to pop the result off the stack.
		static int cnt = 0;
		char buf[50];
		sprintf(buf, "_tmpretval_%d", cnt++);
						// Create a 'tmp' variable.
		Uc_var_symbol *var = fun->add_symbol(buf);
		if (!var)
			return 0;		// Shouldn't happen.  Err. reported.
		var->gen_assign(out);
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
				it != symbols.end(); ++it)
		delete (*it).second;
	for (std::vector<Uc_scope *>::iterator it = scopes.begin();
				it != scopes.end(); ++it)
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


