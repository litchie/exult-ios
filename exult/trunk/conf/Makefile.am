AUTOMAKE_OPTIONS=foreign

all-local:	libconf.a 

CXXFLAGS += $(INCDIRS) -Wall 
CXXFLAGS += $(WINDOWING_SYSTEM)




OFILES=	xmlparse.o xmldump.o XMLEntity.o Configuration.o

libconf.a:	$(OFILES)
	ar rs libconf.a $(OFILES)

clean:
	rm -f libconf.a core $(OFILES)

test: libconf.a xmain.o
	g++ -L. xmain.o -o test $(OFILES)

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

