AUTOMAKE_OPTIONS=foreign

OBJECTS = Flex.o Table.o U7file.o IFF.o

HFILES = Flex.h  Table.h  U7file.h  common_types.h IFF.h

SOURCES = $(HFILES) $(OBJECTS:%.o=%.cc)

EXTRA_DIST = 

SUFFIXES=.cc .h

INCLUDES += -I.
INCLUDES += $(AUDIO_INCLUDES)
INCLUDES += -Iconf
INCLUDES += $(SDL_INCLUDES)
INCLUDES += $(KMID_INCLUDES)

CXXFLAGS += $(PEDANTIC)
CXXFLAGS += -DRELNUM=$(RELNUM)
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< $(INCLUDES)



LFLAGS += -g 

all-local: libu7file.a

clean:
	-rm -f *.o core *.a


libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libu7file.a:      $(OBJECTS)
	ar rs libu7file.a $(OBJECTS)

test:	libu7file.a test.o
	g++ $(CXXFLAGS) -o filetest test.o -L. -lu7file

# dependencies
# DO NOT DELETE

Flex.o: Flex.h ../autoconfig.h common_types.h U7file.h
Table.o: Table.h ../autoconfig.h U7file.h common_types.h
U7file.o: U7file.h Flex.h ../autoconfig.h common_types.h Table.h
