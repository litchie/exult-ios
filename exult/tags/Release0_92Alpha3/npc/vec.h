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
	};

#endif
