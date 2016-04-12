/*
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

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
typedef int64 sint64;
#  if (SIZEOF_INT == SIZEOF_INTP)
typedef sint32 sintptr;
typedef uint32 uintptr;
#  elif (SIZEOF_LONG == SIZEOF_INTP)
typedef   signed long sintptr;
typedef unsigned long uintptr;
#  elif (SIZEOF_LONG_LONG == SIZEOF_INTP)
typedef   signed long long sintptr;
typedef unsigned long long uintptr;
#  else
#    error "Size of pointer type not equal to int, long or long long"
#  endif

#else
//
// {s,u}int{8,16,32,ptr}
//

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

// Note: u?int{8,16,32,64,ptr}_t types from <stdint.h> are mandatory in C99, but
// not in C++. So try to infer integral types anyway.
#ifndef EX_TYPE_INT8
#  define EX_TYPE_INT8 char /* guaranteed by ISO */
#endif

#ifndef EX_TYPE_INT16
#  if (SIZEOF_SHORT == 2)
#    define EX_TYPE_INT16 short
#  elif (SIZEOF_INT == 2)
#    define EX_TYPE_INT16 int
#  else
#    error "Please make sure a 16 bit type is provided by common_types.h"
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
#    error "Please make sure a 32 bit type is provided by common_types.h"
#  endif
#endif /* !EX_TYPE_INT32 */

#ifndef EX_TYPE_INT64
#  if (SIZEOF_LONG == 8)
#    define EX_TYPE_INT64 long
#  elif (SIZEOF_LONG_LONG == 8)
#    define EX_TYPE_INT64 long long
#  else
#    error "Please make sure a 64 bit type is provided by common_types.h"
#  endif
#endif /* !EX_TYPE_INT32 */

#ifndef EX_TYPE_INTPTR
#  if (SIZEOF_INT == SIZEOF_INTP)
#    define EX_TYPE_INTPTR int
#  elif (SIZEOF_LONG == SIZEOF_INTP)
#    define EX_TYPE_INTPTR long
#  elif (SIZEOF_LONG_LONG == SIZEOF_INTP)
#    define EX_TYPE_INTPTR long long
#  else
#    error "Size of pointer type not equal to int, long or long long"
#  endif
#endif /* !EX_TYPE_INTPTR */

#ifdef HAVE_UINT8_T
typedef uint8_t  uint8;
#else
typedef unsigned EX_TYPE_INT8   uint8;
#endif

#ifdef HAVE_UINT16_T
typedef uint16_t uint16;
#else
typedef unsigned EX_TYPE_INT16  uint16;
#endif

#ifdef HAVE_UINT32_T
typedef uint32_t uint32;
#else
typedef unsigned EX_TYPE_INT32  uint32;
#endif

#ifdef HAVE_UINT64_T
typedef uint64_t uint64;
#else
typedef unsigned EX_TYPE_INT64  uint64;
#endif

#ifdef HAVE_UINTPTR_T
typedef uintptr_t uintptr;
#else
typedef unsigned EX_TYPE_INTPTR uintptr;
#endif

#ifdef HAVE_INT8_T
typedef int8_t  sint8;
#else
typedef signed EX_TYPE_INT8   sint8;
#endif

#ifdef HAVE_INT16_T
typedef int16_t sint16;
#else
typedef signed EX_TYPE_INT16  sint16;
#endif

#ifdef HAVE_INT32_T
typedef int32_t sint32;
#else
typedef signed EX_TYPE_INT32  sint32;
#endif

#ifdef HAVE_INT64_T
typedef int64_t sint64;
#else
typedef signed EX_TYPE_INT64  sint64;
#endif

#ifdef HAVE_INTPTR_T
typedef intptr_t sintptr;
#else
typedef signed EX_TYPE_INTPTR sintptr;
#endif

#endif //BeOS

/*
 * Empty string
 */
extern const std::string c_empty_string;


// Debug
#ifdef DEBUG
#  define COUT(x)       do { std::cout << x << std::endl; std::cout.flush(); } while (0)
#  define CERR(x)       do { std::cerr << x << std::endl; std::cerr.flush(); } while (0)
#else
#  define COUT(x)       do { } while(0)
#  define CERR(x)       do { } while(0)
#endif

// Two very useful macros that one should use instead of pure delete; they will additionally
// set the old object pointer to 0, thus helping prevent double deletes (not that "delete 0"
// is a no-op.
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)
#define FORGET_ARRAY(x) do { delete [] x; x = 0; } while(0)


#endif

