/**
 **	Ucmain.cc - Usecode Compiler
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

#include <stdio.h>
#include <string>
#include "ucloc.h"

/*
 *	MAIN.
 */

int main
	(
	int argc,
	char **argv
	)
	{
	extern int yyparse();
	extern FILE *yyin;
	char *src;
	if (argc > 1)
		{
		src = argv[1];
		yyin = fopen(argv[1], "r");
		}
	else
		{
		src = "<stdin>";
		yyin = stdin;
		}
	Uc_location::set_cur(src, 0);
#if 0
//++++TESTING
	int tok;
	extern int yylex();
	while ((tok = yylex()) != EOF)
		printf("%d\n", tok);
#endif
	int result = yyparse();
	return result;
	}

/*
 *	Report error.
 */
void yyerror
	(
	char *s
	)
	{
	Uc_location::yyerror(s);
	}
