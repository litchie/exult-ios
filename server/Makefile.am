INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../files  \
	$(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

noinst_LTLIBRARIES = libserver.la

libserver_la_SOURCES = 	\
	objserial.cc	\
	objserial.h	\
	servemsg.cc	\
	servemsg.h	\
	server.cc	\
	server.h

CLEANFILES = *~
