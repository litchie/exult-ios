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

#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "ucloc.h"
#include "ucfun.h"
#include "ucsymtbl.h"
#include "utils.h"

#ifndef ALPHA_LINUX_CXX
#  include <iosfwd>
#endif

using std::strcpy;
using std::strrchr;
using std::strlen;
using std::ios;

					// THIS is what the parser produces.
extern std::vector<Uc_function *> functions;	

std::vector<char *> include_dirs;	// -I directories.

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
	char outbuf[256];
	char *outname = 0;
	static char *optstring = "o:I:s";
	extern int optind, opterr, optopt;
	extern char *optarg;
	Uc_function::Intrinsic_type ty = Uc_function::unset;
	opterr = 0;			// Don't let getopt() print errs.
	int optchr;
	while ((optchr = getopt(argc, argv, optstring)) != -1)
		switch (optchr)
			{
		case 'o':		// Output to write.
			outname = optarg;
			break;
		case 'I':		// Include dir.
			include_dirs.push_back(optarg);
			break;
		case 's':
			ty = Uc_function::si;
			break;
			}
	char *env = getenv("UCC_INCLUDE");
	if (env)
		include_dirs.push_back(env);
	if (optind < argc)		// Filename?
		{
		src = argv[optind];
		yyin = fopen(argv[optind], "r");
		if (!outname)		// No -o option?
			{		// Set up output name.
			outname = strncpy(outbuf, src, sizeof(outbuf) - 5);
			outbuf[sizeof(outbuf) - 5] = 0;
			char *dot = strrchr(outname, '.');
			if (!dot)
				dot = outname + strlen(outname);
			strcpy(dot, ".uco");
			}
		}
	else
		{
		src = "<stdin>";
		yyin = stdin;
		if (!outname)
			outname = strcpy(outbuf, "a.ucout");
		}
	Uc_location::set_cur(src, 0);
	Uc_function::set_intrinsic_type(ty);
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
	std::ofstream out(outname, ios::binary|ios::out);
	Write4(out, UCSYMTBL_MAGIC0);	// Start with symbol table.
	Write4(out, UCSYMTBL_MAGIC1);
	std::vector<Uc_function *>::iterator it;
	Usecode_symbol_table *symtbl = new Usecode_symbol_table;
	for (it = functions.begin(); it != functions.end(); it++)
		{
		Uc_function *fun = *it;
		Usecode_symbol::Symbol_kind kind = Usecode_symbol::fun_defined;
		// For now, all externs have their ID given.
		if (fun->is_externed())
			kind = Usecode_symbol::fun_extern_defined;
		symtbl->add_sym(new Usecode_symbol(fun->get_name(),
					kind, fun->get_usecode_num()));
		}
	symtbl->write(out);
	delete symtbl;
	for (it = functions.begin(); it != functions.end(); it++)
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
