//
// MSVC Kludges... and precompiled header for Exult
//
// DO NOT INCLUDE
//
// This files allows Exult, and it's tools, to compile under the very non
// standards compilent MSVC. It also serves as a precompiled header to greatly
// improve compile times in MSVC.
//

#ifndef MSVC_KLUDGES_H
#define MSVC_KLUDGES_H

// Fisrtly some things that need to be defined
#define VERSION "0.93alpha4"
#define EXULT_DATADIR "data/"
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define DONT_HAVE_HASH_SET
#define DONT_HAVE_HASH_MAP
#define MSVC_FIND_NEARBY_KLUDGE

// Don't need everything in the windows headers
#define WIN32_LEAN_AND_MEAN

// Disable some warnings
#pragma warning (disable: 4786)	// Debug Len > 255
#pragma warning (disable: 4355)	// 'this' : used in base member initializer list
#pragma warning (disable: 4101) // unreferenced local variable

// Define size_t, but don't define tm, we'll define that later
#define _TM_DEFINED
#include <wctype.h>
#undef _TM_DEFINED

#define _SIZE_T_DEFINED

// we can't put cstdio into std because MSVC MUST have it in the global namespace
#include <cstdio>

// Now put the headers that should be in std into the std namespace
namespace std {

	#include <wchar.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <errno.h>
	#include <ctype.h>
	#include <signal.h>
	#include <cstdlib>
	#include <cstring>
	#include <ctime>
	#include <cmath>
	#include <cstdarg>

	// Kludge to make Exult think that size_t has been put into the
	// std namespace
	typedef ::size_t size_t2;
	#define size_t size_t2

	// These are declared/defined in <cstdio> and the need to be in 
	// the std namespace
	using ::FILE;
	using ::fread;
	using ::va_list;
	using ::fopen;
	using ::fclose;
	using ::fwrite;
	using ::remove;
	
	// Win32 doesn't have snprintf as such. It's got _snprintf, 
	// but it's in stdio. I'll make my own using _vsnprintf 
#if 1
	inline int snprintf(char *out, size_t len, const char *format, ...)
	{
		va_list	argptr;
		va_start (argptr, format);
		int ret = ::_vsnprintf (out, len, format, argptr);
		va_end (argptr);
		return ret;
	}
#else

#define snprintf _snprintf
	using ::snprintf;

#endif

}

// We've got snprintf
#define HAVE_SNPRINTF

// These get put in std when they otherwise should be, or are required by other headers
using std::memcmp;
using std::mbstate_t;
using std::size_t;
using std::time_t;
using std::_fsize_t;
using std::tm;
#define stat std::stat

// Some often used headers that could be included in out precompiled header
#include <fstream>
#include <exception>
#include <vector>
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <assert.h>
#include <fcntl.h>
#include <direct.h>

// Only include these headers if we are actually compiling Exult
#ifdef EXULT
#include <windows.h>
#include <mmsystem.h>

#include "exult_constants.h"
#include "ucmachine.h"
#include "flags.h"
#include "game.h"
#include "rect.h"
#include "gamewin.h"
#include "objs.h"
#include "actors.h"
#include "mouse.h"
#include "gump.h"
#include "barge.h"
#include "text_gump.h"
#include "audio.h"
#include "midi.h"
#include "SDL.h"
#endif

// Why oh why!
// MSVC thinks near and far are actually supposed to be used with pointers
// but because they are no longer used, it consequently causes an error
#undef near
#define near ex_near
#undef far
#define far ex_far

// We need this defined
#define __STRING(x) #x

// When doing a DEBUG compile we will output to the console
// However, SDL doesn't want us to do that
#ifdef DEBUG
#define SDL_main main
#endif

// Don't want SDL Parachute
#define NO_SDL_PARACHUTE

#define CHUNK_OBJ_DUMP

#endif /* !MSVC_KLUDGES_H */


