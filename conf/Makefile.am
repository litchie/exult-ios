INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../files $(SDL_INCLUDES) $(INCDIRS) \
		 $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

if BUILD_TOOLS
noinst_PROGRAMS = confregress
endif

noinst_LTLIBRARIES = libconf.la

libconf_la_SOURCES =	\
	Configuration.cc	\
	Configuration.h		\
	XMLEntity.cc		\
	XMLEntity.h

confregress_SOURCES =	\
	xmain.cc

confregress_LDADD =	\
	libconf.la	\
	../files/libu7file.la	\
	$(SYSLIBS)

EXTRA_DIST = 		\
	config.xml	\
	README

CLEANFILES = *~
