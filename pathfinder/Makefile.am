AUTOMAKE_OPTIONS=foreign

all-local:	libpathfinder.a 

CXXFLAGS += $(INCDIRS) -Wall
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)




OFILES=	PathFinder.o Astar.o Zombie.o path.o

SOURCES= PathFinder.cc PathFinder.h Astar.h Astar.cc Zombie.h Zombie.cc path.cc

libpathfinder.a:	$(OFILES)
	ar rs libpathfinder.a $(OFILES)

clean:
	rm -f libpathfinder.a core $(OFILES)


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

rebuild_master_dependencies:
	makedepend -f Makefile.am -Y $(CXXFLAGS) -I. -I.. -I../conf -I../audio -I../files -I../flic - -- *.cc


# DO NOT DELETE

Astar.o: Astar.h PathFinder.h ../tiles.h
PathFinder.o: PathFinder.h ../tiles.h
Zombie.o: Zombie.h PathFinder.h ../tiles.h
path.o: PathFinder.h ../tiles.h ../objs.h ../vec.h ../usecode.h ../tqueue.h
path.o: ../tiles.h
