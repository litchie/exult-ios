AUTOMAKE_OPTIONS=foreign

all-local:	libaudio.a 

INCLUDES +=	$(SDL_INCLUDES)
INCLUDES +=	$(KMID_INCLUDES)

CXXFLAGS += $(INCDIRS) -Wall 

#LIBS +=  -L/usr/X11R6/lib/ -lc -lstdc++ -lgcc -lm -lXi -lXext -lX11 


OFILES=	Mixer.o Midi.o Audio.o Flex.o IFF.o SDL_mapping.o

libaudio.a:	$(OFILES)
	ar rs libaudio.a $(OFILES)

clean:
	rm -f libaudio.a core $(OFILES)

tar:
	rm -rf /tmp/audio
	mkdir /tmp/audio
	cp Audio.cc Audio.h Flex.cc Flex.h Midi.cc Midi.h Mixer.cc Mixer.h SDL_mapping.h Makefile /tmp/audio
	cd /tmp && tar czvpf exult_audio_lib.tar.gz audio


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

# Dependencies

Audio.o: Audio.cc Audio.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h Midi.h
Flex.o: Flex.cc Flex.h ../autoconfig.h common.h
IFF.o: IFF.cc IFF.h ../autoconfig.h common.h
Midi.o: Midi.cc Midi.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h
Mixer.o: Mixer.cc Mixer.h ../autoconfig.h SDL_mapping.h Flex.h common.h Audio.h Midi.h
SDL_mapping.o: SDL_mapping.cc SDL_mapping.h ../autoconfig.h
test.o: test.cc Audio.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h Midi.h
