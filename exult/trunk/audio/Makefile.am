INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../imagewin -I$(srcdir)/../shapes \
		-I$(srcdir)/../objs -I$(srcdir)/../files -I$(srcdir)/../gumps \
		-I$(srcdir)/../conf \
 $(SDL_CFLAGS) $(KMID_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS) -I../files -I../conf

SUBDIRS = midi_drivers

noinst_LTLIBRARIES = libaudio.la

libaudio_la_SOURCES =	\
	Audio.cc	\
	Audio.h		\
	Midi.cc		\
	Midi.h		\
	conv.cc		\
	conv.h		\
	xmidi.cc	\
	xmidi.h		\
	soundtest.cc	\
	soundtest.h

EXTRA_DIST = 		\
	README

CLEANFILES = *~
