INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../files -I$(srcdir)/../objs \
	-I$(srcdir)/../imagewin -I$(srcdir)/../shapes \
	$(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

noinst_LTLIBRARIES = libserver.la

libserver_la_SOURCES = 	\
	objserial.cc	\
	objserial.h	\
	servemsg.cc	\
	servemsg.h	\
	server.cc	\
	server.h	\
	servewin32.cc	\
	servewin32.h

CLEANFILES = *~
