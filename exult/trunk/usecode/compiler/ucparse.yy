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
#include "ucfunids.h"

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


#define YYERROR_VERBOSE 1

std::vector<Uc_design_unit *> units;	// THIS is what we produce.

static Uc_function *cur_fun = 0;	// Current function being parsed.
static Uc_class *cur_class = 0;		// ...or, current class being parsed.
static int enum_val = -1;		// Keeps track of enum elements.
static Uc_expression *method_this = 0;
static bool is_extern = false;	// Marks a function symbol as being an extern

%}

%union
	{
	class Uc_symbol *sym;
	class Uc_var_symbol *var;
	class Uc_expression *expr;
	class Uc_call_expression *funcall;
	class Uc_function_symbol *funsym;
	class Uc_statement *stmt;
	class std::vector<char *> *strvec;
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
%left UCC_POINTS

/*
 *	Production types:
 */
%type <expr> expression primary declared_var_value opt_script_delay item
%type <expr> script_command start_call addressof new_expr
%type <intval> opt_int eventid direction int_literal converse_options
%type <intval> opt_original
%type <sym> declared_sym
%type <var> declared_var
%type <funsym> function_proto function_decl
%type <strvec> identifier_list opt_identifier_list
%type <stmt> statement assignment_statement if_statement while_statement
%type <stmt> statement_block return_statement function_call_statement
%type <stmt> special_method_call_statement
%type <stmt> array_loop_statement var_decl var_decl_list stmt_declaration
%type <stmt> break_statement converse_statement converse2_statement
%type <stmt> converse_case script_statement
%type <stmt> label_statement goto_statement answer_statement
%type <stmt> delete_statement
%type <block> statement_list converse_case_list
%type <arrayloop> start_array_loop
%type <exprlist> opt_expression_list expression_list script_command_list
%type <funcall> function_call routine_call method_call

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
	| class_decl
	;

class_decl:
	CLASS IDENTIFIER 
		{ cur_class = new Uc_class($2); }
		'{' class_item_list '}'
		{
		units.push_back(cur_class);
		// Add to 'globals' symbol table.
		(void) Uc_class_symbol::create($2, cur_class);
		cur_class = 0;
		}
	;

class_item_list:
	class_item_list class_item
	|				/* Empty */
	;

class_item:
	VAR var_decl_list ';'
	| method
	;

method:
	IDENTIFIER '(' opt_identifier_list ')'
		{
		$3->push_back("this");	// So it's local[0].
		Uc_function_symbol *funsym = 
			Uc_function_symbol::create($1, -1, *$3, is_extern);
		delete $3;		// A copy was made.
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
	opt_var IDENTIFIER opt_int '(' opt_identifier_list ')'
		{
		$$ = Uc_function_symbol::create($2, $3, *$5, is_extern);
		delete $5;		// A copy was made.
		}
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
	| SAY  '(' opt_expression_list ')' ';'
		{ $$ = new Uc_say_statement($3); }
	| MESSAGE '(' opt_expression_list ')' ';'
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
	| STRING string_decl_list ';'
		{ $$ = 0; }
	| const_int_decl
		{ $$ = 0; }
	| enum_decl
		{ $$ = 0; }
	| function_decl
		{
		if (!cur_fun->add_function_symbol($1))
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
	IDENTIFIER '=' expression	
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
	| IDENTIFIER '=' expression
		{
		Uc_var_symbol *var = cur_fun ? cur_fun->add_symbol($1)
					     : cur_class->add_symbol($1);
		$$ = new Uc_assignment_statement(
				new Uc_var_expression(var), $3);
		}
	;

static_decl:
	STATIC_ VAR static_var_decl_list ';'
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
		{ $$ = $3; $$->set_externed(); is_extern = false; }
	;

assignment_statement:
	expression '=' expression ';'
		{ $$ = new Uc_assignment_statement($1, $3); }
	;

if_statement:
	IF '(' expression ')' statement %prec IF
		{ $$ = new Uc_if_statement($3, $5, 0); }
	| IF '(' expression ')' statement ELSE statement
		{ $$ = new Uc_if_statement($3, $5, $7); }
	;

while_statement:
	WHILE '(' expression ')' statement
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
	primary hierarchy_tok SAY '(' opt_expression_list ')' ';'
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
	RETURN expression ';'
		{ $$ = new Uc_return_statement($2); }
	| RETURN ';'
		{ $$ = new Uc_return_statement(); }
	;

converse_statement:
	CONVERSE statement
		{ $$ = new Uc_converse_statement($2); }
	;

converse2_statement:			/* A less wordy form.		*/
	CONVERSE '(' expression ')' '{' converse_case_list '}'
		{ $$ = new Uc_converse2_statement($3, $6); }
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
	expression
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
	| REPEAT expression script_command  ';'
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
	| WAIT expression  ';'		/* Ticks. */
		{ $$ = Create_array(Ucscript::delay_ticks, $2); }
	| WAIT expression HOURS  ';'	/* Game hours. */
		{ $$ = Create_array(Ucscript::delay_hours, $2); }
	| REMOVE ';'			/* Remove item. */
		{ $$ = new Uc_int_expression(Ucscript::remove); }
	| RISE ';'			/* For flying barges. */
		{ $$ = new Uc_int_expression(Ucscript::rise); }
	| DESCEND ';'
		{ $$ = new Uc_int_expression(Ucscript::descend); }
	| FRAME expression ';'
		{ $$ = Create_array(Ucscript::frame, $2); }
	| ACTOR FRAME int_literal ';'	/* 0-15. ++++Maybe have keywords. */
		{ $$ = new Uc_int_expression(0x61 + ($3 & 15)); }
	| HATCH ';'			/* Assumes item is an egg. */
		{ $$ = new Uc_int_expression(Ucscript::egg); }
	| SETEGG expression ',' expression ';'
		{ $$ = Create_array(Ucscript::set_egg, $2, $4); }
	| NEXT FRAME ';'		/* Next, but stop at last. */
		{ $$ = new Uc_int_expression(Ucscript::next_frame_max); }
	| NEXT FRAME CYCLE ';'		/* Next, or back to 0. */
		{ $$ = new Uc_int_expression(Ucscript::next_frame); }
	| PREVIOUS FRAME ';'		/* Prev. but stop at 0. */
		{ $$ = new Uc_int_expression(Ucscript::prev_frame_min); }
	| PREVIOUS FRAME CYCLE ';'
		{ $$ = new Uc_int_expression(Ucscript::prev_frame); }
	| SAY expression ';'
		{ $$ = Create_array(Ucscript::say, $2); }
	| STEP expression ';'		/* Step in given direction (0-7). */
		{ $$ = Create_array(Ucscript::step, $2); }
	| STEP direction ';'
		{ $$ = new Uc_int_expression(Ucscript::step_n + $2); }
	| MUSIC expression ';'
		{ $$ = Create_array(Ucscript::music, $2); }
	| start_call ';'
		{ $$ = Create_array(Ucscript::usecode, $1); }
	| start_call ',' eventid ';'
		{ $$ = Create_array(Ucscript::usecode2, $1, 
				new Uc_int_expression($3)); }
	| SPEECH expression ';'
		{ $$ = Create_array(Ucscript::speech, $2); }
	| SFX expression ';'
		{ $$ = Create_array(Ucscript::sfx, $2); }
	| FACE expression ';'
		{ $$ = Create_array(Ucscript::face_dir, $2); }
	| HIT expression ';'		/* 2nd parm unknown. */
		{ $$ = Create_array(Ucscript::hit, $2); }
	| ATTACK ';'
		{ $$ = new Uc_int_expression(Ucscript::attack); }
	| '{' script_command_list '}'
		{ $$ = $2; }
	;

start_call:
	CALL expression
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
	AFTER expression TICKS
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
		Uc_expression *e = $2->create_expression(); 
		$$ = new Uc_call_statement(
			new Uc_call_expression(Uc_function::get_class_delete(),
				new Uc_array_expression(e), cur_fun));
		}
	;

answer_statement:
	ADD '(' expression_list ')' ';'
		{
		$$ = new Uc_call_statement(
			new Uc_call_expression(Uc_function::get_add_answer(),
								$3, cur_fun));
		}
	| REMOVE '(' expression_list ')' ';'
		{
		$$ = new Uc_call_statement(new Uc_call_expression(
					Uc_function::get_remove_answer(),
								$3, cur_fun));
		}
	;

expression:
	primary
		{ $$ = $1; }
	| expression '+' expression
		{ $$ = new Uc_binary_expression(UC_ADD, $1, $3); }
	| expression '-' expression
		{ $$ = new Uc_binary_expression(UC_SUB, $1, $3); }
	| expression '*' expression
		{ $$ = new Uc_binary_expression(UC_MUL, $1, $3); }
	| expression '/' expression
		{ $$ = new Uc_binary_expression(UC_DIV, $1, $3); }
	| expression '%' expression
		{ $$ = new Uc_binary_expression(UC_MOD, $1, $3); }
	| expression EQUALS expression
		{ $$ = new Uc_binary_expression(UC_CMPEQ, $1, $3); }
	| RESPONSE EQUALS expression
		{ $$ = new Uc_response_expression($3); }
	| expression NEQUALS expression
		{ $$ = new Uc_binary_expression(UC_CMPNE, $1, $3); }
	| expression '<' expression
		{ $$ = new Uc_binary_expression(UC_CMPL, $1, $3); }
	| expression LTEQUALS expression
		{ $$ = new Uc_binary_expression(UC_CMPLE, $1, $3); }
	| expression '>' expression
		{ $$ = new Uc_binary_expression(UC_CMPG, $1, $3); }
	| expression GTEQUALS expression
		{ $$ = new Uc_binary_expression(UC_CMPGE, $1, $3); }
	| expression AND expression
		{ $$ = new Uc_binary_expression(UC_AND, $1, $3); }
	| expression OR expression
		{ $$ = new Uc_binary_expression(UC_OR, $1, $3); }
	| expression UCC_IN expression	/* Value in array. */
		{ $$ = new Uc_binary_expression(UC_IN, $1, $3); }
	| expression '&' expression	/* append arrays */
		{ $$ = new Uc_binary_expression(UC_ARRA, $1, $3); }
	| '-' primary
		{ $$ = new Uc_binary_expression(UC_SUB,
				new Uc_int_expression(0), $2); }
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
		{ $$ = new Uc_arrayelem_expression($1, $3); }
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
	NEW IDENTIFIER
		{ $$ = new Uc_new_expression($2); }
	;

function_call:
	routine_call
	| method_call
	;

method_call:	/* A way to do CALLE or to call an intrinsic. */
	primary hierarchy_tok { method_this = $1;  } routine_call 
		{
		$$ = $4;
		method_this = 0;
		}
	;

hierarchy_tok:
	UCC_POINTS
	| '.'
	;

routine_call:
	IDENTIFIER opt_original '(' opt_expression_list ')'
		{ 
		Uc_symbol *sym = cur_fun->search_up($1);
		if (!sym)		/* Check for intrinsic name.	*/
			{
			string iname = string("UI_") + $1;
			sym = cur_fun->search_up(iname.c_str());
					/* Treat as method call on 'item'.*/
			if (sym && !method_this)
				method_this = new Uc_item_expression();
			}
		if (!sym)
			{
			char buf[150];
			sprintf(buf, "'%s' not declared", $1);
			yyerror(buf);
			$$ = 0;
			}
		else
			{
			$$ = new Uc_call_expression(sym, $4, cur_fun,
							$2 ? true : false);
			$$->set_itemref(method_this);
			method_this = 0;
			}
		}
	| '(' '*' primary ')' '(' opt_expression_list ')'
		{
		$$ = new Uc_call_expression($3, $6, cur_fun);
		$$->set_itemref(method_this);
		method_this = 0;
		}
	;

opt_original:
	ORIGINAL
		{ $$ = 1; }
	|
		{ $$ = 0; }
	;

opt_identifier_list:
	identifier_list
	|
		{ $$ = new std::vector<char *>; }
   	;

identifier_list:
	identifier_list ',' opt_var IDENTIFIER
		{ $1->push_back($4); }
	| opt_var IDENTIFIER
		{
		$$ = new std::vector<char *>;
		$$->push_back($2);
		}
	;

int_literal:				/* A const. integer value.	*/
	INT_LITERAL
	| SHAPENUM '(' INT_LITERAL ')'
		{ $$ = UC_SHAPEFUN($3); }
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
	|
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
