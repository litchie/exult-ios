INCLUDES = -I.. -I../files $(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

noinst_PROGRAMS = confregress

noinst_LTLIBRARIES = libconf.la

libconf_la_SOURCES =	\
	Configuration.cc	\
	Configuration.h		\
	XMLEntity.cc		\
	XMLEntity.h		\
	xmldump.cc		\
	xmlparse.cc

confregress_SOURCES =	\
	xmain.cc

confregress_LDADD =	\
	libconf.la	\
	../files/libu7file.la 

EXTRA_DIST = 		\
	config.xml	\
	README
