INCLUDES = -I$(srcdir)/../../headers -I$(srcdir)/.. -I$(srcdir)/../../files -I$(srcdir)/../.. \
	$(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS)

AM_YFLAGS = -d				# Want ucparse.h.

ucc_SOURCES = ucparse.yy uclex.ll ucmain.cc \
	opcodes.h	\
	ucexpr.cc	\
	ucexpr.h	\
	ucfun.cc	\
	ucfun.h		\
	uclabel.h	\
	ucloc.cc	\
	ucloc.h		\
	ucparse.h	\
	ucstmt.cc	\
	ucstmt.h	\
	ucsym.cc	\
	ucsym.h

if BUILD_COMPILER
bin_PROGRAMS = ucc
endif

ucc_LDADD = 			\
	$(SYSLIBS)



CLEANFILES = *~
MAINTAINERCLEANFILES = uclex.cc ucparse.cc
