INCLUDES = -I.. -I../../files \
	$(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS)

YFLAGS = -d				# Want ucparse.h.

ucc_SOURCES = ucparse.yy uclex.ll ucmain.cc \
	opcodes.h	\
	ucexpr.cc	\
	ucexpr.h	\
	ucfun.cc	\
	ucfun.h		\
	ucloc.cc	\
	ucloc.h		\
	ucparse.h	\
	ucstmt.cc	\
	ucstmt.h	\
	ucsym.cc	\
	ucsym.h

if BUILD_TOOLS
bin_PROGRAMS = ucc
endif

ucc_LDADD = 			\
	$(SYSLIBS)



CLEANFILES = *~ uclex.cc ucparse.cc
