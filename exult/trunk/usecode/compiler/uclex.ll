%{
/**
 **	Uclex.ll - Usecode lexical scanner.
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

#include <string>
#include <vector>
#include "ucparse.h"
#include "ucloc.h"

extern "C" int yywrap() { return 1; }		/* Stop at EOF. */

%}


%%

if		return IF;
else		return ELSE;
return		return RETURN;
while		return WHILE;
for		return FOR;
in		return IN;
var		return VAR;
string		return STRING;

UcSay		return SAY;
UcMessage	return MESSAGE;
UcEvent		return EVENT;
UcFlag		return FLAG;
UcItem		return ITEM;

"&&"		return AND;
"||"		return OR;
"!"		return NOT;

[a-zA-Z][a-zA-Z0-9_]*	{
			yylval.strval = strdup(yytext);
			return IDENTIFIER;
			}
\"[^"]*\"		{
			yylval.strval = strdup(yytext);
			return STRING_LITERAL;
			}
[0-9]+			{
			yylval.intval = atoi(yytext);
			return INT_LITERAL;
			}
0x[0-9a-f]+		{
			yylval.intval = strtol(yytext + 2, 0, 16);
			return INT_LITERAL;
			}

"=="			{ return EQUALS; }
"!="			{ return NEQUALS; }
"<="			{ return LTEQUALS; }
">="			{ return GTEQUALS; }

[ \t]+						/* Ignore spaces. */
"//".*						/* Comments. */
\n			{ Uc_location::increment_cur_line(); }
.			return *yytext;		/* Being lazy. */


%%
