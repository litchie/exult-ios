AUTOMAKE_OPTIONS=foreign

all-local:	libflic.a

INCDIRS += -I.. $(SDL_INCLUDES)
CXXFLAGS += $(INCDIRS) -Wall
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)


OFILES=	playfli.o

SOURCES= playfli.cc playfli.h

libflic.a:	$(OFILES)
	ar rs libflic.a $(OFILES)

clean:
	rm -f libflic.a core $(OFILES)

test:	libflic.a play.o
	g++ $(CXXFLAGS) -o playflic play.o ../utils.o -L. -lflic

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

rebuild_master_dependencies:
	makedepend -f Makefile.am -Y $(CXXFLAGS) -I. -I.. -I../conf -I../audio -I../pathfinder -I../files - -- *.cc


# DO NOT DELETE

play.o: playfli.h ../imagewin.h
playfli.o: playfli.h ../imagewin.h
playfli.o: ../utils.h
