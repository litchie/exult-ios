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


OFILES=	forked_player.o KMIDI.o Timidity_binary.o win_MCI.o

SOURCES = KMIDI.cc Timidity_binary.cc forked_player.cc win_MCI.cc \
          KMIDI.h Timidity_binary.h forked_player.h win_MCI.h

midi-drivers:	$(OFILES)

clean:
	rm -f $(OFILES)


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

rebuild_master_dependencies:
	makedepend -f Makefile.am -Y $(CXXFLAGS) -I. -I.. -I../.. -I../../conf -I../../pathfinder -I../../flic - -- *.cc

# DO NOT DELETE

KMIDI.o: KMIDI.h ../../autoconfig.h ../../fnames.h ../../conf/Configuration.h
KMIDI.o: ../../conf/XMLEntity.h
Timidity_binary.o: ../Midi.h ../../autoconfig.h ../SDL_mapping.h ../Mixer.h
Timidity_binary.o: ../../fnames.h ../Audio.h ../../conf/Configuration.h
Timidity_binary.o: ../../conf/XMLEntity.h Timidity_binary.h ../Flex.h
Timidity_binary.o: ../common.h ../Table.h
forked_player.o: forked_player.h ../../autoconfig.h ../SDL_mapping.h
forked_player.o: ../Flex.h ../common.h ../Table.h ../Mixer.h ../Midi.h
forked_player.o: ../../autoconfig.h ../../fnames.h ../../conf/Configuration.h
forked_player.o: ../../conf/XMLEntity.h
