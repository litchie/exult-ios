/**
 **	Ucloc.cc - Source location.
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

#include <iostream.h>
#include "ucloc.h"

set<char *> Uc_location::source_names;
char *Uc_location::cur_source = 0;
int Uc_location::cur_line = 0;

/*
 *	Print error for stored position.
 */

void Uc_location::error
	(
	char *s
	)
	{
	cout << source << ':' << line + 1 << ": " << s << endl;
	}

/*
 *	Print error for current parse location.
 */

void Uc_location::yyerror
	(
	char *s
	)
	{
	cout << cur_source << ':' << cur_line + 1 << ": " << s << endl;
	}

