INCLUDES = -I.. -I../shapes -I../imagewin -I../files \
	$(GTK_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

if GIMP_PLUGIN
GIMP_PLUGINS=u7shp
else
GIMP_PLUGINS=
endif

if GTK_INTERFACE
GTK_APPS = shapetest paltest exultstudio
GTK_LIBS = -lgtk -lgdk
OUTPUT_LIBS = libmapedit.la
else
GTK_APPS = 
GTK_LIBS =
OUTPUT_LIBS =
endif

shapetest_SOURCES = shapetest.cc

paltest_SOURCES = paltest.cc

exultstudio_SOURCES = exultstudio.cc

bin_PROGRAMS = $(GTK_APPS) $(GIMP_PLUGINS)

shapetest_LDADD = \
	./libmapedit.la			\
	../files/libu7file.la		\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la 		\
	$(GTK_LIBS) $(SYSLIBS) $(x_libraries)

paltest_LDADD = $(shapetest_LDADD)

exultstudio_LDADD = $(shapetest_LDADD) $(GIMP_LIBS)

noinst_LTLIBRARIES = $(OUTPUT_LIBS)

libmapedit_la_SOURCES = \
	paledit.cc \
	paledit.h \
	shapelst.cc \
	shapelst.h

u7shp: u7shp.c
	gimptool --build $<

if GIMP_PLUGIN
install-exec-local:
	gimptool --install-admin-bin u7shp
endif

CLEANFILES = *~ u7shp
