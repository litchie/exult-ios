/**
 **	Ucloc.h - Source location.
 **
 **	Written: 1/0/01 - JSF
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

#ifndef INCL_UCLOC
#define INCL_UCLOC	1

#include <vector>

/*
 *	Location in source code.
 */
class Uc_location
	{
	static std::vector<char *> source_names;// All filenames.
	static char *cur_source;	// Source filename.
	static int cur_line;		// Line #.
	static int num_errors;		// Total #.
	char *source;
	int line;
public:
	Uc_location()			// Use current location.
		: source(cur_source), line(cur_line)
		{  }
	static void set_cur(const char *s, int l);
	static void increment_cur_line()
		{ cur_line++; }
	const int get_line()
		{ return line; }
	const char *get_source()
		{ return source; }
	void error(char *s);		// Print error.
	static void yyerror(char *s);	// Print error at cur. location.
	static int get_num_errors()
		{ return num_errors; }
	};

#endif
