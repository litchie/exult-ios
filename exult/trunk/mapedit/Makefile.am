INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../shapes -I$(srcdir)/../imagewin \
	-I$(srcdir)/../files -I$(srcdir)/../server \
	$(GTK_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

if GIMP_PLUGIN
GIMP_PLUGINS=u7shp
else
GIMP_PLUGINS=
endif

if HAVE_GTK
GTK_APPS = shapetest paltest
GTK_LIBS = -lgtk -lgdk
OUTPUT_LIBS = libmapedit.la
else
GTK_APPS = 
GTK_LIBS =
OUTPUT_LIBS =
endif

if HAVE_GLADE 
GLADE_APPS = exult_studio
else
GLADE_APPS =
endif

shapetest_SOURCES = shapetest.cc

paltest_SOURCES = paltest.cc

exult_studio_SOURCES = \
	chunklst.cc \
	chunklst.h \
	dirbrowser.cc \
	dirbrowser.h \
	eggedit.cc \
	npcedit.cc \
	objedit.cc \
	studio.cc \
	studio.h \
	exult_studio.cc

bin_PROGRAMS = $(GTK_APPS) $(GIMP_PLUGINS) $(GLADE_APPS)

shapetest_LDADD = \
	./libmapedit.la			\
	../files/libu7file.la		\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la 		\
	../server/libserver.la		\
	$(GTK_LIBS) $(SYSLIBS) $(x_libraries)

paltest_LDADD = $(shapetest_LDADD)

exult_studio_LDADD = $(shapetest_LDADD) $(LIBGLADE_LIBS)

mapeditdir = $(datadir)/exult

mapedit_DATA = exult_studio.glade

noinst_LTLIBRARIES = $(OUTPUT_LIBS)

libmapedit_la_SOURCES = \
	objbrowse.h \
	objbrowse.cc \
	paledit.cc \
	paledit.h \
	shapedraw.cc \
	shapedraw.h \
	shapelst.cc \
	shapelst.h

u7shp: u7shp.c
	gimptool --build $<

if GIMP_PLUGIN
install-exec-local:
	gimptool --install-admin-bin u7shp
else
install-exec-local:
endif

CLEANFILES = *~ u7shp
