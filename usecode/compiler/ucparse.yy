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

#include "ucsym.h"
#include "ucexpr.h"
#include "opcodes.h"

void yyerror(char *);
extern int yylex();

#define YYERROR_VERBOSE 1

%}

%union
	{
	class Uc_expression *expr;
	class Uc_function_symbol *funsym;
	class vector<char *> *strvec;
	int intval;
	char *strval;
	}

/*
 *	Keywords:
 */
%token IF ELSE RETURN WHILE FOR IN
%token VAR STRING

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
%left '-' '+'
%left '*' '/' '%'
%left NOT

/*
 *	Production types:
 */
%type <expr> expression primary function_call
%type <intval> opt_int
%type <funsym> function_proto
%type <strval> identifier
%type <strvec> identifier_list opt_identifier_list

%%

design:
	design function
	| function
	;

function:
	function_proto statement_block
		{  }
	;

					/* Opt_int assigns function #. */
function_proto:
	IDENTIFIER opt_int '(' opt_identifier_list ')'
		{
		$$ = new Uc_function_symbol($1, $2, *$4);
		delete $4;		// A copy was made.
		}
	;

decl_type:
	VAR
	| STRING
	;

opt_int:
	INT_LITERAL
	|				/* Empty. */
		{ $$ = -1; }
	;

statement_block:
	'{' statement_list '}'
	;

statement_list:
	statement_list statement
	|				/* Empty. */
	;

statement:
	declaration
	| assignment_statement
	| if_statement
	| while_statement
	| array_loop_statement
	| function_call_statement
	| return_statement
	| statement_block
	| ';'				/* Null statement */
	;

declaration:
	decl_type identifier_list ';'
	;

assignment_statement:
	expression '=' expression ';'
		{  }			/* ++++++++ */
	;

if_statement:
	IF '(' expression ')' statement %prec IF
	| IF '(' expression ')' statement ELSE statement
	;

while_statement:
	WHILE '(' expression ')' statement
	;

array_loop_statement:
	FOR '(' IDENTIFIER IN identifier ')' statement
	;

function_call_statement:
	function_call ';'
		{  }
	;

return_statement:
	RETURN expression ';'
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
	| expression AND expression
		{ $$ = new Uc_binary_expression(UC_AND, $1, $3); }
	| expression OR expression
		{ $$ = new Uc_binary_expression(UC_OR, $1, $3); }
	| '-' primary
		{ $$ = 0; /* ++++++ */ }
	| NOT primary
		{ $$ = new Uc_unary_expression(UC_NOT, $2); }
	| '[' expression_list ']'	/* Concat. into an array. */
		{ $$ = 0; /* ++++++ */ }
	| STRING_LITERAL
		{ $$ = 0; /* ++++++ */ }
	;

opt_expression_list:
	expression_list
	|
	;

expression_list:
	expression_list ',' expression
	| expression
		{  }			/* ++++++++ */
	;

primary:
	INT_LITERAL
		{ $$ = new Uc_int_expression($1); }
	| identifier
		{ $$ = 0; /* ++++++ */ }
	| identifier '[' expression ']'
		{ $$ = 0; /* ++++++ */ }
	| function_call
	| '(' expression ')'
		{ $$ = $2; }
	;

function_call:
	identifier '(' opt_expression_list ')'
		{ $$ = 0; /* ++++++++ */ }
	;

opt_identifier_list:
	identifier_list
	|
		{ $$ = new vector<char *>; }
	;

identifier_list:
	identifier_list ',' identifier
		{ $1->push_back($3); }
	| identifier
		{
		$$ = new vector<char *>;
		$$->push_back($1);
		}
	;

identifier:
	IDENTIFIER
	;

%%

