/**
 **	Objiter.h - Game objects iterator.
 **
 **	Written: 7/29/2000 - JSF
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

#ifndef INCL_OBJITER
#define INCL_OBJITER	1

#include "objlist.h"

class	Map_chunk;
class	Game_object;

/*
 *	Want to make all object iterators modification-safe.  For now, just
 *	trying to detect modification during iteration.
 */
template<class T>
class T_Safe_object_iterator
	{
	T_Object_list<T>& list;
public:
	T_Safe_object_iterator(T_Object_list<T>& l) : list(l)
		{ list.add_iterator(); }
	~T_Safe_object_iterator()
		{ list.remove_iterator(); }
	};

/*
 *	Iterate through list of objects.
 */
template<class T>
class T_Object_iterator : public T_Safe_object_iterator<T>
	{
protected:
	T first;
	T stop;
	T cur;		// Next to return.
public:
	void reset()
		{ cur = first; stop = 0; }
	T_Object_iterator(T_Object_list<T>& objects) 
		: T_Safe_object_iterator<T>(objects), first(objects.get_first())
		{ reset(); }
	T get_next()
		{
		if (cur == stop)
			return 0;
		T ret = cur;
		cur = cur->next;
		stop = first;
		return ret;
		}
	};

typedef T_Object_iterator<Game_object *> Object_iterator;

/*
 *	Iterate through a chunk's nonflat objects.
 */
template<class T, class L>
class T_Nonflat_object_iterator : public T_Object_iterator<T>
	{
	T nonflats;
public:
	void reset()
		{ cur = nonflats; stop = 0; }
	T_Nonflat_object_iterator(L chunk)
		: T_Object_iterator<T>(chunk->get_objects()), nonflats(chunk->get_first_nonflat())
		{ reset(); }
	};

typedef T_Nonflat_object_iterator<Game_object *, Map_chunk *> Nonflat_object_iterator;

/*
 *	Iterate through a chunk's flat objects.
 */
template<class T, class L>
class T_Flat_object_iterator : public T_Safe_object_iterator<T>
	{
	T first;
	T stop;
	T cur;		// Next to return.
	T stop_at;
public:
	void reset()
		{ cur = first; stop = 0; }
	T_Flat_object_iterator(L chunk)
		: T_Safe_object_iterator<T>(chunk->get_objects())
		{
		first = chunk->get_objects().get_first() == chunk->get_first_nonflat() ? 0 :
							chunk->get_objects().get_first();
		stop_at = chunk->get_first_nonflat() ? chunk->get_first_nonflat()
						: chunk->get_objects().get_first();
		reset();
		}
	T get_next()
		{
		if (cur == stop)
			return 0;
		T ret = cur;
		cur = cur->get_next();
		stop = stop_at;
		return ret;
		}
	};

typedef T_Flat_object_iterator<Game_object *, Map_chunk *> Flat_object_iterator;

/*
 *	Iterate backwards through list of objects.
 */
template<class T, class L>
class T_Object_iterator_backwards : public T_Safe_object_iterator<T>
	{
	T first;
	T stop;
	T cur;		// Return prev. to this.
public:
	void reset()
		{ cur = first; stop = 0; }
	T_Object_iterator_backwards(L chunk) 
		: T_Safe_object_iterator<T>(chunk->get_objects()),
		  first(chunk->get_objects().get_first())
		{ reset(); }
	T_Object_iterator_backwards(T_Object_list<T>& objects) 
		: T_Safe_object_iterator<T>(objects),
		  first(objects.get_first())
		{ reset(); }
	T get_next()
		{
		if (cur == stop)
			return 0;
		cur = cur->prev;
		stop = first;
		return cur;
		}
	};

typedef T_Object_iterator_backwards<Game_object *, Map_chunk *> Object_iterator_backwards;

/*
 *	Iterate through a list of objects (recursively).
 */
template<class D> class D_Recursive_object_iterator
	{
					// Child we're going through, or 0.
	D_Recursive_object_iterator<class D> *child;
	D elems;			// Goes through our elements.
public:
	D_Recursive_object_iterator(Object_list& objs)
		: elems(objs), child(0)
		{  }
	Game_object *get_next();	// Get next, going into containers.
	};

/*
 *	Iterate forwards/backwards through a list of objects (recursively).
 */
typedef D_Recursive_object_iterator<Object_iterator_backwards> 
					Recursive_object_iterator_backwards;
typedef D_Recursive_object_iterator<Object_iterator> 
					Recursive_object_iterator;

#endif
