AUTOMAKE_OPTIONS=foreign

all-local:	libaudio.a 

INCLUDES +=	$(SDL_INCLUDES)
INCLUDES +=	$(KMID_INCLUDES)
INCLUDES +=	-I../conf -I.

CXXFLAGS += $(INCDIRS) -Wall 
CXXFLAGS += $(PEDANTIC)
CXXFLAGS += $(DEBUG_FLAGS)
CXXFLAGS += $(WINDOWING_SYSTEM)

SUBDIRS = midi_drivers

#LIBS +=  -L/usr/X11R6/lib/ -lc -lstdc++ -lgcc -lm -lXi -lXext -lX11 


OFILES=	Mixer.o Midi.o Audio.o SDL_mapping.o

SOURCES = Audio.cc Audio.h Midi.cc Midi.h \
		Mixer.cc Mixer.h SDL_mapping.cc SDL_mapping.h \
		common.h test.cc \
		Flex.cc Flex.h IFF.cc IFF.h Table.cc Table.h

libaudio.a:	$(OFILES)
	ar rs libaudio.a $(OFILES) midi_drivers/*.o

u7audiotool:	libaudio.a u7audiotool.o 
	g++ -o u7audiotool u7audiotool.o -L../conf -L. -laudio -lconf  $(KMID_LIBS) $(SDL_LIBS)  -lX11 -lXext -lstdc++ -lgcc -lc -lgcc -L/usr/X11R6/lib


clean:
	rm -f libaudio.a core $(OFILES)

tar:
	rm -rf /tmp/audio
	mkdir /tmp/audio
	cp Audio.cc Audio.h Flex.cc Flex.h Midi.cc Midi.h Mixer.cc Mixer.h SDL_mapping.h Makefile /tmp/audio
	cd /tmp && tar czvpf exult_audio_lib.tar.gz audio


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

rebuild_master_dependencies:
	makedepend -f Makefile.am -Y $(CXXFLAGS) -I. -I.. -I../conf -I../audio -I../pathfinder -I../flic - -- *.cc
	cd midi_drivers && make rebuild_master_dependencies


# DO NOT DELETE

Audio.o: Audio.h ../autoconfig.h SDL_mapping.h Mixer.h Midi.h ../fnames.h
Audio.o: ../files/U7file.h ../conf/Configuration.h ../conf/XMLEntity.h
Flex.o: Flex.h ../autoconfig.h common.h
IFF.o: IFF.h ../autoconfig.h common.h
Midi.o: Midi.h SDL_mapping.h Mixer.h ../fnames.h ../files/U7file.h
Midi.o: ../conf/Configuration.h ../conf/XMLEntity.h
Midi.o: midi_drivers/Timidity_binary.h ../autoconfig.h Flex.h common.h
Midi.o: Table.h midi_drivers/KMIDI.h midi_drivers/forked_player.h
Midi.o: midi_drivers/win_MCI.h
Mixer.o: Mixer.h SDL_mapping.h Audio.h ../autoconfig.h Midi.h
SDL_mapping.o: SDL_mapping.h
Table.o: Table.h common.h
test.o: Audio.h ../autoconfig.h SDL_mapping.h Mixer.h Midi.h
test.o: ../conf/Configuration.h ../conf/XMLEntity.h Table.h common.h
u7audiotool.o: Audio.h ../autoconfig.h SDL_mapping.h Mixer.h Midi.h
u7audiotool.o: ../conf/Configuration.h ../conf/XMLEntity.h Table.h common.h
u7audiotool.o: ../fnames.h
