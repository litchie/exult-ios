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

void yyerror(char *);
extern int yylex();

#define YYERROR_VERBOSE 1

%}

/*
 *	Keywords:
 */
%token IF ELSE RETURN WHILE FOR IN
%token VOID INT STRING ARRAY

/*
 *	Other tokens:
 */
%token STRING_LITERAL INT_LITERAL IDENTIFIER

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

%%

design:
	design function
	| function
	;

function:
	function_proto statement_block

					/* Opt_int assigns function #. */
function_proto:
	decl_type IDENTIFIER opt_int '(' opt_parm_decl_list ')'
	;

opt_parm_decl_list:
	parm_decl_list
	|
	;

parm_decl_list:
	parm_decl_list ',' parm_decl
	| parm_decl
	;

parm_decl:
	decl_type IDENTIFIER
	;

decl_type:
	VOID
	| INT
	| STRING
	| ARRAY
	;

opt_int:
	INT_LITERAL
	|				/* Empty. */
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
	;

return_statement:
	RETURN expression ';'
	;

expression:
	primary
	| expression '+' expression
	| expression '-' expression
	| expression '*' expression
	| expression '/' expression
	| expression '%' expression	/* MOD. */
	| expression AND expression
	| expression OR expression
	| '-' primary
	| NOT primary
	| '[' expression_list ']'	/* Concat. into an array. */
	| STRING_LITERAL
	;

opt_expression_list:
	expression_list
	|
	;

expression_list:
	expression_list ',' expression
	| expression
	;

primary:
	INT_LITERAL
	| identifier
	| identifier '[' expression ']'
	| function_call
	| '(' expression ')'
	;

function_call:
	identifier '(' opt_expression_list ')'
	;

/*
opt_identifier_list:
	identifier_list
	|
	;
*/

identifier_list:
	identifier_list ',' identifier
	| identifier
	;

identifier:
	IDENTIFIER
	;

%%

