/**
 **	Compile.h - Compile a script.  (Code is in script.y).
 **
 **	Written: 5/28/99 - JSF
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

class Npc;
class Slist_iterator;

/*
 *	The script compiler.
 */

class Script_compiler
	{
	int error;			// Set to # errors, or <0 if fatal err.
	Slist_iterator *next;
public:
					// This compiles.
	Script_compiler(char *script_name);
	~Script_compiler();
	int get_error()
		{ return error; }
					// Get next NPC and its actor info.
	int operator()(Npc *& npc, int& shapeid, int& portraitid,
			int& cx, int& cy, int& sx, int& sy);
	};
