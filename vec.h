/**
 **	Vec.h - A resizeable vector.
 **
 **	Written: 4/5/99 - JSF
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

#ifndef INCL_VEC
#define INCL_VEC 1

#include	<vector>

template <class T>
class FeatureVector : public std::vector<T>
{
private:
	typedef	std::vector<T>	baseClass;
	typedef typename		baseClass::size_type       size_type;
public:
	FeatureVector<T>() : baseClass()
		{}
	FeatureVector<T>(size_type n) : baseClass()
		{ reserve(n); }

	T& at(int i) { return (*this)[i]; }
	void put(int i, T& v)		// Set i'th entry.
		{
		if (i >= size())
			{
			T *iter = begin() + size();
			insert(begin() + size(), i - size(), 0);
			push_back(v);
			}
		else
			(*this)[i] = v;
		}
	int put(T& v)			// Put in a free spot & return it.
		{
		int i = find(0);
		if (i < 0)
			i = size();
		put(i, v);
		return (i);
		}
	size_type	find( const T obj ) const
		{
			size_type pos = 0;
			for (const T *X = begin(); X != end(); ++X, ++pos)
			{
				if( *X == obj )
					return pos;
			}
			return -1;
		}

	size_type	append( T const obj )
		{
			push_back( obj );
			return size() - 1;
		}

	void		remove( const T obj )
		{
			for(T *X = begin(); X != end(); ++X)
				if( *X == obj )
				{
					erase(X);
					return;
				}
		}
};


class	Game_object;
class	Egg_object;
class	Actor;

typedef	FeatureVector<Game_object*>	GOVector;
typedef	FeatureVector<Egg_object*>	EggVector;
typedef	FeatureVector<Actor*>		ActorVector;


#if 0
/*
 *	Here's a vector that resizes itself.
 */
class Vector
	{
	void **values;			// List of values.
	int cnt;			// # of values stored.
	int max;			// # of values there's room for.
	void resize(int newmax);	// Resize.
public:
	Vector(int c = 0, int m = 0);
	~Vector()
		{ delete values; }
	int get_cnt()
		{ return cnt; }
	void truncate(int newcnt)	// Reduce count.
		{
		if (newcnt < cnt)
			cnt = newcnt;
		}
	void *get(int i)		// Get i'th entry.
		{ return i >= 0 && i < cnt ? values[i] : 0; }
	int put(int i, void *v)		// Set i'th entry.  Returns 0 if error.
		{
		if (i < 0)
			return (0);
		if (i >= max)
			resize(i + 1);
		if (i >= cnt)
			cnt = i + 1;
		values[i] = v;
		return (1);
		}
	int append(void *v)		// Append entry, and return index.
		{
		int i = cnt;
		put(i, v);
		return (i);
		}
	int find(void *v);		// Find an entry.
	int remove(void *v)		// Remove an entry.
		{
		int i = find(v);
		if (i >= 0)
			values[i] = 0;
		return i;
		}
	int put(void *v)		// Put in a free spot & return it.
		{
		int i = find(0);
		if (i < 0)
			i = cnt;
		put(i, v);
		return (i);
		}
	};
#endif

#endif
