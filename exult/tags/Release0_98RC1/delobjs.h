/*
 *	delobjs.h - Game objects that have been removed, but need deleting.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DELOBJS_H
#define DELOBJS_H	1

#include <set>

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


/*
 *	A pool of removed game objects, waiting to be deleted:
 */
class Deleted_objects : public std::set<Game_object *, Less_objs>
	{
public:
	Deleted_objects() : std::set<Game_object *, Less_objs> ()
		{  }

	void flush();			// Delete them now.
	~Deleted_objects()
// Problems		{ flush(); }
		{  }
	};

#endif	/* DELOBJS_H */
