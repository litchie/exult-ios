INCLUDES = -I$(srcdir)/../../../headers -I$(srcdir)/../include -I$(srcdir)/../../../conf -I$(srcdir)/../.. \
	-I$(srcdir)/../../.. $(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS) \
	-DEXULT_DATADIR=\"$(EXULT_DATADIR)\" $(WARNINGS)

ucxt_SOURCES = \
	opcodes.cc \
	ucdata.cc \
	ucxt.cc \
	ucfunc.cc

ucxt_LDADD = \
	../../../conf/libconf.la \
	../../../files/libu7file.la


if BUILD_TOOLS
bin_PROGRAMS = ucxt
endif
