/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Delobjs.h - Game objects that have been removed, but need deleting.
 **
 **	Written: 5/25/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#ifndef INCL_DELOBJS
#define INCL_DELOBJS	1

#include <hash_set>

/*
 *	Hash function for game objects:
 */
class Hash_objs
	{
public:
	size_t operator() (const Game_object *a) const
		{ return (long) a; }
	};

/*
 *	For testing if two game objects match:
 */
class Equal_objs
	{
public:
     bool operator() (const Game_object *a, const Game_object *b) const
     	{
			return a == b;
		}
	};


/*
 *	A pool of removed game objects, waiting to be deleted:
 */
class Deleted_objects : public hash_set<Game_object *, Hash_objs, Equal_objs>
	{
public:
	Deleted_objects() : hash_set<Game_object *, Hash_objs, Equal_objs> (1013)
		{  }
	void flush();			// Delete them now.
	~Deleted_objects()
		{ flush(); }
	};

#endif
