AUTOMAKE_OPTIONS=foreign

all-local:	libpathfinder.a 

CXXFLAGS += $(INCDIRS) -Wall
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)




OFILES=	PathFinder.o Astar.o Zombie.o

SOURCES= PathFinder.cc PathFinder.h Astar.h Astar.cc Zombie.h Zombie.cc

libpathfinder.a:	$(OFILES)
	ar rs libpathfinder.a $(OFILES)

clean:
	rm -f libpathfinder.a core $(OFILES)


%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

PathFinder.o: PathFinder.h
Astar.o: Astar.h PathFinder.h
Zombie.o: Zombie.h PathFinder.h
