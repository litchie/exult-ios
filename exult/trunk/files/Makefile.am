INCLUDES = -I$(srcdir)/.. $(SDL_CFLAGS) $(INCDIRS) $(WINDOWING_SYSTEM) \
		$(DEBUG_FLAGS) $(CPPFLAGS)

noinst_LTLIBRARIES = libu7file.la

if HAVE_SNPRINTF
SNPRINTF =
else
SNPRINTF = snprintf.cc
endif

libu7file_la_SOURCES =	\
	Flat.cc		\
	Flat.h		\
	Flex.cc		\
	Flex.h		\
	Table.cc	\
	Table.h		\
	IFF.cc		\
	IFF.h		\
	U7file.cc	\
	U7file.h	\
	utils.cc	\
	utils.h		\
	databuf.h	\
	listfiles.cc	\
	listfiles.h	\
	$(SNPRINTF)

EXTRA_DIST = 		\
	README

CLEANFILES = *~
