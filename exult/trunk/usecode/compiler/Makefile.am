INCLUDES = -I.. -I../../files \
	$(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS)

YFLAGS = -d				# Want ucparse.h.

ucc_SOURCES = ucparse.yy uclex.ll ucmain.cc \
	ucexpr.cc	\
	ucexpr.h	\
	ucfun.cc	\
	ucfun.h		\
	ucloc.cc	\
	ucloc.h		\
	ucstmt.cc	\
	ucstmt.h	\
	ucsym.cc	\
	ucsym.h

if BUILD_TOOLS
bin_PROGRAMS = ucc
endif

ucc_LDADD = 			\
	$(SYSLIBS)



