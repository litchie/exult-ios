%{
/**
 **	Scrlex.l - Lexer for Exult script grammar.
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

#include <stdlib.h>
#include <string.h>
#include "script.h"

int line_num = 1;			/* Keeps track of lines.	*/
int num_errors = 0;			/* # errors seen.		*/

%}

%%

npc	return NPC;

state	return STATE;

topic	return TOPIC;

when	return WHEN;

starting	return STARTING;

asked		return ASKED;

told		return TOLD;

always		return ALWAYS;

say		return SAY;

previous	return PREVIOUS;

location	return LOCATION;

shape		return SHAPE;

portrait	return PORTRAIT;

attribute	return ATTRIBUTE;

to		return TO;

--[^\n]*				/* 1-line comments.		*/

[0-9]*		{
		yylval.integer = atol(yytext);
		return INTEGER;
		}

[a-zA-Z][_a-zA-Z0-9]*	{
		yylval.string = strdup(yytext);
		return IDENTIFIER;
		}

\"([^"]|\"\")*\"	{
		yylval.string = strdup(yytext + 1);
					/* Don't want the '"'s.		*/
		yylval.string[strlen(yytext + 1) - 1] = 0;
		return STRING;
		}

\n		{
		line_num++;
		}

[ \t\r]+	/* Eat whitespace. */

"+"|"-"|"*"|"/"|"%"|"<"|">"	return *yytext;	/* Operators. */

"<="		return LTE;
">="		return GTE;
"=="		return EQ;
"!="		return NEQ;
"&&"		return AND;
"||"		return OR;

"="|";"|":"|"{"|"}"|","|"("|")"|"!"|"."		{
		return (*yytext);   	/* Characters handled. */
		}

.		{
		/* Illegal char. */
		}

%%

extern "C" int yywrap()			/* Quit at end of file.		*/
	{ return 1; }
