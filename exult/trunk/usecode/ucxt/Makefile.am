SUBDIRS = src include data Docs

INCLUDES = -I$(srcdir)/../../headers -I$(srcdir)/../include -I$(srcdir)/../../../conf -I$(srcdir)/../.. \
        -I$(srcdir)/../../.. -I$(srcdir)/.. $(INCDIRS) $(DEBUG_FLAGS) \
        $(CPPFLAGS) -DEXULT_DATADIR=\"$(EXULT_DATADIR)\" $(WARNINGS)

if BUILD_TOOLS
noinst_PROGRAMS = head2data
endif

head2data_SOURCES = head2data.cc head2data.test

head2data_LDADD = \
        ../../files/libu7file.la

head2data.test: ../bgintrinsics.h ../siintrinsics.h
	./head2data data/u7bgintrinsics.data data/u7siintrinsics.data

