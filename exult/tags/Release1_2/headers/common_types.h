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

#ifndef	COMMON_TYPES_H
#define	COMMON_TYPES_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string>

#ifdef BEOS
// BeOS headers already define these types
#include <be/support/SupportDefs.h>
typedef int8 sint8;
typedef int16 sint16;
typedef int32 sint32;

#else

#ifndef EX_TYPE_INT8
#  define EX_TYPE_INT8 char /* guaranteed by ISO */
#endif

#ifndef EX_TYPE_INT16
#  if (SIZEOF_SHORT == 2)
#    define EX_TYPE_INT16 short
#  elif (SIZEOF_INT == 2)
#    define EX_TYPE_INT16 int
#  else
#    error "Please make sure a 16 bit type is provided by exult_types.h"
#  endif
#endif /* !EX_TYPE_INT16 */


#ifndef EX_TYPE_INT32
#  if (SIZEOF_INT == 4)
#    define EX_TYPE_INT32 int
#  elif (SIZEOF_LONG == 4)
#    define EX_TYPE_INT32 long
#  elif (SIZEOF_LONG_LONG == 4)
#    define EX_TYPE_INT32 long long
#  else
#    error "Please make sure a 32 bit type is provided by exult_types.h"
#  endif
#endif /* !EX_TYPE_INT32 */


typedef	unsigned EX_TYPE_INT8	uint8;
typedef	unsigned EX_TYPE_INT16	uint16;
typedef	unsigned EX_TYPE_INT32	uint32;

typedef	signed EX_TYPE_INT8		sint8;
typedef	signed EX_TYPE_INT16	sint16;
typedef	signed EX_TYPE_INT32	sint32;


#endif //BeOS

/*
 * Empty string
 */
extern const std::string c_empty_string;


// Debug
#ifdef DEBUG
#  define COUT(x)		do { std::cout << x << std::endl; std::cout.flush(); } while (0)
#  define CERR(x)		do { std::cerr << x << std::endl; std::cerr.flush(); } while (0)
#else
#  define COUT(x)		do { } while(0)
#  define CERR(x)		do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they will additionally
// set the old object pointer to 0, thus helping prevent double deletes (not that "delete 0"
// is a no-op.
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


#endif

