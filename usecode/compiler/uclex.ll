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
#include "ucparse.h"

int uc_line_num = 0;			// Counts lines.
string uc_source = "";			// Source filename.

extern "C" int yywrap() { return 1; }		/* Stop at EOF. */

%}


%%

if		return IF;
else		return ELSE;
return		return RETURN;
while		return WHILE;
for		return FOR;
in		return IN;
void		return VOID;
string		return STRING;
int		return INT;
array		return ARRAY;

"&&"		return AND;
"||"		return OR;
"!"		return NOT;

[a-zA-Z][a-zA-Z0-9_]*	return IDENTIFIER;	/* Return string. */
\"[^"]*\"		return STRING_LITERAL;
[0-9]+			return INT_LITERAL;
0x[0-9a-f]+		return INT_LITERAL;

[ \t]+						/* Ignore spaces. */
"//".*						/* Comments. */
\n			{ uc_line_num++; }
.			return *yytext;		/* Being lazy. */


%%
