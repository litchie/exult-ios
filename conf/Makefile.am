AUTOMAKE_OPTIONS=foreign

all-local:	libconf.a 

CXXFLAGS += $(INCDIRS) -Wall 
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)




OFILES=	xmlparse.o xmldump.o XMLEntity.o Configuration.o

SOURCES= Configuration.cc Configuration.h XMLEntity.cc XMLEntity.h \
		xmain.cc xmldump.cc xmlparse.cc

libconf.a:	$(OFILES)
	ar rs libconf.a $(OFILES)

clean:
	rm -f libconf.a core $(OFILES)

test: libconf.a xmain.o
	g++ -L. xmain.o -o test $(OFILES)

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

