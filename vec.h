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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef VEC_H
#define VEC_H

#include	<vector>

template <class T>
class Exult_vector : public std::vector<T>
{
private:
	typedef	std::vector<T>	baseClass;
public:
	typedef typename		baseClass::size_type       size_type;
	typedef typename		baseClass::iterator		   iterator;

	Exult_vector<T>() : baseClass()
		{}
	Exult_vector<T>(size_type n) : baseClass()
		{ reserve(n); }

#ifndef MACOS /* should be something like PROPER_STD_CPP_LIB or so */
	T& at(int i) { return (*this)[i]; }
#endif
	void put(int i, T& v)		// Set i'th entry.
		{
			if (i >= (int)this->size())
				{
				insert(this->begin() + this->size(), i - this->size(), 0);
				push_back(v);
				}
			else
				(*this)[i] = v;
		}
	int put(T& v)			// Put in a free spot & return it.
		{
			int i = find(0);
			if (i < 0)
				i = this->size();
			put(i, v);
			return (i);
		}
	size_type	find( const T& obj ) const
		{
			size_type pos = 0;
			for (const T *X = &*this->begin(); X != &*this->end(); ++X, ++pos)
				{
				if( *X == obj )
					return pos;
				}
			return -1;
		}

	size_type	append( const T& obj )
		{
			push_back( obj );
			return this->size() - 1;
		}

	void		remove( const T& obj )
		{
			// Correct way. An iterator isn't a pointer, necessarily
			for(iterator X = this->begin(); X != this->end(); ++X)
			{
				if( *X == obj )
				{
					erase(X);
					return;
				}
			}
		}
};


class	Game_object;
class	Egg_object;
class	Actor;

typedef	Exult_vector<Game_object*>	Game_object_vector;
typedef	Exult_vector<Egg_object*>	Egg_vector;
typedef	Exult_vector<Actor*>		Actor_vector;


#endif
