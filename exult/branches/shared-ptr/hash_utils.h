/*
 *  Copyright (C) 2000-2011  The Exult Team
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

#ifndef _HASH_UTILS_H_
#define _HASH_UTILS_H_

#include "exult_types.h"

#include <cstring>

#ifdef DONT_HAVE_HASH_MAP
#  include <map>
#else
#  if (defined(__GNUC__) && (__GNUC__ >= 4) && ( __GNUC_MINOR__ >= 3) && HAVE_TR1_UNORDERED_MAP)
#    include <tr1/unordered_map>
     using std::tr1::unordered_map;
#  elif HAVE_EXT_HASH_MAP
#    include <ext/hash_map>
#    define unordered_map hash_map
#    if (defined(__GNUC__) && (__GNUC__ >= 3) && ( __GNUC_MINOR__ >= 0))
       using __gnu_cxx::hash_map;
#    else
       using std::hash_map;
#    endif
#  else
#    include <hash_map>
#    define unordered_map hash_map
#  endif
#  ifdef MACOS
     using Metrowerks::hash_map;
#    define unordered_map hash_map
#  endif
#endif

#ifdef DONT_HAVE_HASH_SET
#  include <set>
#else
#  if (defined(__GNUC__) && (__GNUC__ >= 4) && ( __GNUC_MINOR__ >= 3) && HAVE_TR1_UNORDERED_SET)
#    include <tr1/unordered_set>
     using std::tr1::unordered_set;
#  elif HAVE_EXT_HASH_SET
#    include <ext/hash_set>
#    define unordered_set hash_set
#    if (defined(__GNUC__) && (__GNUC__ >= 3) && ( __GNUC_MINOR__ >= 0))
       using __gnu_cxx::hash_set;
#    else
       using std::hash_set;
#    endif
#  else
#    include <hash_set>
#    define unordered_map hash_map
#  endif
#  ifdef MACOS
	 using Metrowerks::hash_set;
	 #define unordered_set hash_set
#  endif
#endif


#if defined(DONT_HAVE_HASH_MAP) && defined(DONT_HAVE_HASH_SET)

/*
 *	For testing if a string is "less" than another:
 */
struct ltstr {
	bool operator()(const char* s1, const char* s2) const {
		return (std::strcmp(s1, s2) < 0);
	}
};

#else

/*
 *	Hash function for strings:
 */
struct hashstr
{
	long operator() (const char *str) const
	{
		const uint32 m = 4294967291u;
		uint32 result = 0;
		for (; *str != '\0'; ++str)
			result = ((result << 8) + *str) % m;
		return long(result);
	}
};

/*
 *	For testing if two strings match:
 */
struct eqstr
{
	bool operator()(const char* s1, const char* s2) const {
		return std::strcmp(s1, s2) == 0;
	}
};

#endif

#endif	/* _HASH_UTILS_H_ */
