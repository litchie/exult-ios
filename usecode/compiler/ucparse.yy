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

#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ucfun.h"
#include "ucexpr.h"
#include "ucstmt.h"
#include "opcodes.h"

void yyerror(char *);
extern int yylex();

#define YYERROR_VERBOSE 1

vector<Uc_function *> functions;	// THIS is what we produce.

static Uc_function *function = 0;	// Current function being parsed.

%}

%union
	{
	class Uc_var_symbol *var;
	class Uc_expression *expr;
	class Uc_function_symbol *funsym;
	class Uc_statement *stmt;
	class vector<char *> *strvec;
	class Uc_block_statement *block;
	class Uc_array_expression *exprlist;
	int intval;
	char *strval;
	}

/*
 *	Keywords:
 */
%token IF ELSE RETURN WHILE FOR IN
%token VAR STRING
%token SAY MESSAGE EVENT FLAG ITEM UCTRUE UCFALSE

/*
 *	Other tokens:
 */
%token <strval> STRING_LITERAL IDENTIFIER
%token <intval> INT_LITERAL

/*
 *	Handle if-then-else conflict.
 */
%left IF
%right ELSE

/*
 *	Expression precedence rules:
 */
%left AND OR
%left EQUALS NEQUALS LTEQUALS GTEQUALS '<' '>' IN
%left '-' '+'
%left '*' '/' '%'
%left NOT

/*
 *	Production types:
 */
%type <expr> expression primary function_call
%type <intval> opt_int
%type <var> declared_var
%type <funsym> function_proto
%type <strvec> identifier_list opt_identifier_list
%type <stmt> statement assignment_statement if_statement while_statement
%type <stmt> statement_block return_statement
%type <block> statement_list
%type <exprlist> opt_expression_list expression_list

%%

design:
	design function
	| function
	;

function:
	function_proto 
		{ function = new Uc_function($1); }
		statement_block
		{ 
		function->set_statement($3);
		functions.push_back(function);
		}
	;

					/* Opt_int assigns function #. */
function_proto:
	IDENTIFIER opt_int '(' opt_identifier_list ')'
		{
		$$ = new Uc_function_symbol($1, $2, *$4);
		delete $4;		// A copy was made.
		}
	;

opt_int:
	INT_LITERAL
	|				/* Empty. */
		{ $$ = -1; }
	;

statement_block:
	'{' 
		{ function->push_scope(); }
	statement_list '}'
		{
		$$ = $3;
		function->pop_scope();
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
	declaration
		{ $$ = 0; }
	| assignment_statement
	| if_statement
	| while_statement
	| array_loop_statement
		{ $$ = 0; /* ++++++++ */ }
	| function_call_statement
		{ $$ = 0; /* ++++++++ */ }
	| return_statement
	| statement_block
	| SAY ';'
		{ $$ = new Uc_say_statement(); }
	| MESSAGE '(' expression ')' ';'
		{ $$ = new Uc_message_statement($3); }
	| ';'				/* Null statement */
		{ $$ = 0; }
	;

declaration:
	VAR identifier_list ';'
		{
		for (vector<char*>::const_iterator it = $2->begin();
					it != $2->end(); it++)
			function->add_symbol(*it);
		}
	| STRING IDENTIFIER '=' STRING_LITERAL ';'
		{
		function->add_string_symbol($2, $4);
		}
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
	FOR '(' IDENTIFIER IN declared_var ')' statement
	;

function_call_statement:
	function_call ';'
		{  }
	;

return_statement:
	RETURN expression ';'
		{ $$ = new Uc_return_statement($2); }
	| RETURN ';'
		{ $$ = new Uc_return_statement(); }
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
	| expression IN expression	/* Value in array. */
		{ $$ = new Uc_binary_expression(UC_IN, $1, $3); }
	| '-' primary
		{ $$ = new Uc_binary_expression(UC_SUB,
				new Uc_int_expression(0), $2); }
	| NOT primary
		{ $$ = new Uc_unary_expression(UC_NOT, $2); }
	| '[' expression_list ']'	/* Concat. into an array. */
		{ $$ = $2; }
	| STRING_LITERAL
		{ $$ = new Uc_string_expression(function->add_string($1)); }
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
	| declared_var
		{ $$ = new Uc_var_expression($1); }
	| declared_var '[' expression ']'
		{ $$ = new Uc_arrayelem_expression($1, $3); }
	| function_call
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

function_call:
	IDENTIFIER '(' opt_expression_list ')'
		{ $$ = 0; /* ++++++++ */ }
	;

opt_identifier_list:
	identifier_list
	|
		{ $$ = new vector<char *>; }
	;

identifier_list:
	identifier_list ',' IDENTIFIER
		{ $1->push_back($3); }
	| IDENTIFIER
		{
		$$ = new vector<char *>;
		$$->push_back($1);
		}
	;

declared_var:
	IDENTIFIER
		{
		Uc_var_symbol *var = dynamic_cast<Uc_var_symbol *>
					(function->search_up($1));
		if (!var)
			{
			char buf[150];
			sprintf(buf, "'%s' not declared", $1);
			yyerror(buf);
			var = function->add_symbol($1);
			}
		$$ = var;
		}
	;

%%

