INCLUDES = -I$(srcdir)/../../headers -I$(srcdir)/../../conf -I$(srcdir)/../.. -I$(srcdir)/.. -I$(srcdir)/../../files -I$(srcdir)/../../imagewin -I$(srcdir)/../../shapes $(SDL_CFLAGS) $(KMID_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS) -I../../conf -I../../files

noinst_LTLIBRARIES = libmididrv.la

libmididrv_la_SOURCES =	\
	KMIDI.cc	\
	KMIDI.h		\
	Timidity_binary.cc	\
	Timidity_binary.h	\
	forked_player.cc	\
	forked_player.h		\
	mixer_midiout.cc	\
	mixer_midiout.h		\
	be_midi.cc		\
	be_midi.h		\
	mac_midi.cc		\
	mac_midi.h		\
	amiga_midi.cc		\
	amiga_midi.h		\
	fmopl.cc		\
	fmopl.h			\
	fmopl_midi.cc		\
	fmopl_midi.h		\
	fmopldrv.cc		\
	fmopldrv.h

EXTRA_DIST = 		\
	README

CLEANFILES = *~
