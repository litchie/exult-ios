/*
 * alpha_kludges.h Copyright (C) 2000 The Exult Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/* System-specific kludges */
/* OK, right now AXP-Linux-cxx specific kludges only */


#ifndef ALPHA_KLUDGES_H
#define ALPHA_KLUDGES_H

#ifndef HAVE_HASH_MAP
#  define DONT_HAVE_HASH_MAP
#endif
#ifndef HAVE_HASH_SET
#  define DONT_HAVE_HASH_SET
#endif


/*------------------------*/
/* DEC cxx on Alpha/Linux */
/*------------------------*/
#if defined(__DECCXX) && defined(__linux__)

#undef HAVE_HASH_MAP
#undef HAVE_HASH_SET
#undef DONT_HAVE_HASH_MAP
#undef DONT_HAVE_HASH_SET
#define DONT_HAVE_HASH_SET
#define DONT_HAVE_HASH_MAP

namespace std {
#include <sys/types.h>
}

typedef std::size_t size_t;

namespace std {
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
}
#include <iomanip>
#include <fstream>
#include <assert.h>

using std::FILE;
//using std::iostream;


#include <iostream>
#include <vector>

namespace std{
	using ::ostream;
	using ::istream;
	using ::ofstream;
	using ::ifstream;
	using ::cout;
	using ::cin;
	using ::cerr;
	using ::clog;
	using ::endl;
	using ::ios;
	using ::vector;
	using ::dec;
	using ::hex;
	using ::setfill;
	using ::setw;
};


//using std::strcmp;
//using std::FILE;

#else
#error "alpha_kludges.h should not have been included!"
#endif /* !__DECCXX && Linux*/

#endif /* !ALPHA_KLUDGES_H */


