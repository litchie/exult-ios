/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Delobjs.h - Game objects that have been removed, but need deleting.
 **
 **	Written: 5/25/2000 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

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

#ifndef DELOBJS_H
#define DELOBJS_H	1

#ifdef DONT_HAVE_HASH_SET
#  include <set>
#else
#  include <hash_set>
#  ifdef MACOS
	using Metrowerks::hash_set;
#  endif
#endif

#ifdef DONT_HAVE_HASH_SET

/*
 *	"Less than" relation for objects
 */
class Less_objs
	{
public:
     bool operator() (const Game_object *a, const Game_object *b) const
     	{
			return a < b;
		}
	};

#else

/*
 *	Hash function for game objects:
 */
class Hash_objs
	{
public:
	std::size_t operator() (const Game_object *a) const
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

#endif


/*
 *	A pool of removed game objects, waiting to be deleted:
 */
#ifndef DONT_HAVE_HASH_SET
class Deleted_objects : public hash_set<Game_object *, Hash_objs, Equal_objs>
#else
class Deleted_objects : public std::set<Game_object *, Less_objs>
#endif
	{
public:
#ifndef DONT_HAVE_HASH_SET
	Deleted_objects() : hash_set<Game_object *, Hash_objs, Equal_objs> (1013)
#else
	Deleted_objects() : std::set<Game_object *, Less_objs> ()
#endif
		{  }

	void flush();			// Delete them now.
	~Deleted_objects()
		{ flush(); }
	};

#endif	/* DELOBJS_H */
