INCLUDES = -I.. \
	$(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS)

YFLAGS = -d				# Want ucparse.h.

ucc_SOURCES = ucparse.yy uclex.ll ucmain.cc \
	ucloc.cc	\
	ucloc.h

bin_PROGRAMS = ucc

ucc_LDADD = 			\
	$(SYSLIBS)



