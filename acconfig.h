/* Do we have a version of libkmidi that we can compile against? */
#undef HAVE_LIBKMIDI

/* Do we have a version of freetype that we can compile against? */
#undef HAVE_FREETYPE
      
#undef PACKAGE
#undef VERSION

/* Was debugging selected from configure? */
#undef DEBUG

/* Do we appear to have a runnable copy of Timidity in our path? */
#undef HAVE_TIMIDITY_BIN

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

@BOTTOM@

#ifdef ALPHA_LINUX_CXX
#include "alpha_kludges.h"
#endif
