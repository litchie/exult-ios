/* Do we have a version of SDL that we can compile against? */
#undef HAVE_SDL

/* Do we have a version of libkmidi that we can compile against? */
#undef HAVE_LIBKMIDI

/* Do we have a version of freetype that we can compile against? */
#undef HAVE_FREETYPE
      
#undef PACKAGE
#undef VERSION

/* Was debugging selected from configure? */
#undef DEBUG

/* Multiple inclusion preventer. Is there a cleaner way to do this in autconf files? */
#undef AUTOCONFIGURED


/* Do we appear to have a runnable copy of Timidity in our path? */
#undef HAVE_TIMIDITY_BIN

/* Using alternative memory scheme? */
#undef WANT_ALTERNATE_ALLOCATOR

/* Set memoryu before usage? */
#undef INITIALISE_ALLOCATED_BLOCKS

/* Define if the libraries have mkstemp() */
#undef HAVE_MKSTEMP

