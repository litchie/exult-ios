/*
Copyright (C) 2001 The Exult Team

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

#ifndef OBJLIST_H
#define OBJLIST_H

#include "shapeid.h"

template<class T>
class T_Object_iterator;
template<class T, class L>
class T_Flat_object_iterator;
template<class T, class L>
class T_Object_iterator_backwards;

/*
 *	A list of objects chained together with the 'next' and 'prev'
 *	fields:
 */
template<class T>
class T_Object_list
	{
	friend class T_Object_iterator<T>;
	friend class T_Flat_object_iterator<T, class L>;
	friend class T_Object_iterator_backwards<T, class L>;

	T first;		// ->first in (circular) chain.
	unsigned short iter_count;	// # of iterators.
public:
	T_Object_list(T f = 0) : first(f), iter_count(0)
		{  }
					// Delete the chain.
	~T_Object_list()
		{
		if (!first)
			return;
		T objects = first;
		T obj;
		do
			{
			obj = objects;
			objects = obj->next;
			delete obj;
			}
		while (obj != first);
		}
					// Report iterator problem.
	void report_problem() const
		{
			std::cerr << "Danger! Danger! Object list modified while being iterated." << std::endl;
			std::cerr.flush();
		}
	int is_empty() const
		{ return first == 0; }
	void add_iterator()
		{ iter_count++; }
	void remove_iterator()
		{ iter_count--; }
	T get_first() const
		{ return first; }
					// Insert at head of chain.
	void insert(T nobj)
		{
		if (iter_count)
			report_problem();
		if (!first)		// First one.
			nobj->next = nobj->prev = nobj;
		else
			{
			nobj->next = first;
			nobj->prev = first->prev;
			first->prev->next = nobj;
			first->prev = nobj;
			}
		first = nobj;
		}
					// Insert before given obj.
	void insert_before(T nobj, T before)
		{
		if (iter_count)
			report_problem();
		nobj->next = before;
		nobj->prev = before->prev;
		before->prev->next = nobj;
		before->prev = nobj;
		first = before == first ? nobj : first;
		}
					// Append.
	void append(T nobj)
		{ insert(nobj); first = nobj->next; }
	void remove(T dobj)
		{
		if (iter_count)
			report_problem();
		if (dobj == first)
			first = dobj->next != first ? dobj->next : 0;
		dobj->next->prev = dobj->prev;
		dobj->prev->next = dobj->next;
		}
	};


class Game_object;
typedef T_Object_list<Game_object *> Object_list;


#endif
