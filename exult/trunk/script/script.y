%{
/**
 **	Script.y - NPC script parser for Exult.
 **
 **	Written: 5/26/99 - JSF
 **/

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsm.h"
#include "lists.h"
#include "exec.h"
#include "convers.h"
#include "cond.h"

extern int yylex();
extern void yyerror(char *);

/*
 *	Here's what we produce:
 */
static Slist *npc_list = 0;		// List of Npc_spec's.
char *script_name = 0;			// Name of file being compiled.

%}

/*
 *	Types:
 */
%union
	{
	long integer;
	char *string;
	class Slist *list;
	class Npc_actor *npc;
	class Npc_option *option;
	class Npc_stmt *stmt;
	class Npc_spec *npc_spec;
	class State_spec *state;
	class Response_spec *response;
	class Action_spec *action;
	class Cond_spec *cond;
	class Expr *expr;
	}

/*
 *	Tokens:
 */
%token NPC STATE TOPIC WHEN STARTING ASKED ALWAYS SAY PREVIOUS TO
%token TOLD LOCATION SHAPE PORTRAIT ATTRIBUTE
%token <string> IDENTIFIER STRING
%token <integer> INTEGER

/*
 *	Operators:
 */
%left AND OR
%left '<' '>' LTE GTE EQ NEQ
%left '+' '-'
%left '*' '/' '%'
%left '!'

/*
 *	Non-terminals:
 */
%type <list> npc_list options option_list npc_statement_list
%type <list> state_list response_list action_list
%type <npc_spec> npc
%type <state> state
%type <response> response
%type <cond> cond_clause
%type <action> action say_action begin_topic_action prev_topic_action
%type <action> new_state_action att_assign_action
%type <stmt> attribute_decl topic npc_statement
%type <integer> location
%type <option> option
%type <expr> expr

%%

script:
	{ Sentence::init(); } npc_list
		{ npc_list = $2; }
	;

npc_list:
	npc_list npc
		{ $1->append($2); }
	|				/* Empty.			*/
		{ $$ = new Slist; }
	;

npc:					/* Non-player character.	*/
	NPC IDENTIFIER options '{' npc_statement_list '}'
		{ $$ = Create_npc($2, $3, $5); }
	;

options:
	'(' option_list ')'
		{ $$ = $2; }
	| '(' ')'
		{ $$ = new Slist; }
	;

option_list:
	option_list ',' option
		{ $1->append($3); }
	| option
		{
		$$ = new Slist;
		$$->append($1);
		}
	;

option:
	SHAPE '=' INTEGER
		{ $$ = new Npc_option(SHAPE, $3); }
	| PORTRAIT '=' INTEGER
		{ $$ = new Npc_option(PORTRAIT, $3); }
	| LOCATION '=' location
		{ $$ = new Npc_option(LOCATION, $3); }
	;	

npc_statement_list:
	npc_statement_list npc_statement
		{ $1->append($2); }
	|
		{ $$ = new Slist; }
	;

npc_statement:
	topic
	| attribute_decl
	;

attribute_decl:
	ATTRIBUTE IDENTIFIER '(' INTEGER TO INTEGER ')' '=' INTEGER ';'
		{ $$ = new Npc_att_stmt($2, $4, $6, $9); }
	;

topic:
	TOPIC IDENTIFIER '{' state_list '}'
		{ $$ = new Npc_topic_stmt($2, $4); }
	;

state_list:
	state_list state
		{ $1->append($2); }
	|
		{ $$ = new Slist; }
	;

state:
	STATE IDENTIFIER '{' response_list '}'
		{ $$ = new State_spec($2, $4); }
	;

response_list:				/* Transitions out of the state. */
	response_list response
		{ $1->append($2); }
	|
		{ $$ = new Slist; }
	;

response:
	cond_clause ':' action_list
		{ $$ = new Response_spec($1, $3); }
	;

cond_clause:
	WHEN STARTING
		{ $$ = new Cond_known_spec(new Start_condition); }
	| WHEN asked STRING
		{ 
		$$ = new Cond_known_spec(new Sentence_condition(
				Sentence::create($3)));
		delete $3;
		}
	| WHEN expr
		{ $$ = new Expr_cond_spec($2); }
	| ALWAYS
		{ $$ = new Expr_cond_spec(new Int_expr(1)); }
	;

asked:
	ASKED
	| TOLD
	;

action_list:
	action_list action ';'
		{ $1->append($2); }
	|
		{ $$ = new Slist; }
	;

action:
	say_action
	| begin_topic_action
	| prev_topic_action
	| new_state_action
	| att_assign_action
	;

say_action:
	SAY STRING
		{ $$ = new Say_action_spec($2); }
	;

begin_topic_action:
	TOPIC '=' IDENTIFIER
		{ $$ = new Topic_action_spec($3); }
	;

prev_topic_action:
	TOPIC '=' PREVIOUS
		{ $$ = new Topic_prev_spec; }
	;

new_state_action:
	STATE '=' IDENTIFIER
		{ $$ = new State_action_spec($3); }
	;

att_assign_action:
	ATTRIBUTE IDENTIFIER '=' expr
		{ $$ = new Att_action_spec($2, $4); }
	;

/*
opt_identifier:
	IDENTIFIER
	|
		{ $$ = 0; }
	;
 */

location:
	'(' INTEGER ',' INTEGER ')'
		{ $$ = ($2 << 16) | $4; }
	;

/*
 *	Expressions:
 */
expr:
	INTEGER
		{ $$ = new Int_expr((int) $1); }
	| IDENTIFIER '.' IDENTIFIER
		{ $$ = new Att_name_expr($1, $3); }
	| '(' expr ')'
		{ $$ = $2; }
	| expr EQ expr
		{ $$ = new Binary_expr(Expr::eq, $1, $3); }
	| expr NEQ expr
		{ $$ = new Binary_expr(Expr::neq, $1, $3); }
	| expr '<' expr
		{ $$ = new Binary_expr(Expr::lt, $1, $3); }
	| expr LTE expr
		{ $$ = new Binary_expr(Expr::lte, $1, $3); }
	| expr '>' expr
		{ $$ = new Binary_expr(Expr::gt, $1, $3); }
	| expr GTE expr
		{ $$ = new Binary_expr(Expr::gte, $1, $3); }
	| expr '+' expr
		{ $$ = new Binary_expr(Expr::plus, $1, $3); }
	| expr '-' expr
		{ $$ = new Binary_expr(Expr::minus, $1, $3); }
	| expr '*' expr
		{ $$ = new Binary_expr(Expr::mul, $1, $3); }
	| expr '/' expr
		{ $$ = new Binary_expr(Expr::div, $1, $3); }
	| expr '%' expr
		{ $$ = new Binary_expr(Expr::mod, $1, $3); }
	| expr AND expr
		{ $$ = new Binary_expr(Expr::and, $1, $3); }
	| expr OR expr
		{ $$ = new Binary_expr(Expr::or, $1, $3); }
	| '-' expr
		{ $$ = new Unary_expr(Expr::neg, $2); }
	| '!' expr
		{ $$ = new Unary_expr(Expr::not, $2); }
	;

%%

void yyerror
	(
	char *str
	)
	{
	extern int line_num, num_errors;
	extern char *script_name;
	fprintf(stderr, "%s:%d: %s\n", script_name, line_num, str);
	num_errors++;
	}

/*
 *	Find an NPC by name.
 *
 *	Output:	Npc_spec, or 0.
 */

Npc_spec *Find_npc
	(
	char *nm
	)
	{
	Slist_iterator next(npc_list);
	Npc_spec *each;
	while ((each = (Npc_spec *) next()) != 0)
		if (strcmp(nm, each->get_npc()->get_name()) == 0)
			return (each);
	return (0);
	}

#include "compile.h"

/*
 *	Compile.
 */

Script_compiler::Script_compiler
	(
	char *sname			// Filename.
	) : error(0), next(0)
	{
	extern FILE *yyin;
	extern int yyparse();
	extern int num_errors;

	script_name = sname;		// Set global.
	if ((yyin = fopen(script_name, "r")) == 0)
		{
		fprintf(stderr, "Can't open input '%s'.\n", script_name);
		error = -2;
		return;
		}
	yyparse();
	fclose(yyin);
	if (!num_errors)		// Okay?
		{			// Translate actions.
		Slist_iterator nxt(npc_list);
		Npc_spec *npc_spec;
		while ((npc_spec = (Npc_spec *) nxt()) != 0)
			npc_spec->translate();
		}
	error = num_errors;		// Save # errors.
	}

/*
 *	Done with it.
 */

Script_compiler::~Script_compiler
	(
	)
	{
	delete next;
	}

/*
 *	Get next NPC.
 *
 *	Output:	0 if no more.
 */

int Script_compiler::operator()
	(
	Npc *& npc,			// Return NPC.
	int& shapeid, int& portraitid,	// Return VGA ID's.
	int& cx, int& cy, 		// Return chunk coords.
	int& sx, int& sy		// Return shape-within-chunk coords.
	)
	{
	if (!next)
		next = new Slist_iterator(npc_list);
					// Get next one.
	Npc_spec *npc_spec = (Npc_spec *) (*next)();
	if (!npc_spec)
		return (0);		// All done.
	npc = npc_spec->npc;
	shapeid = npc_spec->shape;
	portraitid = npc_spec->portrait;
	cx = npc_spec->chunkx;
	cy = npc_spec->chunky;
	sx = npc_spec->shapex;
	sy = npc_spec->shapey;
	return (1);
	}
