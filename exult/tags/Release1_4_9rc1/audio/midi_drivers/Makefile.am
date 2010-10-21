INCLUDES = -I$(srcdir)/../../headers -I$(srcdir)/../../conf -I$(srcdir)/../.. \
		-I$(srcdir)/.. -I$(srcdir)/../../files -I$(srcdir)/../../imagewin \
		-I../../conf -I../../files -I../../objs -I$(srcdir)/../../shapes \
		$(SDL_CFLAGS) $(KMID_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
		$(DEBUG_FLAGS) $(CPPFLAGS)

if BUILD_MT32EMU
MT32EMUDIR=mt32emu
MT32EMUDRV=	\
	MT32EmuMidiDriver.cpp	\
	MT32EmuMidiDriver.h
endif

SUBDIRS = timidity $(MT32EMUDIR)

noinst_LTLIBRARIES = libmididrv.la

libmididrv_la_SOURCES =	\
	ALSAMidiDriver.cpp	\
	ALSAMidiDriver.h	\
	amiga_midi.cc		\
	amiga_midi.h		\
	be_midi.cc		\
	be_midi.h		\
	CoreAudioMidiDriver.cpp	\
	CoreAudioMidiDriver.h	\
	CoreMidiDriver.cpp \
	CoreMidiDriver.h \
	FileMidiDriver.cpp	\
	FileMidiDriver.h	\
	fmopl.cpp		\
	fmopl.h			\
	FMOplMidiDriver.cpp	\
	FMOplMidiDriver.h	\
	forked_player.cc	\
	forked_player.h		\
	KMIDI.cc		\
	KMIDI.h			\
	LowLevelMidiDriver.cpp	\
	LowLevelMidiDriver.h	\
	MidiDriver.cpp		\
	MidiDriver.h		\
	mixer_midiout.cc	\
	mixer_midiout.h		\
	$(MT32EMUDRV)	\
	TimidityMidiDriver.cpp	\
	TimidityMidiDriver.h	\
	UnixSeqMidiDriver.cpp	\
	UnixSeqMidiDriver.h	\
	WindowsMidiDriver.cpp	\
	WindowsMidiDriver.h	\
	XMidiEvent.h		\
	XMidiEventList.cpp	\
	XMidiEventList.h	\
	XMidiFile.cpp		\
	XMidiFile.h		\
	XMidiNoteStack.h	\
	XMidiSequence.cpp	\
	XMidiSequence.h		\
	XMidiSequenceHandler.h	\
	FluidSynthMidiDriver.cpp	\
	FluidSynthMidiDriver.h

EXTRA_DIST = 		\
	README

CLEANFILES = *~
