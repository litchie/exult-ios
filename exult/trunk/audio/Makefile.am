INCLUDES = -I.. -I../imagewin -I../shapes -I../files $(SDL_INCLUDES) $(KMID_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS) -I../files -I../conf

SUBDIRS = midi_drivers

noinst_LTLIBRARIES = libaudio.la

libaudio_la_SOURCES =	\
	Audio.cc	\
	Audio.h		\
	Midi.cc		\
	Midi.h		\
	Mixer.cc	\
	Mixer.h		\
	SDL_mapping.cc	\
	SDL_mapping.h	\
	conv.h		\
	pcb.cc		\
	pcb.h		\
	xmidi.cc	\
	xmidi.h

EXTRA_DIST = 		\
	README
