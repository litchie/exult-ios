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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <fstream.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "ucloc.h"
#include "ucfun.h"

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

using std::strcpy;
using std::strrchr;
using std::strlen;
using std::ios;

extern std::vector<Uc_function *> functions;	// THIS is what the parser produces.


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
	char outname[150];
	if (argc > 1)
		{
		src = argv[1];
		yyin = fopen(argv[1], "r");
		strcpy(outname, src);	// Set up output name.
		char *dot = strrchr(outname, '.');
		if (!dot)
			dot = outname + strlen(outname);
		strcpy(dot, ".uco");
		}
	else
		{
		src = "<stdin>";
		yyin = stdin;
		strcpy(outname, "a.ucout");
		}
	Uc_location::set_cur(src, 0);
					// For now, use black gate.
	Uc_function::set_intrinsics(Uc_function::bg);
#if 0
//++++TESTING
	int tok;
	extern int yylex();
	while ((tok = yylex()) != EOF)
		printf("%d\n", tok);
#endif
	yyparse();
	int errs = Uc_location::get_num_errors();
	if (errs > 0)			// Check for errors.
		return errs;
					// Open output.
	ofstream out(outname, ios::binary|ios::out);
	for (std::vector<Uc_function *>::iterator it = functions.begin();
					it != functions.end(); it++)
		{
		Uc_function *fun = *it;
		fun->gen(out);		// Generate function.
		}
	return Uc_location::get_num_errors();
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
