INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../shapes \
	-I$(srcdir)/../imagewin -I$(srcdir)/../conf \
	-I$(srcdir)/../files -I$(srcdir)/../server \
	$(GLADE_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\"$(EXULT_DATADIR)\"

if GIMP_PLUGIN
GIMP_PLUGINS=u7shp
else
GIMP_PLUGINS=
endif

if BUILD_STUDIO
bin_PROGRAMS = exult_studio 
else
bin_PROGRAMS =
endif

u7shp_SOURCES = u7shp.c

exult_studio_SOURCES = \
	chunklst.cc \
	chunklst.h \
	combo.cc \
	combo.h \
	compile.cc \
	eggedit.cc \
	execbox.h \
	execbox.cc \
	locator.cc \
	locator.h \
	maps.cc \
	npcedit.cc \
	objedit.cc \
	shapeedit.cc \
	studio.cc \
	studio.h \
	exult_studio.cc \
	objbrowse.h \
	objbrowse.cc \
	paledit.cc \
	paledit.h \
	shapedraw.cc \
	shapedraw.h \
	shapefile.cc \
	shapefile.h \
	shapegroup.cc \
	shapegroup.h \
	shapelst.cc \
	shapelst.h


noinst_PROGRAMS = $(GIMP_PLUGINS)

exult_studio_LDADD = \
	../conf/libconf.la		\
	../files/libu7file.la		\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la 		\
	../server/libserver.la		\
	-lpng $(FREETYPE2_LIBS) $(SYSLIBS) $(x_libraries) $(GLADE_LIBS)

exult_studio_CFLAGS = $(LIBGLADE_INCLUDES)
exult_studio_LDFLAGS = -export-dynamic		# For libglade 2.x

mapeditdir = $(datadir)/exult

if BUILD_STUDIO
mapedit_DATA = exult_studio.glade
else
mapedit_DATA =
endif

u7shp$(EXEEXT): u7shp.c
	$(GIMPTOOL) --build $<

EXTRA_DIST=	\
	exult_studio.glade \
	logo.xpm

if GIMP_PLUGIN
install-exec-local:
	install -d $(DESTDIR)$(GIMP_PLUGIN_PREFIX)
	install -c $(DESTDIR)$(GIMP_PLUGINS) $(GIMP_PLUGIN_PREFIX)
else
install-exec-local:
endif

CLEANFILES = *~ u7shp
