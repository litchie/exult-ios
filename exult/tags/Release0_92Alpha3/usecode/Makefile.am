INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../files -I$(srcdir)/../imagewin \
		-I$(srcdir)/../shapes -I$(srcdir)/../objs -I$(srcdir)/../audio \
		-I$(srcdir)/../gumps \
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

SUBDIRS = compiler # Compile errors: ucxt

noinst_LTLIBRARIES = libusecode.la

libusecode_la_SOURCES = \
	bgintrinsics.h	\
	siintrinsics.h	\
	intrinsics.cc	\
	ucmachine.cc	\
	ucmachine.h	\
	ucsched.h	\
	ucsched.cc	\
	useval.cc	\
	useval.h	\
	conversation.cc \
	conversation.h

CLEANFILES = *~
