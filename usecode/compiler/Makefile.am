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
	ucsym.cc	\
	ucsym.h

bin_PROGRAMS = ucc

ucc_LDADD = 			\
	$(SYSLIBS)



