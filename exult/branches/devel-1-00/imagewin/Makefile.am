INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../files $(SDL_CFLAGS) $(INCDIRS) $(WINDOWING_SYSTEM) \
		$(DEBUG_FLAGS) $(CPPFLAGS)

noinst_LTLIBRARIES = libimagewin.la

libimagewin_la_SOURCES = \
	ibuf16.cc \
	ibuf16.h \
	ibuf8.cc \
	ibuf8.h \
	imagebuf.cc \
	imagebuf.h \
	imagescl.cc \
	imagewin.cc \
	imagewin.h \
	iwin8.cc \
	iwin8.h \
	scale.h \
	savepcx.cc

EXTRA_DIST = \
	scale.cc
