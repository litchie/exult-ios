%{
/**
 **	Ucparse.y - Usecode parser.
 **
 **	Written: 12/30/2000 - JSF
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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

#include "ucfun.h"
#include "ucclass.h"
#include "ucexpr.h"
#include "ucstmt.h"
#include "opcodes.h"
#include "ucscriptop.h"

using std::strcpy;
using std::strcat;
using std::strlen;
using std::string;

void yyerror(char *);
extern int yylex();
extern void start_script(), end_script();
static Uc_array_expression *Create_array(int, Uc_expression *);
static Uc_array_expression *Create_array(int, Uc_expression *, 
							Uc_expression *);
static Uc_class *Find_class(const char *nm);
static bool Class_unexpected_error(Uc_expression *expr);
static bool Nonclass_unexpected_error(Uc_expression *expr);
static bool Incompatible_classes_error(Uc_class *src, Uc_class *trg);
static Uc_call_expression *cls_method_call(Uc_expression *cls, Uc_class *curcls,
		Uc_class *clsscope, char *nm, Uc_array_expression *parms);
static Uc_call_expression *cls_function_call(Uc_expression *ths,
		Uc_class *curcls, char *nm, bool original, Uc_array_expression *parms);


#define YYERROR_VERBOSE 1

std::vector<Uc_design_unit *> units;	// THIS is what we produce.

static Uc_function *cur_fun = 0;	// Current function being parsed.
static Uc_class *cur_class = 0;		// ...or, current class being parsed.
static int enum_val = -1;		// Keeps track of enum elements.
static bool is_extern = false;	// Marks a function symbol as being an extern
static Uc_class *class_type = 0;	// For declaration of class variables.
static bool has_ret = false;

%}

%union
	{
	class Uc_symbol *sym;
	class Uc_var_symbol *var;
	class Uc_class *cls;
	class Uc_expression *expr;
	class Uc_call_expression *funcall;
	class Uc_function_symbol *funsym;
	class Uc_statement *stmt;
	class std::vector<Uc_var_symbol *> *varvec;
	class Uc_block_statement *block;
	class Uc_arrayloop_statement *arrayloop;
	class Uc_array_expression *exprlist;
	int intval;
	char *strval;
	}

/*
 *	Keywords:
 */
%token IF ELSE RETURN WHILE FOR UCC_IN WITH TO EXTERN BREAK GOTO CASE
%token VAR UCC_INT UCC_CONST STRING ENUM
%token CONVERSE SAY MESSAGE RESPONSE EVENT FLAG ITEM UCTRUE UCFALSE REMOVE
%token ADD HIDE SCRIPT AFTER TICKS STATIC_ ORIGINAL SHAPENUM ABORT CLASS
%token NEW DELETE

/*
 *	Script keywords:
 */
					/* Script commands. */
%token CONTINUE REPEAT NOP NOHALT WAIT REMOVE RISE DESCEND FRAME HATCH
%token NEXT PREVIOUS CYCLE STEP MUSIC CALL SPEECH SFX FACE HIT HOURS ACTOR
%token ATTACK FINISH RESURRECT SETEGG
%token NORTH SOUTH EAST WEST NE NW SE SW

/*
 *	Other tokens:
 */
%token <strval> STRING_LITERAL STRING_PREFIX IDENTIFIER
%token <intval> INT_LITERAL

/*
 *	Handle if-then-else conflict.
 */
%left IF
%right ELSE

/*
 *	Expression precedence rules (lowest to highest):
 */
%left AND OR
%left EQUALS NEQUALS LTEQUALS GTEQUALS '<' '>' UCC_IN
%left '-' '+' '&'
%left '*' '/' '%'
%left NOT
%left UCC_POINTS UCC_SCOPE

/*
 *	Production types:
 */
%type <expr> expression primary declared_var_value opt_script_delay item
%type <expr> script_command start_call addressof new_expr class_expr
%type <expr> nonclass_expr
%type <intval> opt_int eventid direction int_literal converse_options
%type <intval> opt_original opt_var opt_shapenum
%type <sym> declared_sym
%type <var> declared_var param
%type <cls> opt_inheritance defined_class
%type <funsym> function_proto function_decl
%type <varvec> param_list opt_param_list
%type <stmt> statement assignment_statement if_statement while_statement
%type <stmt> statement_block return_statement function_call_statement
%type <stmt> special_method_call_statement
%type <stmt> array_loop_statement var_decl var_decl_list stmt_declaration
%type <stmt> class_decl class_decl_list
%type <stmt> break_statement converse_statement converse2_statement
%type <stmt> converse_case script_statement
%type <stmt> label_statement goto_statement answer_statement
%type <stmt> delete_statement
%type <block> statement_list converse_case_list
%type <arrayloop> start_array_loop
%type <exprlist> opt_expression_list expression_list script_command_list
%type <exprlist> opt_nonclass_expr_list nonclass_expr_list
%type <funcall> function_call

%%

design:
	design global_decl
	| global_decl
	;

global_decl:
	function
	| function_decl
		{
		if (!Uc_function::add_global_function_symbol($1))
			delete $1;
		}
	| const_int_decl
	| enum_decl
	| static_decl
	| class_definition
	;

class_definition:
	CLASS IDENTIFIER opt_inheritance 
		{ $3 ? cur_class = new Uc_class($2, $3)
		     : cur_class = new Uc_class($2); }
		'{' class_item_list '}'
		{
		units.push_back(cur_class);
		// Add to 'globals' symbol table.
		(void) Uc_class_symbol::create($2, cur_class);
		cur_class = 0;
		}
	;

opt_inheritance:
	':' defined_class
		{ $$ = $2; }
	|				/* Empty */
		{ $$ = 0; }
	;

class_item_list:
	class_item_list class_item
	|				/* Empty */
	;

class_item:
	VAR { has_ret = true; } class_var_def
		{ has_ret = false; }
	| CLASS '<' defined_class '>' { class_type = $3; } method
		{ class_type = 0; }
	| method
	;

class_var_def:
	var_decl_list ';'
		{ ; }
	| method
	;

method:
	IDENTIFIER '(' opt_param_list ')'
		{
		$3->insert($3->begin(),		// So it's local[0].
			new Uc_class_inst_symbol("this", cur_class, 0));
		Uc_function_symbol *funsym = 
			Uc_function_symbol::create($1, -1, *$3, false,
					cur_class->get_scope());
		delete $3;		// A copy was made.
		
		// Set return type.
		if (has_ret)
			funsym->set_has_ret();
		else if (class_type)
			funsym->set_ret_type(class_type);
		has_ret = class_type = 0;

		cur_fun = new Uc_function(funsym, cur_class->get_scope());
		}
	function_body
		{
		cur_class->add_method(cur_fun);
		cur_fun = 0;
		}
	;

function:
	function_proto { cur_fun = new Uc_function($1); }
	function_body
		{
		units.push_back(cur_fun);
		cur_fun = 0;
		}
	;

function_body:
		statement_block
		{ 
		cur_fun->set_statement($1);
		}
	;

					/* Opt_int assigns function #. */
function_proto:
	opt_var IDENTIFIER opt_int opt_shapenum '(' opt_param_list ')'
		{
		if ($3 != -1 && $4 != -1)
			{
			char buf[180];
			sprintf(buf, "parse error: 'shape#(%d)' unexpected", $4);
			yyerror(buf);
			}
		$$ = Uc_function_symbol::create($2, $3, *$6, is_extern, 0, $4);
		if ($1)
			$$->set_has_ret();
		delete $6;		// A copy was made.
		}
	| CLASS '<' defined_class '>' IDENTIFIER opt_int opt_shapenum '(' opt_param_list ')'
		{
		if ($6 > -1 && $7 > -1)
			{
			char buf[180];
			sprintf(buf, "parse error: 'shape#(%d)' unexpected", $7);
			yyerror(buf);
			}
		$$ = Uc_function_symbol::create($5, $6, *$9, is_extern, 0, $7);
		$$->set_ret_type($3);
		delete $9;		// A copy was made.
		}
	;

opt_shapenum:
	SHAPENUM '(' INT_LITERAL ')'
		{ $$ = $3; }
	|				/* Empty. */
		{ $$ = -1; }
	;

opt_int:
	int_literal
	|				/* Empty. */
		{ $$ = -1; }
	;

statement_block:
	'{' 
		{ cur_fun->push_scope(); }
	statement_list '}'
		{
		$$ = $3;
		cur_fun->pop_scope();
		}
	;

statement_list:
	statement_list statement
		{
		if ($2)
			$$->add($2); 
		}
	|				/* Empty. */
		{ $$ = new Uc_block_statement(); }
	;

statement:
	stmt_declaration
	| assignment_statement
	| if_statement
	| while_statement
	| array_loop_statement
	| function_call_statement
	| special_method_call_statement
	| return_statement
	| statement_block
	| converse_statement
	| converse2_statement
	| script_statement
	| break_statement
	| label_statement
	| goto_statement
	| delete_statement
	| SAY  '(' opt_nonclass_expr_list ')' ';'
		{ $$ = new Uc_say_statement($3); }
	| MESSAGE '(' opt_nonclass_expr_list ')' ';'
		{ $$ = new Uc_message_statement($3); }
	| answer_statement
	| ABORT ';'
		{ $$ = new Uc_opcode_statement(UC_ABRT); }
	| ';'				/* Null statement */
		{ $$ = 0; }
	;

stmt_declaration:
	VAR var_decl_list ';'
		{ $$ = $2; }
	| CLASS '<' defined_class '>' { class_type = $3; } class_decl_list ';'
		{ class_type = 0; $$ = $6; }
	| STRING string_decl_list ';'
		{ $$ = 0; }
	| const_int_decl
		{ $$ = 0; }
	| enum_decl
		{ $$ = 0; }
	| function_decl
		{
		if (!cur_fun->add_function_symbol($1, cur_class ?
				cur_class->get_scope() : 0))
			delete $1;
		$$ = 0;
		}
	| static_decl
		{ $$ = 0; }
	;

var_decl_list:
	var_decl_list ',' var_decl
		{
		if (!$3)
			$$ = $1;
		else if (!$1)
			$$ = $3;
		else			/* Both nonzero.  Need a list. */
			{
			Uc_block_statement *b = 
				dynamic_cast<Uc_block_statement *>($1);
			if (!b)
				{
				b = new Uc_block_statement();
				b->add($1);
				}
			b->add($3);
			$$ = b;
			}
		}
	| var_decl
		{ $$ = $1; }
	;

enum_decl:				/* Decls. the elems, not the enum. */
	ENUM IDENTIFIER { enum_val = -1; } '{' enum_item_list '}' ';'
	;

enum_item_list:
	enum_item_list ',' enum_item
	| enum_item
	;

enum_item:
	const_int
	| IDENTIFIER
		{			/* Increment last value.	*/
		++enum_val;
		if (cur_fun)
			cur_fun->add_int_const_symbol($1, enum_val);
		else			// Global.
			Uc_function::add_global_int_const_symbol($1, enum_val);
		}
	;

const_int_decl:
	UCC_CONST UCC_INT const_int_decl_list ';'
	;

const_int_decl_list:
	const_int_decl_list ',' const_int
	| const_int
	;

const_int:
	IDENTIFIER '=' nonclass_expr	
		{
		int val;		// Get constant.
		if ($3->eval_const(val))
			{
			if (cur_fun)
				cur_fun->add_int_const_symbol($1, val);
			else		// Global.
				Uc_function::add_global_int_const_symbol(
								$1, val);
			enum_val = val;	// In case we're in an enum.
			}
		}
	;

var_decl:
	IDENTIFIER
		{
		if (cur_fun)
			cur_fun->add_symbol($1);
		else
			cur_class->add_symbol($1);
		$$ = 0;
		}
	| IDENTIFIER '=' nonclass_expr
		{
		if (cur_class)
			{
			char buf[180];
			sprintf(buf, "Initialization of class member var '%s' must be done through constructor", $1);
			yyerror(buf);
			$$ = 0;
			}
		else
			{
			Uc_var_symbol *var = cur_fun ? cur_fun->add_symbol($1)
							 : cur_class->add_symbol($1);
			$$ = new Uc_assignment_statement(
					new Uc_var_expression(var), $3);
			}
		}
	;

class_decl_list:
	class_decl_list ',' class_decl
		{
		if (!$3)
			$$ = $1;
		else if (!$1)
			$$ = $3;
		else		/*	Both nonzero; need a list.	*/
			{
			Uc_block_statement *b = dynamic_cast<Uc_block_statement *>($1);
			if (!b)
				{
				b = new Uc_block_statement();
				b->add($1);
				}
			b->add($3);
			$$ = b;
			}
		}
	| class_decl
		{ $$ = $1; }

class_decl:
	IDENTIFIER
		{
		if (cur_fun)
			cur_fun->add_symbol($1, class_type);
		else
			// Unsupported for now
			;
		$$ = 0;
		}
	| IDENTIFIER '=' class_expr
		{
		if (Nonclass_unexpected_error($3))
			$$ = 0;
		else
			{
			Uc_class *src = $3->get_cls();
			if (Incompatible_classes_error(src, class_type))
				$$ = 0;
			else
				{
				Uc_var_symbol *v = cur_fun->add_symbol($1, class_type);
				$$ = new Uc_assignment_statement(new Uc_class_expression(v), $3);
				}
			}
		}
	;

class_expr:
	new_expr
		{ $$ = $1; }
	| IDENTIFIER
		{
		Uc_symbol *sym = cur_fun->search_up($1);
		if (!sym)
			{
			char buf[150];
			sprintf(buf, "'%s' not declared", $1);
			yyerror(buf);
			sym = cur_fun->add_symbol($1);
			}
		Uc_class_inst_symbol *c = dynamic_cast<Uc_class_inst_symbol *>(sym);
		if (!c)
			{
			char buf[150];
			sprintf(buf, "'%s' not a class", $1);
			yyerror(buf);
			$$ = 0;
			}
		else
			$$ = new Uc_class_expression(c);
		}
	;

static_decl:
	STATIC_ VAR static_var_decl_list ';'
	| STATIC_ CLASS '<' defined_class '>'
		{ class_type = $4; }
		static_cls_decl_list ';'
		{ class_type = 0; }
	;

static_var_decl_list:
	static_var
	| static_var_decl_list ',' static_var
	;

static_var:
	IDENTIFIER
		{
		if (cur_fun)
			cur_fun->add_static($1);
		else
			Uc_function::add_global_static($1);
		}
	;

static_cls_decl_list:
	static_cls
	| static_cls_decl_list ',' static_cls
	;

static_cls:
	IDENTIFIER
		{
		if (cur_fun)
			cur_fun->add_static($1, class_type);
		else
			Uc_function::add_global_static($1, class_type);
		}
	;

string_decl_list:
	string_decl_list ',' string_decl
	| string_decl
	;

string_decl:
	IDENTIFIER '=' STRING_LITERAL 
		{
		cur_fun->add_string_symbol($1, $3);
		}
	;

function_decl:
	EXTERN { is_extern = true; } function_proto ';'
		{ $$ = $3; is_extern = false; }
	;

assignment_statement:
	expression '=' expression ';'
		{
		// Some rudimentary type-checking for classes
		if ($1->is_class())
			{
			if (Nonclass_unexpected_error($3))
				$$ = 0;
			else
				{
				Uc_class *trg = $1->get_cls();
				Uc_class *src = $3->get_cls();
				if (Incompatible_classes_error(src, trg))
					$$ = 0;
				else
					$$ = new Uc_assignment_statement($1, $3);
				}
			}
		else if (Class_unexpected_error($3))
			$$ = 0;
		else
			$$ = new Uc_assignment_statement($1, $3);
		}
	;

if_statement:
	IF '(' expression ')' statement %prec IF
		{ $$ = new Uc_if_statement($3, $5, 0); }
	| IF '(' expression ')' statement ELSE statement
		{ $$ = new Uc_if_statement($3, $5, $7); }
	;

while_statement:
	WHILE '(' nonclass_expr ')' statement
		{ $$ = new Uc_while_statement($3, $5); }
	;

array_loop_statement:
	start_array_loop ')' statement
		{
		$1->set_statement($3);
		$1->finish(cur_fun);
		cur_fun->pop_scope();
		}
	| start_array_loop WITH IDENTIFIER 
		{ $1->set_index(cur_fun->add_symbol($3)); }
					')' statement
		{
		$1->set_statement($6);
		$1->finish(cur_fun);
		cur_fun->pop_scope();
		}
	| start_array_loop WITH IDENTIFIER 
		{ $1->set_index(cur_fun->add_symbol($3)); }
				TO IDENTIFIER 
		{ $1->set_array_size(cur_fun->add_symbol($6)); }
						')' statement
		{
		$1->set_statement($9);
		cur_fun->pop_scope();
		}
	;

start_array_loop:
	start_for IDENTIFIER UCC_IN declared_var
		{
		if ($4->get_cls())
			{
			char buf[150];
			sprintf(buf, "Can't convert class '%s' into non-class",
					$4->get_name());
			yyerror(buf);
			}
		Uc_var_symbol *var = cur_fun->add_symbol($2);
		$$ = new Uc_arrayloop_statement(var, $4);
		}
	;

start_for:
	FOR '('
		{ cur_fun->push_scope(); }
	;

function_call_statement:
	function_call ';'
		{ $$ = new Uc_call_statement($1);  }
	;

special_method_call_statement:
					/* Have 'primary' say something.*/
	primary hierarchy_tok SAY '(' opt_nonclass_expr_list ')' ';'
		{
		Uc_block_statement *stmts = new Uc_block_statement();
					/* Set up 'show' call.		*/
		stmts->add(new Uc_call_statement(
			new Uc_call_expression(Uc_function::get_show_face(),
			new Uc_array_expression($1, new Uc_int_expression(0)), 
								cur_fun)));
		stmts->add(new Uc_say_statement($5));
		$$ = stmts;
		}
	| primary hierarchy_tok HIDE '(' ')' ';'
		{
		$$ = new Uc_call_statement(
			new Uc_call_expression(Uc_function::get_remove_face(),
				new Uc_array_expression($1), cur_fun));
		}
	;

return_statement:
	RETURN '0' ';'
		{
		// Allow return of '0' in functions that return a class.
		if (!cur_fun->get_has_ret())
			{
			char buf[180];
			sprintf(buf, "Function '%s' can't return a value",
					cur_fun->get_name());
			yyerror(buf);
			$$ = 0;
			}
		else
			$$ = new Uc_return_statement(new Uc_int_expression(0));
		}
	| RETURN expression ';'
		{
		if (!cur_fun->get_has_ret())
			{
			char buf[180];
			sprintf(buf, "Function '%s' can't return a value",
					cur_fun->get_name());
			yyerror(buf);
			$$ = 0;
			}
		else
			{
			Uc_class *src = $2->get_cls();
			Uc_class *trg = cur_fun->get_cls();
			if (!src && !trg)
				$$ = new Uc_return_statement($2);
			else if (!src || !trg)
				{
				char buf[210];
				sprintf(buf, "Function '%s' expects a return of %s '%s' but supplied value is %s'%s'",
						cur_fun->get_name(),
						trg ? "class" : "type",
						trg ? trg->get_name() : "var",
						src ? "class " : "",
						src ? src->get_name() : "var");
				yyerror(buf);
				$$ = 0;
				}
			else if (Incompatible_classes_error(src, trg))
				$$ = 0;
			else
				$$ = new Uc_return_statement($2);
			}
		}
	| RETURN ';'
		{
		if (cur_fun->get_has_ret())
			{
			Uc_class *cls = cur_fun->get_cls();
			char buf[180];
			sprintf(buf, "Function '%s' must return a '%s'",
					cur_fun->get_name(), cls ? cls->get_name() : "var");
			yyerror(buf);
			$$ = 0;
			}
		else
			$$ = new Uc_return_statement();
		}
	;

converse_statement:
	CONVERSE statement
		{ $$ = new Uc_converse_statement($2); }
	;

converse2_statement:			/* A less wordy form.		*/
	CONVERSE '(' expression ')' '{' converse_case_list '}'
		{
		if (Class_unexpected_error($3))
			$$ = 0;
		else
			$$ = new Uc_converse2_statement($3, $6);
		}
	;

converse_case_list:
	converse_case_list converse_case
		{
		if ($2)
			$$->add($2);
		}
	|
		{ $$ = new Uc_block_statement(); }
	;

converse_case:
	CASE STRING_LITERAL converse_options ':' statement_list
		{
		$$ = new Uc_converse_case_statement(cur_fun->add_string($2),
				($3 ? true : false), $5);
		}
	;

converse_options:
	'(' REMOVE ')'			/* For now, just one.		*/
		{ $$ = 1; }
	|
		{ $$ = 0; }
	;

script_statement:			/* Yes, this could be an intrinsic. */
	SCRIPT { start_script(); } item opt_script_delay script_command 
		{
		Uc_array_expression *parms = new Uc_array_expression();
		parms->add($3);		// Itemref.
		parms->add($5);		// Script.
		if ($4)			// Delay?
			parms->add($4);
					// Get the script intrinsic.
		Uc_symbol *sym = Uc_function::get_intrinsic($4 ? 2 : 1);
		Uc_call_expression *fcall = 
				new Uc_call_expression(sym, parms, cur_fun);
		$$ = new Uc_call_statement(fcall);
		end_script();
		}
	;

item:					/* Any object, NPC.	*/
	nonclass_expr
	;

script_command_list:
	script_command_list script_command
		{ $$->concat($2); }
	| script_command
		{
		$$ = new Uc_array_expression();
		$$->concat($1);
		}
	;

script_command:
	FINISH ';'
		{ $$ = new Uc_int_expression(Ucscript::finish); }
	| RESURRECT ';'
		{ $$ = new Uc_int_expression(Ucscript::resurrect); }
	| CONTINUE ';'			/* Continue script without painting. */
		{ $$ = new Uc_int_expression(Ucscript::cont); }
	| REPEAT nonclass_expr script_command  ';'
		{
		Uc_array_expression *result = new Uc_array_expression();
		result->concat($3);	// Start with cmnds. to repeat.
		int sz = result->get_exprs().size();
		result->add(new Uc_int_expression(Ucscript::repeat));
					// Then -offset to start.
		result->add(new Uc_int_expression(-sz));
		result->add($2);	// Then #times to repeat.
		$$ = result;
		}
					/* REPEAT2? */
	| NOP  ';'
		{ $$ = new Uc_int_expression(Ucscript::nop); }
	| NOHALT  ';'
		{ $$ = new Uc_int_expression(Ucscript::dont_halt); }
	| WAIT nonclass_expr  ';'		/* Ticks. */
		{ $$ = Create_array(Ucscript::delay_ticks, $2); }
	| WAIT nonclass_expr HOURS  ';'	/* Game hours. */
		{ $$ = Create_array(Ucscript::delay_hours, $2); }
	| REMOVE ';'			/* Remove item. */
		{ $$ = new Uc_int_expression(Ucscript::remove); }
	| RISE ';'			/* For flying barges. */
		{ $$ = new Uc_int_expression(Ucscript::rise); }
	| DESCEND ';'
		{ $$ = new Uc_int_expression(Ucscript::descend); }
	| FRAME nonclass_expr ';'
		{ $$ = Create_array(Ucscript::frame, $2); }
	| ACTOR FRAME int_literal ';'	/* 0-15. ++++Maybe have keywords. */
		{ $$ = new Uc_int_expression(0x61 + ($3 & 15)); }
	| HATCH ';'			/* Assumes item is an egg. */
		{ $$ = new Uc_int_expression(Ucscript::egg); }
	| SETEGG nonclass_expr ',' nonclass_expr ';'
		{ $$ = Create_array(Ucscript::set_egg, $2, $4); }
	| NEXT FRAME ';'		/* Next, but stop at last. */
		{ $$ = new Uc_int_expression(Ucscript::next_frame_max); }
	| NEXT FRAME CYCLE ';'		/* Next, or back to 0. */
		{ $$ = new Uc_int_expression(Ucscript::next_frame); }
	| PREVIOUS FRAME ';'		/* Prev. but stop at 0. */
		{ $$ = new Uc_int_expression(Ucscript::prev_frame_min); }
	| PREVIOUS FRAME CYCLE ';'
		{ $$ = new Uc_int_expression(Ucscript::prev_frame); }
	| SAY nonclass_expr ';'
		{ $$ = Create_array(Ucscript::say, $2); }
	| STEP nonclass_expr ';'		/* Step in given direction (0-7). */
		{ $$ = Create_array(Ucscript::step, $2); }
	| STEP direction ';'
		{ $$ = new Uc_int_expression(Ucscript::step_n + $2); }
	| MUSIC nonclass_expr ';'
		{ $$ = Create_array(Ucscript::music, $2); }
	| start_call ';'
		{ $$ = Create_array(Ucscript::usecode, $1); }
	| start_call ',' eventid ';'
		{ $$ = Create_array(Ucscript::usecode2, $1, 
				new Uc_int_expression($3)); }
	| SPEECH nonclass_expr ';'
		{ $$ = Create_array(Ucscript::speech, $2); }
	| SFX nonclass_expr ';'
		{ $$ = Create_array(Ucscript::sfx, $2); }
	| FACE nonclass_expr ';'
		{ $$ = Create_array(Ucscript::face_dir, $2); }
	| HIT nonclass_expr ';'		/* 2nd parm unknown. */
		{ $$ = Create_array(Ucscript::hit, $2); }
	| ATTACK ';'
		{ $$ = new Uc_int_expression(Ucscript::attack); }
	| '{' script_command_list '}'
		{ $$ = $2; }
	;

start_call:
	CALL nonclass_expr
		{ $$ = $2; }
	;

direction:
	NORTH
		{ $$ = 0; }
	| NE
		{ $$ = 1; }
	| EAST
		{ $$ = 2; }
	| SE
		{ $$ = 3; }
	| SOUTH
		{ $$ = 4; }
	| SW
		{ $$ = 5; }
	| WEST
		{ $$ = 6; }
	| NW
		{ $$ = 7; }
	;

eventid:
	int_literal
	;

opt_script_delay:
	AFTER nonclass_expr TICKS
		{ $$ = $2; }
	|
		{ $$ = 0; }
	;

break_statement:
	BREAK ';'
		{ $$ = new Uc_break_statement(); }
	;

label_statement:
	IDENTIFIER ':'
		{
			Uc_label *label = cur_fun->search_label($1);
			if (label) {
				char buf[150];
				sprintf(buf, "duplicate label: '%s'", $1);
				yyerror(buf);
				$$ = 0;
			}
			else {
				label = new Uc_label($1);
				cur_fun->add_label(label);
				$$ = new Uc_label_statement(label);
			}
		}
	;

goto_statement:
	GOTO IDENTIFIER
		{ $$ = new Uc_goto_statement($2); }
	;

delete_statement:
	DELETE declared_var ';'
		{
		Uc_class_inst_symbol *cls = dynamic_cast<Uc_class_inst_symbol *>($2);
		if (!cls)
			{
			char buf[150];
			sprintf(buf, "'%s' is not a class", $2->get_name());
			yyerror(buf);
			$$ = 0;
			}
		else
			$$ = new Uc_delete_statement(new Uc_del_expression(cls));
		}
	;

answer_statement:
	ADD '(' nonclass_expr_list ')' ';'
		{
		$$ = new Uc_call_statement(
			new Uc_call_expression(Uc_function::get_add_answer(),
								$3, cur_fun));
		}
	| REMOVE '(' nonclass_expr_list ')' ';'
		{
		$$ = new Uc_call_statement(new Uc_call_expression(
					Uc_function::get_remove_answer(),
								$3, cur_fun));
		}
	;

opt_nonclass_expr_list:
	nonclass_expr_list
	|
		{ $$ = new Uc_array_expression(); }
	;

nonclass_expr_list:
	nonclass_expr_list ',' nonclass_expr
		{ $$->add($3); }
	| nonclass_expr
		{
		$$ = new Uc_array_expression();
		$$->add($1);
		}
	;

nonclass_expr:
	expression
		{
		if (Class_unexpected_error($1))
			$$ = 0;
		else
			$$ = $1;
		}

expression:
	primary
		{ $$ = $1; }
	| nonclass_expr '+' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_ADD, $1, $3); }
	| nonclass_expr '-' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_SUB, $1, $3); }
	| nonclass_expr '*' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_MUL, $1, $3); }
	| nonclass_expr '/' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_DIV, $1, $3); }
	| nonclass_expr '%' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_MOD, $1, $3); }
	| nonclass_expr EQUALS nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPEQ, $1, $3); }
	| RESPONSE EQUALS nonclass_expr
		{ $$ = new Uc_response_expression($3); }
	| nonclass_expr NEQUALS nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPNE, $1, $3); }
	| nonclass_expr '<' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPL, $1, $3); }
	| nonclass_expr LTEQUALS nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPLE, $1, $3); }
	| nonclass_expr '>' nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPG, $1, $3); }
	| nonclass_expr GTEQUALS nonclass_expr
		{ $$ = new Uc_binary_expression(UC_CMPGE, $1, $3); }
	| nonclass_expr AND nonclass_expr
		{ $$ = new Uc_binary_expression(UC_AND, $1, $3); }
	| nonclass_expr OR nonclass_expr
		{ $$ = new Uc_binary_expression(UC_OR, $1, $3); }
	| nonclass_expr UCC_IN nonclass_expr	/* Value in array. */
		{ $$ = new Uc_binary_expression(UC_IN, $1, $3); }
	| nonclass_expr '&' nonclass_expr	/* append arrays */
		{ $$ = new Uc_binary_expression(UC_ARRA, $1, $3); }
	| '-' primary
		{
		if (Class_unexpected_error($2))
			$$ = 0;
		else
			$$ = new Uc_binary_expression(UC_SUB,
				new Uc_int_expression(0), $2);
		}
	| addressof
		{ $$ = $1; }
	| NOT primary
		{ $$ = new Uc_unary_expression(UC_NOT, $2); }
	| '[' opt_expression_list ']'	/* Concat. into an array. */
		{ $$ = $2; }
	| STRING_LITERAL
		{ $$ = new Uc_string_expression(cur_fun->add_string($1)); }
	| STRING_PREFIX
		{ $$ = new Uc_string_prefix_expression(cur_fun, $1); }
	| new_expr
	;

addressof:
	'&' IDENTIFIER
		{	// A way to retrieve the function's assigned
			// usecode number
		Uc_symbol *sym = cur_fun->search_up($2);
		if (!sym)	/* See if the symbol is defined */
			{
			char buf[150];
			sprintf(buf, "'%s' not declared", $2);
			yyerror(buf);
			$$ = 0;
			}
		Uc_function_symbol *fun = (Uc_function_symbol *) sym;
		if (!fun)	/* See if the symbol is a function */
			{
			char buf[150];
			sprintf(buf, "'%s' is not a function", $2);
			yyerror(buf);
			$$ = 0;
			}
		else		/* Output the function's assigned number */
			$$ = new Uc_int_expression(fun->get_usecode_num());
		}
	;

opt_expression_list:
	expression_list
	|
		{ $$ = new Uc_array_expression(); }
	;

expression_list:
	expression_list ',' expression
		{ $$->add($3); }
	| expression
		{
		$$ = new Uc_array_expression();
		$$->add($1);
		}
	;

primary:
	INT_LITERAL
		{ $$ = new Uc_int_expression($1); }
	| declared_var_value
		{ $$ = $1; }
	| declared_var '[' expression ']'
		{
		if ($1->get_cls())
			{
			char buf[150];
			sprintf(buf, "Can't convert class '%s' into non-class",
					$1->get_name());
			yyerror(buf);
			$$ = new Uc_arrayelem_expression($1, $3);
			}
		else if ($1->is_static())
			$$ = new Uc_static_arrayelem_expression($1, $3);
		else
			$$ = new Uc_arrayelem_expression($1, $3);
		}
	| FLAG '[' int_literal ']'
		{ $$ = new Uc_flag_expression($3); }
	| function_call
		{ $$ = $1; }
	| UCTRUE
		{ $$ = new Uc_bool_expression(true); }
	| UCFALSE
		{ $$ = new Uc_bool_expression(false); }
	| EVENT
		{ $$ = new Uc_event_expression(); }
	| ITEM
		{ $$ = new Uc_item_expression(); }
	| '(' expression ')'
		{ $$ = $2; }
	;

new_expr:
	NEW defined_class '(' opt_nonclass_expr_list ')'
		{
		$$ = new Uc_new_expression(new Uc_class_inst_symbol("", $2, 0), $4);
		}
	;

hierarchy_tok:
	UCC_POINTS
	| '.'
	;

function_call:
	primary hierarchy_tok IDENTIFIER opt_original '(' opt_expression_list ')'
		{
		$$ = cls_function_call($1, cur_class, $3, $4, $6);
		}
	| IDENTIFIER opt_original '(' opt_expression_list ')'
		{
		$$ = cls_function_call(0, cur_class, $1, $2, $4);
		}
	| primary hierarchy_tok defined_class UCC_SCOPE IDENTIFIER '(' opt_expression_list ')'
		{
		$$ = cls_method_call($1, $1->get_cls(), $3, $5, $7);
		}
	| defined_class UCC_SCOPE IDENTIFIER '(' opt_expression_list ')'
		{
		$$ = cls_method_call(0, cur_class, $1, $3, $5);
		}
	| primary hierarchy_tok '(' '*' primary ')' '(' opt_expression_list ')'
		{
		$$ = new Uc_call_expression($5, $8, cur_fun);
		$$->set_itemref($1);
		}
	| '(' '*' primary ')' '(' opt_expression_list ')'
		{
		$$ = new Uc_call_expression($3, $6, cur_fun);
		$$->set_itemref(0);
		}
	;

opt_original:
	ORIGINAL
		{ $$ = 1; }
	|
		{ $$ = 0; }
	;

opt_param_list:
	param_list
	|
		{ $$ = new std::vector<Uc_var_symbol *>; }
   	;

param_list:
	param_list ',' param
		{ $1->push_back($3); }
	| param
		{
		$$ = new std::vector<Uc_var_symbol *>;
		$$->push_back($1);
		}
	;

param:
	IDENTIFIER
		{ $$ = new Uc_var_symbol($1, 0); }
	| VAR IDENTIFIER
		{ $$ = new Uc_var_symbol($2, 0); }
	| CLASS '<' defined_class '>' IDENTIFIER
		{ $$ = new Uc_class_inst_symbol($5, $3, 0); }

int_literal:				/* A const. integer value.	*/
	INT_LITERAL
	| declared_sym
		{
		Uc_const_int_symbol *sym = 
				dynamic_cast<Uc_const_int_symbol *>($1);
		if (!sym)
			{
			char buf[150];
			sprintf(buf, "'%s' is not a const int");
			yyerror(buf);
			$$ = 0;
			}
		else
			$$ = sym->get_value();
		}
	;

opt_var:
	VAR
		{ $$ = 1; }
	|
		{ $$ = 0; }
	;

declared_var_value:
	declared_sym
		{
		$$ = $1->create_expression();
		if (!$$)
			{
			char buf[150];
			sprintf(buf, "Can't use '%s' here", $1->get_name());
			yyerror(buf);
			$$ = new Uc_int_expression(0);
			}
		}
	;

declared_var:
	declared_sym
		{
		Uc_var_symbol *var = dynamic_cast<Uc_var_symbol *>($1);
		if (!var)
			{
			char buf[150];
			sprintf(buf, "'%s' not a 'var'", $1);
			yyerror(buf);
			sprintf(buf, "%s_needvar", $1->get_name());
			var = cur_fun->add_symbol(buf);
			}
		$$ = var;
		}
	;

declared_sym:
	IDENTIFIER
		{
		Uc_symbol *sym = cur_fun->search_up($1);
		if (!sym)
			{
			char buf[150];
			sprintf(buf, "'%s' not declared", $1);
			yyerror(buf);
			sym = cur_fun->add_symbol($1);
			}
		$$ = sym;
		}
	;

defined_class:
	IDENTIFIER
		{ $$ = Find_class($1); }
	;

%%

/*
 *	Create an array with an integer as the first element.
 */

static Uc_array_expression *Create_array
	(
	int e1,
	Uc_expression *e2
	)
	{
	Uc_array_expression *arr = new Uc_array_expression();
	arr->add(new Uc_int_expression(e1));
	arr->add(e2);
	return arr;
	}
static Uc_array_expression *Create_array
	(
	int e1,
	Uc_expression *e2,
	Uc_expression *e3
	)
	{
	Uc_array_expression *arr = new Uc_array_expression();
	arr->add(new Uc_int_expression(e1));
	arr->add(e2);
	arr->add(e3);
	return arr;
	}
static Uc_class *Find_class
	(
	const char *nm
	)
	{
	Uc_class_symbol *csym = dynamic_cast<Uc_class_symbol *>(
			Uc_function::search_globals(nm));
	if (!csym)
		{
		char buf[150];
		sprintf(buf, "'%s' not found, or is not a class.", nm);
		yyerror(buf);
		return 0;
		}
	return csym->get_cls();
	}

static bool Class_unexpected_error(Uc_expression *expr)
	{
	if (expr->is_class())
		{
		char buf[150];
		sprintf(buf, "Can't convert class into non-class");
		yyerror(buf);
		return true;
		}
	return false;
	}

static bool Nonclass_unexpected_error(Uc_expression *expr)
	{
	if (!expr->is_class())
		{
		char buf[150];
		sprintf(buf, "Can't convert non-class into class.");
		yyerror(buf);
		return true;
		}
	return false;
	}

static bool Incompatible_classes_error(Uc_class *src, Uc_class *trg)
	{
	if (!src->is_class_compatible(trg->get_name()))
		{
		char buf[180];
		sprintf(buf, "Class '%s' can't be converted into class '%s'",
				src->get_name(), trg->get_name());
		yyerror(buf);
		return true;
		}
	return false;
	}

static Uc_call_expression *cls_method_call
	(
	Uc_expression *ths,
	Uc_class *curcls,
	Uc_class *clsscope,
	char *nm,
	Uc_array_expression *parms
	)
	{
	if (!curcls && !(ths && ths->is_class()))
		{
		char buf[150];
		sprintf(buf, "'%s' requires a class object", nm);
		yyerror(buf);
		return 0;
		}

	if (Incompatible_classes_error(curcls, clsscope))
		return 0;

	Uc_symbol *sym = clsscope->get_scope()->search(nm);
	if (!sym)
		{
		char buf[150];
		sprintf(buf, "Function '%s' is not declared in class '%s'",
				nm, clsscope->get_name());
		yyerror(buf);
		return 0;
		}

	Uc_function_symbol *fun = dynamic_cast<Uc_function_symbol *>(sym);
	if (!fun)
		{
		char buf[150];
		sprintf(buf, "'%s' is not a function", nm);
		yyerror(buf);
		return 0;
		}

	Uc_call_expression *ret =
			new Uc_call_expression(sym, parms, cur_fun, false);
	ret->set_itemref(ths);
	ret->set_call_scope(clsscope);
	ret->check_params();
	return ret;
	}

static Uc_call_expression *cls_function_call
	(
	Uc_expression *ths,
	Uc_class *curcls,
	char *nm,
	bool original,
	Uc_array_expression *parms
	)
	{
	Uc_symbol *sym = 0;
	// Check class methods first.
	if (!ths && curcls)
		sym = curcls->get_scope()->search(nm);
	else if (ths && ths->is_class())
		sym = ths->get_cls()->get_scope()->search(nm);
	
	// Search for defined functions.
	if (!sym)
		sym = cur_fun->search_up(nm);

	// Check for intrinsic name.
	if (!sym)		
		{
		string iname = string("UI_") + nm;
		sym = cur_fun->search_up(iname.c_str());
		// Treat as method call on 'item'.
		if (sym && !ths)
			ths = new Uc_item_expression();
		}

	if (!sym)
		{
		char buf[150];
		sprintf(buf, "'%s' not declared", nm);
		yyerror(buf);
		return 0;
		}
	else
		{
		Uc_call_expression *ret =
				new Uc_call_expression(sym, parms, cur_fun, original);
		ret->set_itemref(ths);
		ret->check_params();
		return ret;
		}
	}
