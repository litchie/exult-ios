INCLUDES = -I.. -I../files $(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

lib_LTLIBRARIES = libconf.la

libconf_la_SOURCES =	\
	Configuration.cc	\
	Configuration.h		\
	XMLEntity.cc		\
	XMLEntity.h		\
	xmldump.cc		\
	xmlparse.cc

EXTRA_DIST = 		\
	config.xml	\
	README
