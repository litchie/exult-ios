INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../files -I$(srcdir)/../imagewin \
		-I$(srcdir)/../shapes -I$(srcdir)/../objs -I$(srcdir)/../audio\
		-I$(srcdir)/../gumps -I$(srcdir)/../tools \
	$(SDL_CFLAGS) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

SUBDIRS = compiler ucxt

noinst_LTLIBRARIES = libusecode.la

libusecode_la_SOURCES = \
	bgintrinsics.h	\
	siintrinsics.h	\
	intrinsics.cc	\
	ucinternal.cc	\
	ucinternal.h	\
	ucmachine.cc	\
	ucmachine.h	\
	ucsched.h	\
	ucsched.cc	\
	ucscriptop.h	\
	useval.cc	\
	useval.h	\
	conversation.cc \
	conversation.h	\
	keyring.cc	\
	keyring.h	\
	ucdisasm.cc

CLEANFILES = *~
