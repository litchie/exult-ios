INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/.. -I$(srcdir)/../shapes \
	-I$(srcdir)/../imagewin -I$(srcdir)/../conf \
	-I$(srcdir)/../files -I$(srcdir)/../server \
	$(GTK_INCLUDES) $(LIBGLADE_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\"$(EXULT_DATADIR)\"

if GIMP_PLUGIN
GIMP_PLUGINS=u7shp
else
GIMP_PLUGINS=
endif

GTK_LIBS = @GTK_LIBS@

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
	dirbrowser.cc \
	dirbrowser.h \
	eggedit.cc \
	locator.cc \
	locator.h \
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
	-lpng $(GTK_LIBS) $(FREETYPE2_LIBS) $(SYSLIBS) $(x_libraries) \
	$(LIBGLADE_LIBS)

exult_studio_CFLAGS = $(LIBGLADE_INCLUDES)

mapeditdir = $(datadir)/exult

mapedit_DATA = exult_studio.glade

u7shp$(EXEEXT): u7shp.c
	gimptool --build $<

EXTRA_DIST=	\
	exult_studio.glade \
	logo.xpm

if GIMP_PLUGIN
install-exec-local:
	install -d $(GIMP_PLUGIN_PREFIX)
	install -c $(GIMP_PLUGINS) $(GIMP_PLUGIN_PREFIX)
else
install-exec-local:
endif

CLEANFILES = *~ u7shp
