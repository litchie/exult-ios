INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/../files -I$(srcdir)/../usecode \
	-I$(srcdir)/.. -I$(srcdir)/../shapes -I$(srcdir)/../imagewin \
	-I$(srcdir)/../conf $(DEBUG_FLAGS) $(CPPFLAGS) $(SDL_CFLAGS)

if HAVE_PNG
IPACK = ipack
else
IPACK =
endif

if BUILD_TOOLS
NOINSTTOOLS = wuc wud mklink rip ucdump cmanip
EXTRATOOLS = splitshp shp2pcx textpack $(IPACK)
else
EXTRATOOLS =
NOINSTTOOLS =
endif

bin_PROGRAMS = expack $(EXTRATOOLS)
noinst_PROGRAMS = $(NOINSTTOOLS)

expack_SOURCES = \
	expack.cc

expack_LDADD = \
	../files/libu7file.la \
	$(SYSLIBS)

ipack_SOURCES = \
	ipack.cc

ipack_LDADD = \
	../files/libu7file.la \
	../shapes/libshapes.la \
	../imagewin/libimagewin.la \
	-lpng $(SYSLIBS)

wuc_SOURCES = \
	wuc.c \
	uctools.h

wud_SOURCES = wud.c

mklink_SOURCES = mklink.c

rip_SOURCES = rip.c

ucdump_SOURCES = ucdump.c

shp2pcx_SOURCES = shp2pcx.cc

shp2pcx_LDADD = $(SYSLIBS)

splitshp_SOURCES = splitshp.cc

splitshp_LDADD = $(SYSLIBS)

textpack_SOURCES = textpack.cc

textpack_LDADD = \
	../files/libu7file.la \
	$(SYSLIBS)

cmanip_SOURCES = \
	cmanip.cc 

cmanip_LDADD = \
	../conf/libconf.la \
	../files/libu7file.la \
	$(SYSLIBS)

EXTRA_DIST = \
	intrins1.txt \
	intrins2.txt \
	README \
	u7bgflag.txt \
	u7siflag.txt \
	ucformat.txt \
	expack.txt

CLEANFILES = *~
