/**
 **	Vec.cc - A resizeable vector.
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

#include <string.h>
#include "vec.h"

/*
 *	Resize.
 */

void Vector::resize
	(
	int newmax			// New max. needed.
	)
	{
	if (newmax <= 0)		// Clear it out?
		{
		cnt = max = 0;
		delete values;
		values = 0;
		}
	else
		{			// Make it a little bit bigger.
		newmax += newmax/4;
		void **newvalues = new void *[newmax];
		if (cnt)		// Copy previous values.
			memcpy(newvalues, values, cnt * sizeof(void *));
					// Clear the rest.
		memset((char *) newvalues + cnt*sizeof(void *), 0,
					(newmax - cnt) * sizeof(void *));
		max = newmax;
		delete values;
		values = newvalues;
		}
	}

/*
 *	Create a vector.
 */

Vector::Vector
	(
	int c,				// Initial count.
	int m				// Inital max.
	) : cnt(c), max(m)
	{
	if (cnt < 0)
		cnt = 0;
	if (max < cnt)
		max = cnt;
	if (!max)			// Empty?
		values = 0;
	else
		{
		values = new void *[max];
		memset(values, 0, max*sizeof(void *));
		}
	}
