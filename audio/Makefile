
.PHONY= tar all clean

all:	libaudio.a 

INCDIRS=	-I/usr/include/SDL

CPPFLAGS= $(INCDIRS) -Wall $(CXXFLAGS) -g

EXTRALIBS=  -L/usr/X11R6/lib/ -lgdk -lgtk -lgtkmm -lgdkmm -lc -lstdc++ -lgcc -lm -lXi -lXext -lX11 


OFILES=	Mixer.o Midi.o Audio.o Flex.o

libaudio.a:	$(OFILES)
	ar rs libaudio.a $(OFILES)

test:	libaudio.a test.o
	g++ $(CPPFLAGS) test.o -L. -laudio -o test $(EXTRALIBS) `sdl-config --libs`
	#cd .. && make

clean:
	rm -f libaudio.a core $(OFILES)

tar:
	rm -rf /tmp/audio
	mkdir /tmp/audio
	cp Audio.cc Audio.h Flex.cc Flex.h Midi.cc Midi.h Mixer.cc Mixer.h SDL_mapping.h Makefile /tmp/audio
	cd /tmp && tar czvpf exult_audio_lib.tar.gz audio

Audio.o: Audio.cc Audio.h Mixer.h Midi.h
Flex.o: Flex.cc Flex.h 
Midi.o: Midi.cc Midi.h SDL_mapping.h Mixer.h 
Mixer.o: Mixer.cc Mixer.h SDL_mapping.h Audio.h Midi.h
