INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../../../conf -I$(srcdir)/../.. \
	-I$(srcdir)/../../.. $(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS) \
	-DEXULT_DATADIR=\"$(EXULT_DATADIR)\"

ucxt_SOURCES = \
	opcodec_.cc \
	opcodes.cc \
	printucc.cc \
	stacki.cc \
	ucdata.cc \
	ucdump.cc \
	ucfunc.cc

ucxt_LDADD = \
	../../../conf/libconf.la \
	../../../files/libu7file.la


if BUILD_TOOLS
bin_PROGRAMS = ucxt
endif
