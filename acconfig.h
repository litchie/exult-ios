/* Do we have a version of libkmidi that we can compile against? */
#undef HAVE_LIBKMIDI

/* Do we have a version of freetype that we can compile against? */
#undef HAVE_FREETYPE
      
#undef PACKAGE
#undef VERSION
#undef VER_MAJOR
#undef VER_MINOR
#undef VER_EXTRA

/* Was debugging selected from configure? */
#undef DEBUG

/* Do we appear to have a runnable copy of Timidity in our path? */
#undef HAVE_TIMIDITY_BIN

/* Enable support for communication with Exult Studio? */
#undef USE_EXULTSTUDIO

/* Using alternative memory scheme? */
#undef WANT_ALTERNATE_ALLOCATOR

/* Set memory to zero before usage? */
#undef INITIALISE_ALLOCATED_BLOCKS

/* Set memory to a non-zero pattern before usage? */
#undef POISON_ALLOCATED_BLOCKS

/* Define if the libraries have mkstemp() */
#undef HAVE_MKSTEMP

/* disable SDL parachute? */
#undef NO_SDL_PARACHUTE

/* have GIMP devel environment */
#undef HAVE_GIMP

/* Compiling with CXX on Alpha */
#undef ALPHA_LINUX_CXX

/* Compiling on OpenBSD */
#undef OPENBSD

/* Compiling on MacOSX */
#undef MACOSX

/* Compiling in Cygwin */
#undef CYGWIN

/* getaddrinfo() available? */
#undef HAVE_GETADDRINFO

/* Some platforms don't have snprintf */
#undef HAVE_SNPRINTF

/* Do we have gtk development files? */
#undef HAVE_GTK

/* Is freetype2 available? */
#undef HAVE_FREETYPE2

/* Do we have zlib? (for compressed savegames) */
#undef HAVE_ZIP_SUPPORT

@BOTTOM@

#ifdef ALPHA_LINUX_CXX
#include "alpha_kludges.h"
#endif
