SUBDIRS = . src include data Docs

INCLUDES = -I$(srcdir)/../../headers -I$(srcdir)/../include -I$(srcdir)/../../../conf -I$(srcdir)/../.. \
        -I$(srcdir)/../../.. -I$(srcdir)/.. $(INCDIRS) $(DEBUG_FLAGS) \
        $(CPPFLAGS) -DEXULT_DATADIR=\"$(EXULT_DATADIR)\" $(WARNINGS)

if BUILD_TOOLS
noinst_PROGRAMS = head2data
endif

head2data_SOURCES = head2data.cc
