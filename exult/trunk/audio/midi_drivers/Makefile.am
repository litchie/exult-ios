AUTOMAKE_OPTIONS=foreign

all-local:	midi-drivers

INCLUDES +=	$(SDL_INCLUDES)
INCLUDES +=	$(KMID_INCLUDES)
INCLUDES +=	-I../../conf
INCLUDES +=	-I..

CXXFLAGS += $(INCDIRS) -Wall 
CXXFLAGS += $(PEDANTIC)
CXXFLAGS += $(DEBUG_FLAGS)
CXXFLAGS += $(WINDOWING_SYSTEM)

#LIBS +=  -L/usr/X11R6/lib/ -lc -lstdc++ -lgcc -lm -lXi -lXext -lX11 


OFILES=	forked_player.o KMIDI.o Timidity_binary.o

SOURCES = Audio.cc Audio.h Flex.cc Flex.h IFF.cc IFF.h Midi.cc Midi.h \
		Mixer.cc Mixer.h SDL_mapping.cc SDL_mapping.h Table.cc Table.h \
		common.h test.cc

midi-drivers:	$(OFILES)

clean:
	rm -f $(OFILES)


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

forked_player.o: forked_player.h ../Midi.h
KMIDI.o: KMIDI.h ../Midi.h
Timidity_binary.o: Timidity_binary.h ../Midi.h
