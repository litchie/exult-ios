/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// istring.cpp -- case insensitive stl strings

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "istring.h"

namespace Pentagram {

int strncasecmp(const char *s1, const char *s2, uint32 length) {
	uint32  c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!length--)
			return 0;       // strings are equal until end point

		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return (c1 < c2) ? -1 : 1; // strings not equal
		}
	} while (c1);

	return 0;       // strings are equal
}

int strcasecmp(const char *s1, const char *s2) {
	return strncasecmp(s1, s2, 2147483647);
}

bool ichar_traits::eq(const char_type &c1, const char_type &c2) {
	uint32 c3 = c1;
	uint32 c4 = c2;
	if (c3 >= 'a' && c3 <= 'z')
		c3 -= ('a' - 'A');
	if (c4 >= 'a' && c4 <= 'z')
		c4 -= ('a' - 'A');
	return c3 == c4;
}

bool ichar_traits::lt(const char_type &c1, const char_type &c2) {
	uint32 c3 = c1;
	uint32 c4 = c2;
	if (c3 >= 'a' && c3 <= 'z')
		c3 -= ('a' - 'A');
	if (c4 >= 'a' && c4 <= 'z')
		c4 -= ('a' - 'A');
	return c3 < c4;
}

int ichar_traits::compare(const char_type *s1, const char_type *s2, size_t length) {
	return strncasecmp(s1, s2, length);
}

}

