INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../imagewin -I$(srcdir)/../shapes \
		-I$(srcdir)/../pathfinder -I$(srcdir)/../files \
		-I$(srcdir)/../audio -I$(srcdir)/../usecode $(SDL_INCLUDES) \
		$(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

noinst_LTLIBRARIES = libobjs.la

libobjs_la_SOURCES = \
	animate.cc \
	animate.h \
	barge.cc \
	barge.h \
	chunks.cc \
	chunks.h \
	citerate.h \
	contain.cc \
	contain.h \
	egg.cc \
	egg.h \
	flags.h \
	iregobjs.cc \
	iregobjs.h \
	objiter.h \
	objlist.h \
	objs.cc \
	objs.h \
	ordinfo.h \
	spellbook.cc \
	spellbook.h \
	virstone.cc \
	virstone.h

CLEANFILES = *~
