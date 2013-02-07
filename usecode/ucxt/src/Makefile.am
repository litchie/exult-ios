AM_CPPFLAGS = -I$(srcdir)/../../../headers -I$(srcdir)/../include -I$(srcdir)/../../../conf -I$(srcdir)/../.. \
	-I$(srcdir)/../../../usecode -I$(srcdir)/../../.. $(INCDIRS) $(DEBUG_FLAGS) $(CPPFLAGS) \
	-DEXULT_DATADIR=\"$(EXULT_DATADIR)\" $(WARNINGS)

ucxt_SOURCES = \
	ops.cc \
	ucdata.cc \
	ucxt.cc \
	ucfunc.cc

ucxt_LDADD = \
	../../../conf/libconf.la \
	../../../gamemgr/libgamemgr.la \
	../../../files/libu7file.la \
	../../../files/zip/libminizip.la \
	../../../usecode/libusecode.la \
	$(ZLIB_LIBS) \
	$(SYSLIBS)
	


if BUILD_TOOLS
bin_PROGRAMS = ucxt
endif
