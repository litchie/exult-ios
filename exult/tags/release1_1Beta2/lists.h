/*
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

#ifndef _LISTS_H_
#define _LISTS_H_

// TODO: merge lists.h and vec.h into one file?!

#include <list>
#include "exceptions.h"

class Actor;

template <class T>
class Exult_queue
{
public:
	typedef typename std::list<T>::size_type		size_type;
	typedef typename std::list<T>::const_iterator	const_iterator;

protected:
	std::list<T> data;

public:
	// exception class for illegal operations on an empty queue
	class empty_queue_exception : public exult_exception
	{
	public:
		empty_queue_exception () : exult_exception("attempt to read an empty queue") {  }
	};

	// number of elements
	size_type size() const
		{ return data.size(); }

	// queue empty?
	bool empty() const
		{ return data.empty(); }

	// insert element into the queue
	void push (const T& obj)
		{ data.push_back(obj); }

	// insert element at the front of the queue
	void push_front (const T& obj)
		{ data.push_front(obj); }

	// pop element out of the queue and return its value
	T pop ()
	{
		if (data.empty())
			throw empty_queue_exception();
		T obj(data.front());
		data.pop_front();
		return obj;
	}

	// return value of next element
	T& front ()
	{
		if (data.empty())
			throw empty_queue_exception();
		return data.back();
	}
	
	// remove an element from the queue
	void remove(const T& obj)
		{ data.remove(obj); }
	
	// clear the queue
	void clear()
		{ data.clear(); }

	// iterators:
	const_iterator         begin() const
		{ return data.begin(); }
	const_iterator         end() const
		{ return data.end(); }
};

typedef Exult_queue<Actor*> Actor_queue;


#endif
